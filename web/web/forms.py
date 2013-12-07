from django import forms


class ActivatedFormMixin(forms.Form):
    form_id = forms.CharField(widget=forms.HiddenInput())

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if not hasattr(self, "form_id_str"):
            raise AttributeError("form_id_str is required for ActivatedFormMixin siblings!")
        self.fields["form_id"].initial = self.form_id_str

    def is_active(self):
        return self.data.get("form_id") == self.form_id_str

class RequestRequiredMixin(forms.Form):
    def __init__(self, *args, request = None, **kwargs):
        if request is None:
            raise ValueError("Request can not be None!")
        self.request = request
        super().__init__(*args, **kwargs)