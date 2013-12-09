import json
import types
from django.test import RequestFactory
from core.models import User

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
