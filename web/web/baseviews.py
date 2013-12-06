class PageTitleMixin:
    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context['TITLE'] = getattr(self, "title", "")
        return context

