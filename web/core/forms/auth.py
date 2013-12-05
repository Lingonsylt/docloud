from django import forms
from django.core.exceptions import ValidationError
from django.utils.text import slugify
from django.utils.translation import ugettext as _
from core.models import Organization

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