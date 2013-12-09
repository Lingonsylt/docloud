import json
import types
from django.test import TestCase
import mock
from core.models import User
from indexer.views import index_query
from django.test.client import RequestFactory

def j(o):
    return json.dumps(o).encode("utf-8")

class AuthorizedRequestFactory(RequestFactory):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._user = User.objects.get(email="test@test.com")

    def request(self, **request):
        r = super().request(**request)
        setattr(r, "user", self._user.auth_user)
        setattr(r, "_user", self._user)
        def loggedin(self):
            return self._user
        setattr(r, "loggedin", types.MethodType(loggedin, r))
        return r

# Create your tests here.
class TestQuery(TestCase):
    fixtures = ["small.json"]

    def setUp(self):
        self.rfactory = AuthorizedRequestFactory()

    def _getCallArgs(self, func):
        results = []
        for call in func.call_args_list:
            if len(call) == 2:
                args, kwargs = call
            else:
                name, args, kwargs = call
            results.append({"args":args, "kwargs": kwargs})
        return results

    def assertNotCalledWith(self, func, *args, **kwargs):
        for called_args, called_kwargs in self._getCallArgs(func):
            called = True
            for arg in args:
                if arg not in called_args:
                    called = False

            for key, value in kwargs:
                if key not in called_kwargs or kwargs[key] != called_kwargs[key]:
                    called = False
            if called:
                raise AssertionError("%s called with %s" % (func, (args, kwargs)))

    @mock.patch("indexer.views.solr", autospec=True)
    def test_no_link(self, solr_mock):
        solr_mock.get.return_value = {"doc":{"links_ss":["C:\\file.txt"], "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}}
        solr_mock.update.return_value = {"responseHeader":{"status":0}}

        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt", "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}]), content_type="application/json")
        response = index_query(request)

        called_args = self._getCallArgs(solr_mock.update)
        self.assertEqual(len(called_args), 1)
        self.assertGreaterEqual(len(called_args[0]["args"][0]), 1)
        for document in called_args[0]["args"][0]:
            self.assertFalse("links_ss" in document)

        self.assertEqual(response.content, j({"results":[{"hash":"abc", "index":True}], "success": True}))

    @mock.patch("indexer.views.solr", autospec=True)
    def test_link(self, solr_mock):
        solr_mock.get.return_value = {"doc":{"links_ss":["C:\\file.txt"], "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}}
        solr_mock.update.return_value = {"responseHeader":{"status":0}}

        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file2.txt", "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}]), content_type="application/json")
        response = index_query(request)

        called_args = self._getCallArgs(solr_mock.update)
        self.assertEqual(len(called_args), 1)
        self.assertGreaterEqual(len(called_args[0]["args"][0]), 1)
        link_updated = False
        for document in called_args[0]["args"][0]:
            if "links_ss" in document:
                link_updated = True
                break
        self.assertTrue(link_updated)

        self.assertEqual(response.content, j({"results":[{"hash":"abc", "index":True}], "success":True}))

    @mock.patch("indexer.views.solr", autospec=True)
    def test_parsed(self, solr_mock):
        solr_mock.get.return_value = {"doc":{"links_ss":["C:\\file.txt"], "parsed_b":True, "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}}
        solr_mock.update.return_value = {"responseHeader":{"status":0}}

        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt", "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}]), content_type="application/json")
        response = index_query(request)
        self.assertEqual(response.content, j({"results":[{"hash":"abc", "index":False}], "success":True}))

