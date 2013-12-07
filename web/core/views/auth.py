import uuid
from django.contrib.auth import authenticate, logout, login
from django.contrib.auth.decorators import login_required
from django.contrib.auth.models import User as AuthUser, Group, Permission
from django.contrib.contenttypes.models import ContentType
from django.core.urlresolvers import reverse
from django.db import transaction
from django.http import HttpResponse
from django.shortcuts import redirect, render
from core.forms.auth import NewUserAndOrganizationForm, ChangeEmailForm, ChangePasswordForm, LoginForm
from core.models import Installation, Organization, User

def _createNewUser(email, org, owner=False):
    with transaction.atomic():
        auth_user = AuthUser.objects.create_user(email,
                                                 email, uuid.uuid1().hex)

        user = User.objects.create(name="", email=email,
                                   organization = org, auth_user = auth_user, owner=owner)
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

def auth_register(request):
    register_form = NewUserAndOrganizationForm(request.POST)
    if register_form.is_valid():
        with transaction.atomic():
            org = Organization.objects.create(name=register_form.cleaned_data["organization_name"])
            user, auth_user = _createNewUser(register_form.cleaned_data["email"], org, owner=True)
            logout(request)
            login(request, auth_user)
        return redirect(reverse("tags:organization", args=(org.slug,)) + "?token")
    return _auth(request, register_form=register_form)

def auth_login(request):
    login_form = LoginForm(request.POST)
    if login_form.is_valid():
        auth_user = authenticate(username=login_form.cleaned_data["email"], password=login_form.cleaned_data["password"])
        logout(request)
        login(request, auth_user)
        return redirect(reverse("searcher:search"))
    return _auth(request, login_form=login_form)

def auth(request):
    return _auth(request)

def _auth(request, register_form=None, login_form=None):
    if not register_form:
        register_form = NewUserAndOrganizationForm()
    if not login_form:
        login_form = LoginForm()
    return render(request, "auth/auth.html", {"TITLE":"Registera / Logga in",
                                                 "register_form": register_form,
                                                 "login_form": login_form})

@login_required
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
    auth_user = authenticate(token = token)
    if auth_user:
        logout(request)
        login(request, auth_user)
        return redirect(reverse("tags:organization", args=(auth_user.docloud_users.get().organization.slug,)))
    else:
        return render(request, "auth/login_failed.html", {"TITLE":"Inloggningen misslyckades"})

@login_required
def profile_change_email(request):
    change_email_form = ChangeEmailForm(request.POST, request=request)
    if change_email_form.is_valid():
        with transaction.atomic():
            request.loggedin().email = change_email_form.cleaned_data["email"]
            request.user.email = change_email_form.cleaned_data["email"]
            request.user.username = change_email_form.cleaned_data["email"]
            request.loggedin().save()
            request.user.save()
        return _profile(request, email_changed=True)
    return _profile(request, change_email_form=change_email_form)

@login_required
def profile_change_password(request):
    change_password_form = ChangePasswordForm(request.POST, request=request)
    if change_password_form.is_valid():
        with transaction.atomic():
            request.loggedin().has_password = True
            request.loggedin().save()
            request.user.set_password(change_password_form.cleaned_data["password"])
            request.user.save()
        return _profile(request, password_changed=True)
    return _profile(request, change_password_form=change_password_form)

@login_required
def profile(request):
    return _profile(request)

def _profile(request, change_email_form=None, email_changed=False, change_password_form=None, password_changed=False):
    if not change_email_form:
        change_email_form = ChangeEmailForm(request=request)
    if not change_password_form:
        change_password_form = ChangePasswordForm(request=request)
    return render(request, "auth/profile.html", {"TITLE":"Profil",
                                                 "change_email_form": change_email_form,
                                                 "email_changed": email_changed,
                                                 "change_password_form": change_password_form,
                                                 "password_changed": password_changed})

