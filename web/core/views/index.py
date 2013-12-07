from django.core.urlresolvers import reverse
from django.shortcuts import render, redirect


def index(request):
    if request.loggedin():
        return redirect(reverse("searcher:search"))
    return render(request, "index.html", {})