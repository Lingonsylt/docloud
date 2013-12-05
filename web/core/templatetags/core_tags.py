from django import template
from django.template import loader
from django.template.base import token_kwargs
from django.utils import six

register = template.Library()

@register.tag
def confirm_delete(parser, token):
    nodelist = parser.parse(('endconfirm_delete',))
    bits = token.split_contents()
    remaining_bits = bits[1:]
    extra_context = token_kwargs(remaining_bits, parser, support_legacy=True)
    parser.delete_first_token()
    return UpperNode(nodelist, extra_context)

class UpperNode(template.Node):
    def __init__(self, nodelist, extra_content):
        self.nodelist = nodelist
        self.extra_context = extra_content
        self.inclusion_template = loader.get_template("templatetags/_confirm_delete.html")
    def render(self, context):
        values = dict([(key, loader.Template(val.resolve(context)).render(context)) for key, val in
                       six.iteritems(self.extra_context)])
        context.update(values)
        context["message"] = self.nodelist.render(context)
        return self.inclusion_template.render(context)