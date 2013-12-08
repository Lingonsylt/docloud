from functools import reduce
import io
import json
import os
import random
import requests
import hashlib
import sys

SEARCH_PATHS = ["C:\\Users\\lingon\\Desktop\\odt"]
FILE_EXTENSIONS = [".docx", ".odt", ".pdf", ".doc"]
API_URL = "http://localhost:8000/"
TOKEN = "5c8af4945e8811e3ab5790e6ba0d52ef"

token_header = {"X-Docloud-Token": TOKEN}

availble_tags = requests.get(API_URL + "index/tags/",
                     headers = token_header).json()
print("Available tags: %s, tagging with %s" % (availble_tags, ",".join([tag["name"] for tag in availble_tags])))

file_infos = {}
for search_path in SEARCH_PATHS:
    for root, dirs, files in os.walk(search_path):
        for file in files:
            file = os.path.join(root, file)
            if os.path.splitext(file)[1] in FILE_EXTENSIONS:
                with open(file, "rb") as f:
                    data = f.read()
                    h = hashlib.sha1(data).hexdigest()
                    if h not in file_infos:
                        file_infos[h] = {"paths":[], "data":data}
                    file_infos[h]["paths"].append(
                        {"path":file,
                         "tags": [tag["id"] for tag in
                                  random.sample(availble_tags, random.randint(1, len(availble_tags)))]})

print("Found %s unique files" % len(file_infos))

request_data = []
for h, d in file_infos.items():
    for path in d["paths"]:
        request_data.append({"hash":h, "path":path["path"], "tags": path["tags"]})

print("Requesting index info about %s paths" % len(request_data))

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
                     files = {"file" : io.BytesIO(file_infos[index_request["hash"]]["data"])}).json()
        print("Indexed %s (%s): %s" % (index_request["hash"],
                                       file_infos[index_request["hash"]]["paths"], index_update_response))