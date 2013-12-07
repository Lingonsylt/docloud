from django.contrib.auth.decorators import permission_required, login_required
from django.core.urlresolvers import reverse
from django.db import transaction
from django.http import Http404
from django.shortcuts import redirect, render
from core.forms.tags import AddUserForm, NewUserForm, NewTagForm
from core.models import UserTag, Tag
from core.views.auth import _createNewUser


def _lazyAddUserForm(request, usertags):
    """
    If the user is admin, it should be able to see all tags, including ones where it isn't subscribed
    Create a lazy list of UserTags based on all tags available for the organization, and all marked as "owned"
    """
    for usertag in usertags:
        def lazyCrateUserForm(usertag):
            def curry():
                add_user_form = AddUserForm(tag_id=usertag.tag.id)
                add_user_form.fields['user'].queryset = add_user_form.fields['user'].queryset. \
                    filter(organization = request.loggedin().organization).exclude(usertags__tag = usertag.tag)
                return add_user_form
            return curry
        setattr(usertag, "add_user_form", lazyCrateUserForm(usertag))
        yield usertag

def _getLazyTagCreatorUserTags(request):
    usertags = (UserTag(tag=tag, user=request.loggedin(), owns_tag = True)
                for tag in request.loggedin().organization.tags.all())
    return _lazyAddUserForm(request, usertags)

def _processNewUserForm(request):
    """
    Return a NewUserForm if the user is owner, and create a new user if POST
    Returns form=None if not owner and user=False if no user created
    Returns (form, user)
    """
    if request.loggedin().owner:
        if request.method == "POST":
            new_user_form = NewUserForm(request.POST)
            if new_user_form.is_active():
                if new_user_form.is_valid():
                    user, auth_user = _createNewUser(new_user_form.cleaned_data["email"], request.loggedin().organization)
                    return NewUserForm(), user
                else:
                    return new_user_form, False
            else:
                return NewUserForm(), False
        else:
            return NewUserForm(), False
    return None, False

def _processNewTagForm(request):
    """
    Return a NewTagForm if the user is admin, and create a Tag if POST
    Returns None if not admin
    """
    if request.loggedin().owner or request.loggedin().tag_creator:
        if request.method == "POST":
            new_tag_form = NewTagForm(request.POST, request = request)
            if new_tag_form.is_active():
                if new_tag_form.is_valid():
                    with transaction.atomic():
                        new_tag = Tag.objects.create(organization = request.loggedin().organization,
                                                     name = new_tag_form.cleaned_data["name"])
                        UserTag.objects.create(user = request.loggedin(), tag = new_tag, owns_tag = True)
                else:
                    return new_tag_form
        return NewTagForm(request=request)
    return None

@login_required
def delete_tag(request, org_slug, tag_id):
    """
    Delete the tag pointed to by tag_id redirecting to tags:organization when done
    """
    org = request.loggedin().organization
    if org_slug != org.slug:
        raise Http404()
    if request.loggedin().owner or request.loggedin().tag_creator:
        try:
            tag = Tag.objects.get(pk=tag_id)
            if tag.organization.id == request.loggedin().organization.id:
                tag.delete()
        except Tag.DoesNotExist:
            pass
    return redirect(reverse("tags:organization", args=(org.slug,)))

@login_required
def delete_usertag(request, org_slug, tag_id, user_id):
    """
    Delete the usertag pointed to by tag_id + user_id, redirecting to tags:organization when done
    """
    org = request.loggedin().organization
    if org_slug != org.slug:
        raise Http404()
    if request.loggedin().owner or request.loggedin().tag_creator:
        try:
            usertag = UserTag.objects.get(tag__id = tag_id, user__id = user_id)
            if usertag.tag.organization == request.loggedin().organization:
                allowed = True
                if not request.loggedin().owner and not request.loggedin().tag_creator:
                    loggedin_usertag = usertag.tag.usertags.get(user = request.loggedin())
                    if not loggedin_usertag.owns_tag:
                        allowed = False
                if allowed:
                    usertag.delete()
        except UserTag.DoesNotExist:
            pass
    return redirect(reverse("tags:organization", args=(org.slug,)))



@login_required
def organization(request, org_slug):
    """
    Serve a page listing all tags the user is subscribed to (or all if admin), along with other users subscribed
    to those tags. Show create/delete-functions for Tags, Users and UserTags if the loggedin is admin
    """
    # "Return" 404 if the url doesn't match the logged in users organization
    org = request.loggedin().organization
    if org_slug != org.slug:
        raise Http404()

    # If the user was redirected from the registration page, set "token" to the first installation uuid for downloading
    # TODO: This is temporary since we don't have a client yet
    token = False
    if request.GET.get("token", None) is not None:
        installations = list(request.loggedin().installations.all())
        if len(installations) == 1:
            token = installations[0].uuid

    # Create form for creating new user if the loggedin is the owner of the organization
    # Handles POST to the form and returns the user if created
    new_user_form, new_user_created = _processNewUserForm(request)

    # Create form for creating new tags if the loggedin is owner or tag_creator
    # Handles POST to the form and creates new tags
    new_tag_form = _processNewTagForm(request)

    if request.loggedin().owner or request.loggedin().tag_creator:
        # Since this user is admin, it should be able to see all tags, including ones where it isn't subscribed
        # Create a lazy list of UserTags based on all tags available for the organization, and all marked as "owned"
        usertags = _getLazyTagCreatorUserTags(request)

        # If the user has the ability to add users to tags, we check if a valid AddUserForm has been submitted
        # Then validate it against the list of users not already added to that tag and then add the user
        if request.method == "POST" and request.POST.get("add_user_tag_id") is not None:
            try:
                tag_id = int(request.POST["add_user_tag_id"])
                tag = Tag.objects.get(pk=tag_id)
                add_user_form = AddUserForm(request.POST, tag_id=tag.id)
                add_user_form.fields['user'].queryset = add_user_form.fields['user'].queryset. \
                    filter(organization = request.loggedin().organization).exclude(usertags__tag = tag)
                if add_user_form.is_valid():
                    UserTag.objects.create(user = add_user_form.cleaned_data["user"], tag = tag, owns_tag = False)
            except Tag.DoesNotExist:
                pass
    else:
        # Fetch all usertags that this user subscribes to
        usertags = request.loggedin().usertags.all()

    return render(request, "tags/organization.html", {"TITLE": org.name,
                                                        "org":org,
                                                        "usertags":usertags,
                                                        "token": token,
                                                        "new_tag_form":new_tag_form,
                                                        "new_user_form":new_user_form,
                                                        "new_user_created":new_user_created})