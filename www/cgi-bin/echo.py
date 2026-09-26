import os
import sys

body = sys.stdin.read()

print("Content-Type: text/plain")
print()
print("Methode :", os.environ.get("REQUEST_METHOD", ""))
print("Query :", os.environ.get("QUERY_STRING", ""))
print("Corps :", body)
