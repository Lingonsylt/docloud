from django.test import TestCase
import mock
from searcher.views import search
from web.utils import AuthorizedRequestFactory

class TestQuery(TestCase):
    fixtures = ["small.json"]

    def setUp(self):
        self.rfactory = AuthorizedRequestFactory()

    @mock.patch("searcher.views.solr", autospec=True)
    def test_query(self, solr_mock):
        solr_mock.query.return_value = {"responseHeader":
                                          {"status":0},
                                        "response":
                                           {"docs":
                                                [{"id":"doc_id1",
                                                  "links_ss":["C:\\file.txt"],
                                                  "tags_is":[self.rfactory._user.tags.get(name="test-ab-tag").id],
                                                  "content_txt":"Hello content!"}]},
                                        "highlighting":
                                           {"doc_id1":
                                                {"content_txt":["<em>Hello</em> content!"]}}}

        request = self.rfactory.get('/?q=hello')
        response = search(request)
        self.assertContains(response, "<em>Hello</em> content!")
        self.assertContains(response, "file.txt")
        self.assertContains(response, self.rfactory._user.tags.get(name="test-ab-tag").name)

    @mock.patch("searcher.views.solr", autospec=True)
    def test_missed_query(self, solr_mock):
        solr_mock.query.return_value = {"responseHeader":
                                            {"status":0},
                                        "response":
                                            {"docs": []},
                                        "highlighting":{}}

        request = self.rfactory.get('/?q=asdf')
        response = search(request)
        self.assertNotContains(response, "<em>Hello</em> content!")
        self.assertNotContains(response, "file.txt")
        self.assertNotContains(response, self.rfactory._user.tags.get(name="test-ab-tag").name)

