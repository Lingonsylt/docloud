import os
import random
import requests

SEARCH_PATHS = ["C:\\Users\\lingon\\Desktop\\odt"]
FILE_EXTENSIONS = [".docx", ".odt", ".pdf", ".doc"]
API_URL = "http://localhost:8000/"
TOKEN = "5c8af4945e8811e3ab5790e6ba0d52ef"

token_header = {"X-Docloud-Token": TOKEN}

availble_tags = requests.get(API_URL + "index/tags/",
                             headers = token_header).json()
print("Commander: Available tags: %s, tagging with %s" % (availble_tags, ",".join([tag["name"] for tag in availble_tags])))

file_infos = []
for search_path in SEARCH_PATHS:
    for root, dirs, files in os.walk(search_path):
        for file in files:
            file = os.path.join(root, file)
            if os.path.splitext(file)[1] in FILE_EXTENSIONS:
                file_infos.append(
                    (file,
                     [tag["id"] for tag in
                              random.sample(availble_tags, random.randint(1, len(availble_tags)))]))

print("Commander: Found %s files" % len(file_infos))

with open("f.txt", "w") as f:
    for path, tags in file_infos:
        f.write("add;%s;%s\n" % (",".join(map(str, tags)), path))
        if random.random() > 0.4:
            f.write("delete;%s\n" % path)

print("Commander: Starting http_client.py")
os.system("C:\\Users\\lingon\\docloud\\Scripts\\python.exe http_client.py < f.txt")

"""
#p = subprocess.Popen()
p = subprocess.Popen(["C:\\Users\\lingon\\docloud\\Scripts\\python.exe", "http_client.py"], shell=True, bufsize=32, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
#(child_stdout, child_stdin) = (p.stdout, p.stdin)
#print("P", p)
#def gen():
tiow = TextIOWrapper(p.stdin)
#p.stdin.write(tiow)
for path, tags in file_infos:
    s = "add;%s;%s\n" % (",".join(map(str, tags)), path)
    print("WRITING TO STDIN: ", s)
    #s = (s)
    #print("s", s)
    tiow.write(s)
    tiow.writelines([s])

p.stdin.close()
print("READING FROM STDOUT")
print(p.stdout.read())
#stdout, stderr = p.communicate(gen())
#while True:
#    stdout.
"""