from django import forms
from django.core.exceptions import ValidationError
from core.models import User, Tag
from web.forms import ActivatedFormMixin, RequestRequiredMixin
from django.utils.translation import ugettext as _

class NewUserForm(ActivatedFormMixin, forms.Form):
    form_id_str = "new_user_form"
    email = forms.EmailField()

    def clean_email(self):
        if len(User.objects.filter(email = self.cleaned_data["email"])) > 0:
            raise ValidationError(
                _('En användare med emailen %(value)s finns redan'),
                code='invalid',
                params={'value': self.cleaned_data["email"]},
                )
        return self.cleaned_data["email"]

class NewTagForm(RequestRequiredMixin, ActivatedFormMixin, forms.Form):
    name = forms.CharField(max_length=512, label="Ny tagg", required=True)
    form_id_str = "new_tag_form"

    def clean_name(self):
        if len(Tag.objects.filter(organization = self.request.loggedin().organization, name = self.cleaned_data["name"])) > 0:
            raise ValidationError(
                _('En tagg med namnet %(value)s finns redan'),
                code='invalid',
                params={'value': self.cleaned_data["name"]},
                )
        return self.cleaned_data["name"]

class AddUserForm(forms.Form):
    user = forms.ModelChoiceField(queryset=User.objects.all(), empty_label="Välj användare")
    add_user_tag_id = forms.CharField(widget=forms.HiddenInput())

    def __init__(self, *args, tag_id = None, **kwargs):
        if tag_id is None:
            raise ValueError("tag_id can not be None!")
        super().__init__(*args, **kwargs)
        self.fields["add_user_tag_id"].initial = str(tag_id)