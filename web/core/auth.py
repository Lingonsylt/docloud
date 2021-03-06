import types
from core.models import User
from django.contrib.auth import get_user_model, authenticate, logout, login
from django.contrib.auth.models import User as AuthUser


def loggedin(self):
    if self.user.is_authenticated():
        user = getattr(self, "__loggedinuser", None)
        if user is None:
            try:
                user = self.user.docloud_users.get()
            except User.DoesNotExist:
                user = None
            setattr(self, "__loggedinuser", user)
        return user
    else:
        return None

class LoggedInMiddleware(object):
    def process_request(self, request):
        token = request.META.get("HTTP_X_DOCLOUD_TOKEN")
        if token:
            try:
                auth_user = AuthUser.objects.get(docloud_users__installations__uuid = token)
                auth_user = authenticate(auth_user=auth_user)
                logout(request)
                login(request, auth_user)
            except AuthUser.DoesNotExist:
                pass
        setattr(request, "loggedin", types.MethodType(loggedin, request))

        if token:
            request.loggedin().active_installation = request.loggedin().installations.get(pk=token)
        return None

class TokenLoginBackend(object):
    def authenticate(self, token):
        try:
            return AuthUser.objects.get(docloud_users__installations__uuid = token)
        except AuthUser.DoesNotExist:
            raise

    def get_user(self, user_id):
        UserModel = get_user_model()
        try:
            return UserModel._default_manager.get(pk=user_id)
        except UserModel.DoesNotExist:
            return None

class AutologinBackend(object):
    def authenticate(self, auth_user):
        return auth_user

    def get_user(self, user_id):
        UserModel = get_user_model()
        try:
            return UserModel._default_manager.get(pk=user_id)
        except UserModel.DoesNotExist:
            return None
