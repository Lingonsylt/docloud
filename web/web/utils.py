import json
import types
from django.test import RequestFactory
from django.test.runner import DiscoverRunner
from core.models import User
from teamcity import underTeamcity as is_running_under_teamcity
from teamcity.unittestpy import TeamcityTestRunner

def j(o):
    return json.dumps(o).encode("utf-8")

def jd(s):
    return json.loads(s.decode("utf-8"))

class AuthorizedRequestFactory(RequestFactory):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._user = User.objects.get(email="test@test.com")
        self._user.active_installation = self._user.installations.all()[:1][0]

    def request(self, **request):
        r = super().request(**request)
        setattr(r, "user", self._user.auth_user)
        setattr(r, "_user", self._user)

        def loggedin(self):
            return self._user
        setattr(r, "loggedin", types.MethodType(loggedin, r))
        return r

class TeamCityTestRunner(DiscoverRunner):
    def run_suite(self, suite, **kwargs):
        if is_running_under_teamcity():
            return TeamcityTestRunner().run(suite)
        else:
            return super().run_suite(suite)
