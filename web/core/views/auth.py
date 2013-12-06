import uuid
from django.contrib.auth import authenticate, logout, login
from django.contrib.auth.models import User as AuthUser, Group, Permission
from django.contrib.contenttypes.models import ContentType
from django.core.urlresolvers import reverse
from django.db import transaction
from django.http import HttpResponse
from django.shortcuts import redirect, render
from django.views.generic import FormView
from core.forms.auth import NewUserAndOrganizationForm
from core.models import Installation, Organization, User
from web.baseviews import PageTitleMixin


def _createNewUser(email, org):
    with transaction.atomic():
        auth_user = AuthUser.objects.create_user(email,
                                                 email, uuid.uuid1().hex)

        user = User.objects.create(name="", email=email,
                                   organization = org, auth_user = auth_user, owner=True)
        Installation.objects.create(user = user)
        auth_user = authenticate(auth_user=auth_user)
        customer_group, created = Group.objects.get_or_create(name = "customer")
        content_type = ContentType.objects.get_for_model(User)
        is_customer_permission, created = Permission.objects.get_or_create(codename='is_customer',
                                                                  name='Is customer',
                                                                  content_type=content_type)
        customer_group.permissions=[is_customer_permission]
        auth_user.groups.add(customer_group)
        auth_user.save()
    return user, auth_user

class RegisterView(PageTitleMixin, FormView):
    """
    Show a registration form asking for email and organization name
    Create a new User and Organization and log in the user, redirecting to tags:organization
    """
    title = "Starta docloud!"
    template_name = 'auth/register.html'
    form_class = NewUserAndOrganizationForm
    success_url = None

    def form_valid(self, form):
        with transaction.atomic():
            org = Organization.objects.create(name=form.cleaned_data["organization_name"])
            user, auth_user = _createNewUser(form.cleaned_data["email"], org)
            logout(self.request)
            login(self.request, auth_user)
            self.success_url = reverse("tags:organization", args=(org.slug,)) + "?token"
        return super().form_valid(form)

def link_download(request, token):
    """
    Return a docloud.url file pointing to the loginpage using token
    """
    response_str = """[InternetShortcut]
URL=%s
""" % request.build_absolute_uri(reverse("auth:token_login", args=(token,)))
    response = HttpResponse(response_str, content_type="application/octet-stream")
    response['Content-Disposition'] = 'attachment; filename="docloud.url"'
    return response

def token_login(request, token):
    """
    Login a user based on a token
    Relies on core.auth.TokenLoginBackend
    """
    logout(request)
    auth_user = authenticate(token = token)
    login(request, auth_user)
    if auth_user:
        return redirect(reverse("tags:organization", args=(auth_user.docloud_users.get().organization.slug,)))
    else:
        return render(request, "auth/login_failed.html", {"TITLE":"Inloggningen misslyckades"})