from django.conf import settings
from django.shortcuts import render
from django.http import HttpResponse
import os
from web import solr

def search(request):
    query = request.GET.get('q', "")
    delete = query.find("#delete") != -1
    if delete:
        query = query.replace("#delete", "")

    docs = []
    if query is not "":
        query_response = solr.query("content_txt:%s" % query, {"hl":"true",
                                                               "hl.fl":"content_txt",
                                                               "fl":["id", "links_ss"],
                                                               "hl.fragsize" :"200"})
        highlights = query_response["highlighting"]
        docs = query_response["response"]["docs"]
        for doc in docs:
            doc["highlight"] = highlights[doc["id"]]["content_txt"][0]
            doc['links'] = []
            for link in doc["links_ss"]:
                doc['links'].append({"path": os.path.dirname(link) + ("/" if link.find("/") != -1 else "\\"),
                                     "filename" : os.path.basename(link)})

    if delete:
        solr.deleteAll()

    return render(request, "searcher/search.html", {'query': query, 'docs': docs})

def download(request, hash):
    f = open(os.path.join(settings.DATA_STORAGE_DIR, hash), "rb")
    response = HttpResponse(f, content_type="application/octet-stream")
    response['Content-Disposition'] = 'attachment; filename="%s"' % request.GET["filename"]
    return response