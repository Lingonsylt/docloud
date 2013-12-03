import json
from django.conf import settings
from django.http import QueryDict
import requests

def get(id, params):
    if params is None:
        params = dict()
    params["wt"] = "json"
    params["id"] = id
    return requests.get(settings.SOLR_API_URL +
                 "collection1/get?" + _dict_to_querystring(params)).json()

def update(data, params = None):
    if params is None:
        params = dict()
    params["wt"] = "json"
    params["commit"] = "true"
    return requests.post(settings.SOLR_API_URL + "collection1/update?" + _dict_to_querystring(params),
                  data=json.dumps(data),
                  headers = {'content-type': 'application/json'}).json()

def query(q, params):
    if params is None:
        params = dict()
    params["wt"] = "json"
    params["q"] = q
    return requests.get(settings.SOLR_API_URL + "collection1/select?" + _dict_to_querystring(params)).json()

def extract(params, file):
    if params is None:
        params = dict()
    params["wt"] = "json"
    params["commit"] = "true"
    return requests.post(settings.SOLR_API_URL +
                                   "update/extract?" + _dict_to_querystring(params),
                                   files = {file.name : file}).json()

def deleteAll():
    requests.get(settings.SOLR_API_URL + "update?stream.body=%3Cdelete%3E%3Cquery%3E*:*%3C/query%3E%3C/delete%3E&commit=true")

def _dict_to_querystring(d):
    params = QueryDict("", mutable=True)
    if d is not None:
        for (k, v) in d.items():
            params.setlist(k, v if isinstance(v, list) else [v])
    return params.urlencode()


