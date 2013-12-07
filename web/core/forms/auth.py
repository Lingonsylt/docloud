from django import forms
from django.contrib.auth import authenticate
from django.core.exceptions import ValidationError
from django.utils.text import slugify
from django.utils.translation import ugettext as _
from core.models import Organization, User
from web.forms import RequestRequiredMixin


class CleanEmailMixin(forms.Form):
    def clean_email(self):
        if len(User.objects.filter(email = self.cleaned_data["email"])) > 0:
            raise ValidationError(
                _('En användare med emailen %(value)s finns redan'),
                code='invalid',
                params={'value': self.cleaned_data["email"]},
                )
        return self.cleaned_data["email"]

class CurrentPasswordMixin(RequestRequiredMixin, forms.Form):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if not self.request.loggedin().has_password:
            del self.fields["current_password"]

    def clean_current_password(self):
        auth_user = authenticate(username=self.request.loggedin().email, password=self.cleaned_data["current_password"])
        if not auth_user:
            raise ValidationError(
                _('Lösenordet stämmer inte'),
                code='invalid',
                )
        return self.cleaned_data["current_password"]

class NewUserAndOrganizationForm(CleanEmailMixin, forms.Form):
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

class ChangePasswordForm(CurrentPasswordMixin, RequestRequiredMixin, forms.Form):
    current_password = forms.CharField(widget=forms.PasswordInput(), required=True, label="Ditt nuvarande lösenord")
    password = forms.CharField(widget=forms.PasswordInput(), required=True, label="Nytt lösenord")
    password_again = forms.CharField(widget=forms.PasswordInput(), required=True, label="Nytt lösenord (igen)")

    def clean(self):
        cleaned_data = super().clean()
        if self._errors:
            return cleaned_data
        if cleaned_data["password"] != cleaned_data["password_again"]:
            raise ValidationError(
                _('De två lösenorden matchar inte'),
                code='invalid',
                )
        return cleaned_data

class ChangeEmailForm(CurrentPasswordMixin, RequestRequiredMixin, CleanEmailMixin, forms.Form):
    email = forms.EmailField(required=True, label="Ny email")
    email_again = forms.EmailField(required=True, label="Ny email (igen)")
    current_password = forms.CharField(widget=forms.PasswordInput(), required=True, label="Lösenord")

    def clean(self):
        cleaned_data = super().clean()
        if self._errors:
            return cleaned_data
        if cleaned_data["email"] != cleaned_data["email_again"]:
            self._errors["email_again"] = self.error_class(["Emailen stämmer inte överrens med den första"])
            del cleaned_data["email_again"]
        return cleaned_data

class LoginForm(forms.Form):
    email = forms.EmailField(required=True, label="Ny email")
    password = forms.CharField(widget=forms.PasswordInput(), required=True, label="Lösenord")

    def clean(self):
        cleaned_data = super().clean()
        if self._errors:
            return cleaned_data
        if not authenticate(username=cleaned_data["email"], password=cleaned_data["password"]):
            raise ValidationError(
                    _('Ingen användare hittades med denna email och detta lösenord'),
                    code='invalid',
                    )
        return cleaned_data