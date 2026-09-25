import concurrent.futures
import json
import os
import pathlib
import signal
import socket
import struct
import subprocess
import tempfile
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
BINARY = os.environ.get("WEBSERV_BINARY", str(ROOT / "webserv"))
checks = 0

with tempfile.TemporaryDirectory(prefix="webserv-cgi-") as tmp:
    directory = pathlib.Path(tmp)
    (directory / "index.html").write_text("STATIC")
    scripts = directory / "scripts"
    scripts.mkdir()
    (scripts / "relative.txt").write_text("RELATIVE")
    (scripts / "echo.py").write_text("""import json, os, sys
body = sys.stdin.buffer.read()
result = {"body": body.hex(), "relative": open("relative.txt").read(), "env": dict(os.environ)}
sys.stdout.write("Content-Type: application/json\\r\\n\\r\\n" + json.dumps(result))
""")
    (scripts / "large.py").write_text("""import sys
sys.stdout.buffer.write(b"Content-Type: application/octet-stream\\r\\n\\r\\n" + b"x" * 262144)
sys.stdout.buffer.flush()
sys.stdout.buffer.write(sys.stdin.buffer.read())
""")
    (scripts / "slow.py").write_text("""import time
time.sleep(2)
print("Content-Type: text/plain\\n\\nSLOW", end="")
""")
    (scripts / "timeout.py").write_text("""import time
time.sleep(60)
""")
    (scripts / "crash.py").write_text("raise RuntimeError('expected test failure')\n")
    (scripts / "badexec.py").write_text("print('unused')\n")
    (scripts / "forbidden.py").write_text("print('unused')\n")
    (scripts / "forbidden.py").chmod(0)
    outputs = {
        "status": b"Status: 201 Created\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nCREATED",
        "redirect": b"Location: https://example.org/\r\n\r\n",
        "local": b"Status: 302 Found\r\nLocation: /\r\n\r\n",
        "empty": b"Status: 204 No Content\r\n\r\n",
        "notmodified": b"Status: 304 Not Modified\r\n\r\n",
        "lf": b"Content-Type: application/octet-stream\n\nA\r\n\r\nB",
        "binary": b"Content-Type: application/octet-stream\r\n\r\n\x00\xffABC",
        "invalid": b"not a CGI response",
        "badstatus": b"Status: xx\r\nContent-Type: text/plain\r\n\r\nBAD",
        "short": b"Content-Type: text/plain\r\nContent-Length: 99\r\n\r\nSHORT",
        "duplicate": b"Content-Type: text/plain\r\nContent-Type: text/plain\r\n\r\nBAD",
        "missingtype": b"\r\n\r\n",
        "control": b"Content-Type: text/plain\r\nX-Test: bad\x00value\r\n\r\nBAD",
        "hugeheader": b"X-Test: " + b"x" * 20000,
    }
    for name, output in outputs.items():
        (scripts / (name + ".py")).write_text("import sys\nsys.stdout.buffer.write(" + repr(output) + ")\n")
    (scripts / "fragmented.py").write_text("""import sys, time
for part in [b"Content-T", b"ype: text/plain\\r", b"\\n\\r", b"\\nFRAGMENTED"]:
    sys.stdout.buffer.write(part)
    sys.stdout.buffer.flush()
    time.sleep(.03)
""")
    (scripts / "closed.py").write_text("""import os, time
os.close(1)
time.sleep(60)
""")
    (scripts / "excess.py").write_text("""import sys
sys.stdout.buffer.write(b"Content-Type: text/plain\\r\\n\\r\\n")
for i in range(65):
    sys.stdout.buffer.write(b"x" * 1048576)
""")
    interpreter = directory / "invalid-interpreter"
    interpreter.write_text("not an executable format")
    interpreter.chmod(0o700)
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        port = sock.getsockname()[1]
    config = directory / "server.conf"
    config.write_text(f"""server {{
listen 127.0.0.1:{port};
root {directory};
index index.html;
client_max_body_size 4M;
location / {{ allowed_methods GET POST DELETE; }}
location /cgi {{ root {scripts}; cgi .py /usr/bin/python3; allowed_methods GET POST; index echo.py; }}
location /readonly {{ root {scripts}; cgi .py /usr/bin/python3; allowed_methods GET; }}
location /redirect {{ root {scripts}; cgi .py /usr/bin/python3; return 302 /; }}
location /broken {{ root {scripts}; cgi .py {interpreter}; }}
location /missingbin {{ root {scripts}; cgi .py /does/not/exist; }}
}}""")
    log = open(directory / "server.log", "wb")
    process = subprocess.Popen([BINARY, str(config)], cwd=ROOT, stdout=log, stderr=log)

    def connect():
        return socket.create_connection(("127.0.0.1", port), timeout=15)

    def request(path, body=b"", chunked=False, method="GET"):
        head = f"{method} {path} HTTP/1.1\r\nHost: localhost:{port}\r\nContent-Type: application/octet-stream\r\nX-Test: present\r\n".encode()
        if chunked:
            head += b"Transfer-Encoding: chunked\r\n"
            wire = b"".join(f"{len(body[i:i+4096]):x}\r\n".encode() + body[i:i+4096] + b"\r\n" for i in range(0, len(body), 4096)) + b"0\r\n\r\n"
        else:
            head += f"Content-Length: {len(body)}\r\n".encode()
            wire = body
        with connect() as sock:
            sock.sendall(head + b"\r\n" + wire)
            data = b""
            while True:
                part = sock.recv(65536)
                if not part:
                    break
                data += part
        head, content = data.split(b"\r\n\r\n", 1)
        status = int(head.split()[1])
        headers = dict(line.lower().split(b": ", 1) for line in head.split(b"\r\n")[1:])
        if b"content-length" in headers:
            assert int(headers[b"content-length"]) == len(content)
        return status, headers, content

    def check(path, status=200, body=b"", chunked=False, method="GET"):
        global checks
        result = request(path, body, chunked, method)
        assert result[0] == status, (path, result)
        checks += 1
        return result

    def children():
        path = pathlib.Path(f"/proc/{process.pid}/task/{process.pid}/children")
        return path.read_text().split() if path.exists() else []

    def wait_clean():
        for _ in range(150):
            if not children():
                return
            time.sleep(.02)
        raise AssertionError(("remaining children", children()))

    try:
        for _ in range(100):
            if process.poll() is not None:
                raise RuntimeError((directory / "server.log").read_text())
            try:
                with connect():
                    break
            except OSError:
                time.sleep(.02)
        result = json.loads(check("/cgi/echo.py/extra?x=1")[2])
        assert result["relative"] == "RELATIVE"
        env = result["env"]
        for key, value in {"SCRIPT_NAME": "/cgi/echo.py", "PATH_INFO": "/extra", "QUERY_STRING": "x=1", "REMOTE_ADDR": "127.0.0.1", "SERVER_PORT": str(port), "HTTP_X_TEST": "present", "CONTENT_LENGTH": "0"}.items():
            assert env[key] == value, (key, env.get(key))
        payload = bytes(range(256)) * 4096
        for chunked in (False, True):
            result = json.loads(check("/cgi/echo.py", body=payload, chunked=chunked, method="POST")[2])
            assert bytes.fromhex(result["body"]) == payload
            assert result["env"]["CONTENT_LENGTH"] == str(len(payload))
            assert "HTTP_TRANSFER_ENCODING" not in result["env"]
        result = json.loads(check("/cgi/")[2])
        assert result["env"]["SCRIPT_NAME"] == "/cgi/echo.py"
        assert bytes.fromhex(json.loads(check("/cgi/", body=b"index post", method="POST")[2])["body"]) == b"index post"
        assert check("/cgi/large.py", body=payload, method="POST")[2] == b"x" * 262144 + payload
        assert check("/cgi/binary.py")[2] == b"\x00\xffABC"
        assert check("/cgi/fragmented.py")[2] == b"FRAGMENTED"
        assert check("/cgi/lf.py")[2] == b"A\r\n\r\nB"
        assert check("/cgi/status.py", 201)[2] == b"CREATED"
        assert check("/cgi/status.py", 201, body=payload, method="POST")[2] == b"CREATED"
        assert check("/cgi/redirect.py", 302)[1][b"location"] == b"https://example.org/"
        assert check("/cgi/local.py", 302)[1][b"location"] == b"/"
        assert check("/cgi/empty.py", 204)[2] == b""
        assert check("/cgi/notmodified.py", 304)[2] == b""
        check("/cgi/absent.py", 404)
        check("/cgi/forbidden.py", 403)
        check("/readonly/echo.py", 405, method="POST")
        check("/redirect/echo.py", 302)
        check("/broken/badexec.py", 502)
        check("/missingbin/echo.py", 500)
        for name in ("invalid", "badstatus", "short", "duplicate", "missingtype", "control", "hugeheader", "crash", "excess"):
            check("/cgi/" + name + ".py", 502)
        with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
            slow = pool.submit(check, "/cgi/slow.py")
            time.sleep(.15)
            start = time.monotonic()
            assert check("/")[2] == b"STATIC"
            assert time.monotonic() - start < 1
            assert slow.result()[2] == b"SLOW"
            jobs = [pool.submit(check, "/cgi/echo.py", body=b"parallel", method="POST") for _ in range(16)]
            for job in jobs:
                assert bytes.fromhex(json.loads(job.result()[2])["body"]) == b"parallel"
        wait_clean()
        fd_count = len(list(pathlib.Path(f"/proc/{process.pid}/fd").iterdir()))
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
            jobs = [pool.submit(check, path, 504) for path in ("/cgi/timeout.py", "/cgi/closed.py")]
            for job in jobs:
                job.result()
        wait_clean()
        sock = connect()
        sock.sendall(b"GET /cgi/timeout.py HTTP/1.1\r\nHost: localhost\r\n\r\n")
        for _ in range(100):
            if children():
                break
            time.sleep(.01)
        assert children()
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))
        sock.close()
        wait_clean()
        checks += 1
        assert len(list(pathlib.Path(f"/proc/{process.pid}/fd").iterdir())) == fd_count
        assert check("/")[2] == b"STATIC"
        diagnostics = (directory / "server.log").read_text()
        assert "ERROR: AddressSanitizer" not in diagnostics and "runtime error:" not in diagnostics
        print(f"{checks} CGI checks passed; concurrency, binary input/output, timeouts, fd cleanup and process cleanup verified.")
    finally:
        for pid in children():
            try:
                os.kill(int(pid), signal.SIGKILL)
            except ProcessLookupError:
                pass
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        log.close()
