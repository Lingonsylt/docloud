import json
import tempfile
from django.test import TestCase
import mock
from core.models import FileInfo
import indexer
from indexer.views import index_query, index_delete, index_update
from web.utils import AuthorizedRequestFactory, j, jd


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
    def test_query_new(self, solr_mock):
        solr_mock.update.return_value = {"responseHeader":{"status":0}}

        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt", "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}]), content_type="application/json")
        response = index_query(request)
        solr_mock.update.assert_called_once_with([{"id":"abc",
                                            "links_ss":{"set":["C:\\file.txt"]},
                                            "tags_is":{"set":[self.rfactory._user.tags.get(name="test-ab-tag").id]}}])
        self.assertFalse(solr_mock.delete.called)
        self.assertTrue(jd(response.content)["success"])


    @mock.patch("indexer.views.solr", autospec=True)
    def test_query_existing(self, solr_mock):
        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt", "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}]), content_type="application/json")
        fi = FileInfo.objects.create(hash="abc",
                                installation=self.rfactory._user.active_installation,
                                path="C:\\file.txt")
        fi.tags = [self.rfactory._user.tags.get(name="test-ab-tag").id]
        fi.save()

        response = index_query(request)
        self.assertFalse(solr_mock.update.called)
        self.assertFalse(solr_mock.delete.called)
        self.assertTrue(jd(response.content)["success"])

    @mock.patch("indexer.views.solr", autospec=True)
    def test_query_parsed(self, solr_mock):
        solr_mock.get.return_value = {"doc":{"parsed_b":True}}
        solr_mock.update.return_value = {"responseHeader":{"status":0}}

        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt", "tags":[self.rfactory._user.tags.get(name="test-ab-tag").id]}]), content_type="application/json")
        response = index_query(request)
        self.assertEqual(response.content, j({"results":[{"hash":"abc", "index":False}], "success":True}))

    @mock.patch("indexer.views.solr", autospec=True)
    def test_delete_last(self, solr_mock):
        solr_mock.delete.return_value = {"responseHeader":{"status":0}}
        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt"}]), content_type="application/json")
        fi = FileInfo.objects.create(hash="abc",
                                     installation=self.rfactory._user.active_installation,
                                     path="C:\\file.txt")
        fi.tags = [self.rfactory._user.tags.get(name="test-ab-tag").id]
        fi.save()

        response = index_delete(request)
        self.assertFalse(solr_mock.update.called)
        solr_mock.delete.assert_called_once_with("abc")
        self.assertTrue(jd(response.content)["success"])

    @mock.patch("indexer.views.solr", autospec=True)
    def test_delete_not_last(self, solr_mock):
        solr_mock.update.return_value = {"responseHeader":{"status":0}}

        fi = FileInfo.objects.create(hash="abc",
                                     installation=self.rfactory._user.active_installation,
                                     path="C:\\file.txt")
        fi.tags = [self.rfactory._user.tags.get(name="test-ab-tag").id]
        fi.save()

        fi2 = FileInfo.objects.create(hash="abc",
                                     installation=self.rfactory._user.active_installation,
                                     path="C:\\file2.txt")
        fi2.tags = [self.rfactory._user.tags.get(name="test-ab-tag").id]
        fi2.save()

        request = self.rfactory.post('', data = json.dumps([{"hash":"abc", "path":"C:\\file.txt"}]), content_type="application/json")
        response = index_delete(request)
        self.assertFalse(solr_mock.delete.called)
        solr_mock.update.assert_called_once_with([{"id":"abc",
                                                   "links_ss":{"set":["C:\\file2.txt"]},
                                                   "tags_is":{"set":[self.rfactory._user.tags.get(name="test-ab-tag").id]}}])
        self.assertTrue(jd(response.content)["success"])

    @mock.patch("indexer.views.solr", autospec=True)
    @mock.patch("indexer.views.open", mock.mock_open(), create=True)
    def test_index(self, solr_mock):
        solr_mock.extract.return_value = {"responseHeader":{"status":0}}
        solr_mock.update.return_value = {"responseHeader":{"status":0}}
        tdir = tempfile.gettempdir()
        file = tempfile.NamedTemporaryFile(suffix=".file", dir=tdir)
        file.write(b'hello')
        file.seek(0)
        request = self.rfactory.post('', data = {"metadata": j({"hash": "abc"}).decode("utf-8"), "file": file})
        fi = FileInfo.objects.create(hash="abc",
                                     installation=self.rfactory._user.active_installation,
                                     path="C:\\file.txt")
        fi.tags = [self.rfactory._user.tags.get(name="test-ab-tag").id]
        fi.save()

        response = index_update(request)
        self.assertTrue(indexer.views.open().write.called)
        solr_mock.extract.assert_called_once_with({"literal.parsed_b": "true",
                                                   "literal.id": "abc",
                                                   "uprefix": "attr_",
                                                   "fmap.content": "content_txt"},
                                                   mock.ANY)
        solr_mock.update.assert_called_once_with([{"id":"abc",
                                                   "links_ss":{"set":["C:\\file.txt"]},
                                                   "tags_is":{"set":[self.rfactory._user.tags.get(name="test-ab-tag").id]}}])
        self.assertTrue(jd(response.content)["success"])