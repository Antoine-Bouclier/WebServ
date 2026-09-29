import json
import os
import sys

body = sys.stdin.buffer.read()
result = {
    "method": os.environ.get("REQUEST_METHOD"),
    "query": os.environ.get("QUERY_STRING"),
    "script": os.environ.get("SCRIPT_NAME"),
    "path_info": os.environ.get("PATH_INFO"),
    "content_length": os.environ.get("CONTENT_LENGTH"),
    "body": body.decode("utf-8", errors="replace"),
    "relative_file": open("message.txt").read().strip(),
}
print("Content-Type: application/json")
print()
print(json.dumps(result))
