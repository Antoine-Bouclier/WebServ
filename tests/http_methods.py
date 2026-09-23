import socket, subprocess, tempfile, time, pathlib, concurrent.futures, os, signal, resource
ROOT=pathlib.Path(__file__).resolve().parents[1]
BINARY=os.environ.get("WEBSERV_BINARY", str(ROOT / "webserv"))
checks=0
with tempfile.TemporaryDirectory(prefix='webserv-methods-') as tmp:
 d=pathlib.Path(tmp); www=d/'www'; www.mkdir(); uploads=www/'files'; uploads.mkdir(); (www/'index.html').write_bytes(b'INDEX'); (www/'empty').write_bytes(b''); (www/'dir').mkdir(); (www/'dir'/'<a>.txt').write_text('listing'); (www/'noindex').mkdir(); (www/'nested').mkdir(); (www/'nested'/'index.html').write_bytes(b'NESTED'); binary=bytes(range(256))*1600; (www/'large.bin').write_bytes(binary); (www/'error.html').write_bytes(b'CUSTOM404')
 with socket.socket() as s: s.bind(('127.0.0.1',0)); port=s.getsockname()[1]
 config=d/'test.conf'; config.write_text(f'''server {{ listen 127.0.0.1:{port}; root {www}; index index.html; client_max_body_size 1M; error_page 404 /error.html;
 location / {{ allowed_methods GET POST DELETE; }}
 location /files {{ root {uploads}; allowed_methods GET DELETE; autoindex on; }}
 location /upload {{ allowed_methods POST; upload_path {uploads}; }}
 location /small {{ allowed_methods POST; upload_path {uploads}; client_max_body_size 100; }}
 location /dir {{ root {www}/dir; allowed_methods GET; autoindex on; }}
 location /readonly {{ root {www}; allowed_methods GET; }}
 location /old {{ return 301 /; }}
 }}''')
 log=open(d/'log','wb'); process=subprocess.Popen([BINARY,str(config)],cwd=ROOT,stdout=log,stderr=log)
 def request(method,path,body=b'',ctype=None,chunked=False):
  h=f'{method} {path} HTTP/1.1\r\nHost: localhost\r\n'.encode()
  if ctype is not None: h+=b'Content-Type: '+ctype.encode()+b'\r\n'
  if chunked:
   h+=b'Transfer-Encoding: chunked\r\n'; wire=b''.join(f'{len(body[i:i+17]):x}\r\n'.encode()+body[i:i+17]+b'\r\n' for i in range(0,len(body),17))+b'0\r\n\r\n'
  else: h+=f'Content-Length: {len(body)}\r\n'.encode(); wire=body
  with socket.create_connection(('127.0.0.1',port),timeout=3) as s:
   s.settimeout(5); s.sendall(h+b'\r\n'+wire); data=b''
   while True:
    part=s.recv(65536)
    if not part: break
    data+=part
  head,content=data.split(b'\r\n\r\n',1); status=int(head.split()[1]); headers=dict(line.split(b': ',1) for line in head.split(b'\r\n')[1:]); return status,headers,content
 def check(label,method,path,status,body=b'',ctype=None,chunked=False,expected=None):
  global checks
  r=request(method,path,body,ctype,chunked); assert r[0]==status,(label,r[0],r[2][:100]);
  if expected is not None: assert r[2]==expected,(label,'body mismatch')
  if b'Content-Length' in r[1]: assert int(r[1][b'Content-Length'])==len(r[2]),label
  checks+=1; return r
 def multipart(parts,boundary='AaB03x'):
  result=b''
  for filename,data in parts:
   disposition='form-data; name="file"'+('' if filename is None else '; filename="'+filename+'"')
   result+=b'--'+boundary.encode()+b'\r\nContent-Disposition: '+disposition.encode()+b'\r\nContent-Type: application/octet-stream\r\n\r\n'+data+b'\r\n'
  return result+b'--'+boundary.encode()+b'--\r\n'
 ctype='multipart/form-data; boundary=AaB03x'
 try:
  for _ in range(100):
   if process.poll() is not None: raise RuntimeError((d/'log').read_text())
   try:
    with socket.create_connection(('127.0.0.1',port),timeout=.1): break
   except OSError: time.sleep(.02)
  check('index','GET','/',200,expected=b'INDEX')
  check('binary streamed','GET','/large.bin',200,expected=binary)
  check('empty GET','GET','/empty',200,expected=b'')
  check('custom 404','GET','/missing',404,expected=b'CUSTOM404')
  r=check('directory redirect query','GET','/dir?x=1',301); assert r[1][b'Location']==b'/dir/?x=1'
  r=check('autoindex','GET','/dir/',200); assert b'&lt;a&gt;.txt' in r[2] and b'%3Ca%3E.txt' in r[2]
  check('nested index','GET','/nested/',200,expected=b'NESTED')
  check('no listing','GET','/noindex/',403)
  check('configured redirect','GET','/old',301)
  check('traversal','GET','/%2e%2e/outside',403)
  r=check('method restricted','DELETE','/readonly/index.html',405); assert b'GET' in r[1][b'Allow']
  check('delete directory','DELETE','/files',403)
  check('delete missing','DELETE','/files/no',404)
  check('post not configured','POST','/',403)
  check('missing type','POST','/upload',415,body=b'x')
  check('wrong type','POST','/upload',415,body=b'{}',ctype='application/json')
  check('missing boundary','POST','/upload',400,ctype='multipart/form-data')
  check('empty boundary','POST','/upload',400,ctype='multipart/form-data; boundary=""')
  check('bad quoted boundary','POST','/upload',400,ctype='multipart/form-data; boundary="abc')
  check('duplicate boundary','POST','/upload',400,ctype=ctype+'; boundary=x')
  check('long boundary','POST','/upload',400,ctype='multipart/form-data; boundary='+'a'*71)
  check('binary upload','POST','/upload',201,multipart([('binary.bin',binary)]),ctype)
  assert (uploads/'binary.bin').read_bytes()==binary
  check('retrieve upload','GET','/files/binary.bin',200,expected=binary)
  check('conflict','POST','/upload',409,multipart([('binary.bin',b'overwrite')]),ctype); assert (uploads/'binary.bin').read_bytes()==binary
  r=check('delete upload','DELETE','/files/binary.bin',204,expected=b''); assert b'Content-Length' not in r[1] and not (uploads/'binary.bin').exists()
  check('deleted upload','GET','/files/binary.bin',404)
  check('fields multiple empty files','POST','/upload',201,multipart([(None,b'text'),('one.txt',b'one'),('empty.bin',b'')]),ctype)
  assert (uploads/'one.txt').read_bytes()==b'one' and (uploads/'empty.bin').read_bytes()==b''
  check('quoted reordered parameters','POST','/upload',201,multipart([('quoted.txt',b'abc')]),'Multipart/Form-Data; charset=UTF-8; BOUNDARY="AaB03x"')
  check('chunked upload','POST','/upload',201,multipart([('chunk.txt',b'\x00chunk\xff')]),ctype,True); assert (uploads/'chunk.txt').read_bytes()==b'\x00chunk\xff'
  fake=b'abc\r\n--AaB03xNO\r\n--AaB03x--NO\x00tail'
  check('boundary lookalikes','POST','/upload',201,multipart([('look.bin',fake)]),ctype); assert (uploads/'look.bin').read_bytes()==fake
  check('quoted filename semicolon','POST','/upload',201,multipart([('semi;colon.txt',b'x')]),ctype)
  check('escaped filename quote','POST','/upload',201,multipart([('quote\\".txt',b'x')]),ctype); assert (uploads/'quote".txt').exists()
  for name in ['../escape','a/b','..','.','bad\x00name']:
   check('unsafe filename '+repr(name),'POST','/upload',400,multipart([(name,b'x')]),ctype)
  check('no selected file','POST','/upload',400,multipart([('',b'')]),ctype)
  check('text only','POST','/upload',400,multipart([(None,b'hi')]),ctype)
  check('duplicate filenames','POST','/upload',400,multipart([('dup',b'a'),('dup',b'b')]),ctype); assert not (uploads/'dup').exists()
  check('truncated multipart','POST','/upload',400,multipart([('truncated',b'x')])[:-8],ctype); assert not (uploads/'truncated').exists()
  check('rollback on conflict','POST','/upload',409,multipart([('aaa-new',b'x'),('one.txt',b'bad')]),ctype); assert not (uploads/'aaa-new').exists() and (uploads/'one.txt').read_bytes()==b'one'
  check('bad last part no partial writes','POST','/upload',400,multipart([('aaa-new',b'x'),('../bad',b'y')]),ctype); assert not (uploads/'aaa-new').exists()
  check('body limit','POST','/small',413,multipart([('big',b'a'*200)]),ctype)
  malformed= b'--AaB03x\r\nContent-Disposition: form-data; name="f"; filename="x"\r\nContent-Disposition: form-data; name="f"\r\n\r\nx\r\n--AaB03x--\r\n'
  check('duplicate part header','POST','/upload',400,malformed,ctype)
  (www/'private').write_bytes(b'private'); (www/'private').chmod(0)
  check('unreadable GET','GET','/private',403)
  (www/'private').chmod(0o600)
  uploads.chmod(0o555)
  try:
   check('unwritable upload','POST','/upload',403,multipart([('permission',b'x')]),ctype)
   check('unwritable delete','DELETE','/files/one.txt',403)
   assert (uploads/'one.txt').read_bytes()==b'one'
  finally: uploads.chmod(0o755)
  with concurrent.futures.ThreadPoolExecutor(max_workers=12) as pool:
   rs=list(pool.map(lambda _:request('GET','/large.bin'),range(24)))
  assert all(r[0]==200 and r[2]==binary for r in rs); checks+=24
  check('alive after errors','GET','/',200,expected=b'INDEX')
  diagnostics=(d/'log').read_text()
  assert 'ERROR: AddressSanitizer' not in diagnostics and 'runtime error:' not in diagnostics, diagnostics
  process.terminate(); process.wait(timeout=3)
  def limit_file_size():
   signal.signal(signal.SIGXFSZ, signal.SIG_IGN)
   resource.setrlimit(resource.RLIMIT_FSIZE, (1024, 1024))
  process=subprocess.Popen([BINARY,str(config)],cwd=ROOT,stdout=subprocess.DEVNULL,stderr=log,preexec_fn=limit_file_size)
  for _ in range(100):
   if process.poll() is not None: raise RuntimeError((d/'log').read_text())
   try:
    with socket.create_connection(('127.0.0.1',port),timeout=.1): break
   except OSError: time.sleep(.02)
  check('disk write failure rollback','POST','/upload',500,multipart([('aaa-before-failure',b'ok'),('zzz-write-failure',b'x'*2048)]),ctype)
  assert not (uploads/'aaa-before-failure').exists() and not (uploads/'zzz-write-failure').exists()
  check('alive after disk failure','GET','/',200,expected=b'INDEX')
  diagnostics=(d/'log').read_text()
  assert 'ERROR: AddressSanitizer' not in diagnostics and 'runtime error:' not in diagnostics, diagnostics
  print(f'{checks} HTTP checks passed; file content, permissions and rollback verified.')
 finally:
  process.terminate()
  try: process.wait(timeout=3)
  except subprocess.TimeoutExpired: process.kill(); process.wait()
  log.close()
