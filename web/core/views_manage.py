import uuid
from django.contrib.auth import login, authenticate, logout
from django.contrib.auth.decorators import permission_required
from django.contrib.auth.models import User as AuthUser
from django.core.exceptions import ValidationError
from django.core.urlresolvers import reverse, reverse_lazy

# Create your views here.
from django.db import transaction
from django.http import Http404, HttpResponse
from django.shortcuts import render, redirect
from django.utils.text import slugify
from django.views.generic import FormView

from django import forms
from core.models import Organization, User, customer_group, Installation
from web.baseviews import PageTitleMixin

from django.utils.translation import ugettext_lazy as _

class NewUserAndOrganizationForm(forms.Form):
    email = forms.EmailField(required=True, label="Din email")
    organization_name = forms.CharField(required=True, label="Organisationsnamn", max_length=512)

    def clean_organization_name(self):
        if len(Organization.objects.filter(slug=slugify(self.cleaned_data["organization_name"]))) > 0:
            raise ValidationError(
                _('En organisation med namnet %(value)s finns redan'),
                code='invalid',
                params={'value': self.cleaned_data["organization_name"]},
                )
        return self.cleaned_data["organization_name"]

class RegisterView(PageTitleMixin, FormView):
    title = "Starta docloud!"
    template_name = 'manage/register.html'
    form_class = NewUserAndOrganizationForm
    success_url = None

    def form_valid(self, form):
        with transaction.atomic():
            org = Organization.objects.create(name=form.cleaned_data["organization_name"])
            auth_user = AuthUser.objects.create_user(form.cleaned_data["email"],
                                                     form.cleaned_data["email"], uuid.uuid1().hex)

            user = User.objects.create(name="", email=form.cleaned_data["email"],
                                organization = org, auth_user = auth_user, owner=True)
            Installation.objects.create(user = user)
            auth_user = authenticate(auth_user=auth_user)
            auth_user.groups.add(customer_group)
            auth_user.save()
            logout(self.request)
            login(self.request, auth_user)
            self.success_url = reverse("manage:organization", args=(org.slug,)) + "?token"
        return super().form_valid(form)

@permission_required('core.is_customer', login_url="/inloggning/")
def organization(request, org_slug):
    org = request.loggedin().organization
    if org_slug != org.slug:
        raise Http404()

    token = False
    if request.GET.get("token", None) is not None:
        installations = list(request.loggedin().installations.all())
        if len(installations) == 1:
            token = installations[0].uuid

    usertags = request.loggedin().usertag_set.all()
    return render(request, "manage/organization.html", {"TITLE": org.name,
                                                        "org":org,
                                                        "usertags":usertags,
                                                        "token": token})

def link_download(request, token):
    response_str = """[InternetShortcut]
URL=%s
""" % request.build_absolute_uri(reverse("manage:token_login", args=(token,)))
    response = HttpResponse(response_str, content_type="application/octet-stream")
    response['Content-Disposition'] = 'attachment; filename="docloud.url"'
    return response

def token_login(request, token):
    logout(request)
    auth_user = authenticate(token = token)
    login(request, auth_user)
    if auth_user:
        return redirect(reverse("manage:organization", args=(auth_user.docloud_users.get().organization.slug,)))
    else:
        return render(request, "manage/login_failed.html", {"TITLE":"Inloggningen misslyckades"})