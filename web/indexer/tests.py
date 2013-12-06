import json
from django.test import TestCase
import mock
from indexer.views import index_query
from django.test.client import RequestFactory

def j(o):
    return json.dumps(o).encode("utf-8")

# Create your tests here.
class TestQuery(TestCase):
    @mock.patch("indexer.views.solr", autospec=True)
    def test_no_link(self, solr_mock):
        solr_mock.get.return_value = {"doc":{"links_ss":["C:\\file.txt"]}}
        request = RequestFactory().post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt"}]), content_type="application/json")
        response = index_query(request)
        self.assertFalse(solr_mock.update.called)
        self.assertEqual(response.content, j([{"hash":"abc", "index":True}]))

    @mock.patch("indexer.views.solr", autospec=True)
    def test_link(self, solr_mock):
        solr_mock.get.return_value = {"doc":{"links_ss":["C:\\file.txt"]}}
        request = RequestFactory().post('', data = json.dumps([{"hash":"abc", "path":"C:\\file2.txt"}]), content_type="application/json")
        response = index_query(request)
        self.assertTrue(solr_mock.update.called)
        self.assertEqual(response.content, j([{"hash":"abc", "index":True}]))

    @mock.patch("indexer.views.solr", autospec=True)
    def test_parsed(self, solr_mock):
        solr_mock.get.return_value = {"doc":{"links_ss":["C:\\file.txt"], "parsed_b":True}}
        request = RequestFactory().post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt"}]), content_type="application/json")
        response = index_query(request)
        self.assertEqual(response.content, j([{"hash":"abc", "index":False}]))

