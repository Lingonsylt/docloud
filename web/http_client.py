from functools import reduce
import io
import json
import requests
import hashlib
import sys

API_URL = "http://localhost:8000/"
TOKEN = "5c8af4945e8811e3ab5790e6ba0d52ef"

token_header = {"X-Docloud-Token": TOKEN}

add_file_infos = {}
delete_file_infos = {}

for line in sys.stdin:
    command, *args = line.strip().split(";")
    if command == "add":
        tags, filename = args
    elif command == "delete":
        filename = args[0]

    with open(filename, "rb") as f:
        data = f.read()
        h = hashlib.sha1(data).hexdigest()
        if command == "add":
            if h not in add_file_infos:
                add_file_infos[h] = {"paths":[], "data":data}
                add_file_infos[h]["paths"].append(
                    {"path": filename,
                     "tags": list(map(int, tags.split(",")))})
        elif command == "delete":
            if h not in delete_file_infos:
                delete_file_infos[h] = {"paths":[], "data":data}
                delete_file_infos[h]["paths"].append(filename)

if not add_file_infos and not delete_file_infos:
    print("No valid input! Shutting down")
    sys.exit(1)

print("Found %s unique files to add" % len(add_file_infos))
request_data = []
for h, d in add_file_infos.items():
    for path in d["paths"]:
        request_data.append({"hash":h, "path":path["path"], "tags": path["tags"]})

print("Requesting index info about %s paths" % request_data)

index_result = requests.post(API_URL + "index/query/",
                     data=json.dumps(request_data),
                     headers = dict({'content-type': 'application/json'}, **token_header)).json()
if not index_result["success"]:
    print("Index request failed! Shutting down")
    sys.exit(1)
index_result = index_result["results"]

num_index = reduce(lambda accum, item: accum + (1 if item["index"] else 0), index_result, 0)
print("Will index %s files, skipping %s" % (num_index, len(index_result) - num_index))

for index_request in index_result:
    if index_request["index"]:
        index_update_response = requests.post(API_URL +
                     "index/update/",
                     headers = token_header,
                     data = {"metadata":json.dumps({"hash":index_request["hash"]})},
                     files = {"file" : io.BytesIO(add_file_infos[index_request["hash"]]["data"])}).json()
        print("Indexed %s (%s): %s" % (index_request["hash"],
                                       add_file_infos[index_request["hash"]]["paths"], index_update_response))

request_data = []
for h, d in delete_file_infos.items():
    for path in d["paths"]:
        request_data.append({"hash":h, "path":path})

print("Found %s paths to delete" % len(delete_file_infos))

delete_result = requests.post(API_URL + "index/delete/",
                            data=json.dumps(request_data),
                            headers = dict({'content-type': 'application/json'}, **token_header)).json()
if not delete_result["success"]:
    print("Delete request failed! Shutting down")
    print(delete_result.content)
    sys.exit(1)

for h, d in delete_file_infos.items():
    for path in d["paths"]:
        print("Deleted %s (%s)" % (h, path))

print("Job well done!")