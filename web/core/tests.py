from unittest import skip
from django.core.urlresolvers import reverse
from django.template import loader, Context
from django.test import TestCase
from core.models import User
from core.views.tags import organization
from web.utils import AuthorizedRequestFactory
import re

class TestDB(TestCase):
    def test_db_connection(self):
        self.assertEqual(list(User.objects.all()), [])

class TestOrganization(TestCase):
    fixtures = ["small.json"]

    def setUp(self):
        self.rfactory = AuthorizedRequestFactory()

    @skip
    def test_tag_forloop_id(self):
        self.assertEqual(len(self.rfactory._user.organization.tags.all()), 2)
        User.objects.get(email="test2@test.com").usertags.all().get().delete()
        for tag in self.rfactory._user.organization.tags.all():
            self.assertEqual(len(tag.usertags.all()), 1)

        request = self.rfactory.get(reverse("tags:organization", args=(self.rfactory._user.organization.slug,)))
        response = organization(request, self.rfactory._user.organization.slug)
        content = response.content.decode("utf-8")
        self.assertEqual(content.count("tag-item"), 2)
        tag_collapses = re.findall(r"[^\.]tag-collapse-(\d)", content)
        self.assertEqual(len(tag_collapses), 2)
        self.assertNotEqual(tag_collapses[0], tag_collapses[1])

    def test_confirm_delete_tag(self):
        confirm_delete_template = """
        {% confirm_delete title="Title" delete_url="/delete-url/" %}
            Body
        {% endconfirm_delete %}
        """

        for_template = """
        %s
        {%% for i in range %%}
            range-{{ i }}={{ forloop.counter0 }}
            {%% for n in shortrange %%}
                %s
            {%% endfor %%}
        {%% endfor %%}
        """
        ctx = Context({"range" : range(5), "shortrange": [1]})
        for match in re.findall(r"(\d)=(\d)", loader.Template(for_template % ("", "")).render(ctx)):
            self.assertEqual(match[0], match[1])

        for match in re.findall(r"(\d)=(\d)", loader.Template(for_template % ("{% load core_tags %}", confirm_delete_template)).render(ctx)):
            print(match)
            self.assertEqual(match[0], match[1])
