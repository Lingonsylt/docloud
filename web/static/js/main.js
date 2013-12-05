$(document).ready(function() {
    var popovers = $(".popover-target");
    popovers.popover();
    popovers.on('shown.bs.popover', function () {
        var popoveMaster = $(this);
        popoveMaster.next(".popover").find(".popover-cancel").click(function (evt) {
            evt.preventDefault();
            popoveMaster.popover('hide');
        });
    });

    var collapseIconToggle = function () {
        var collapseMaster = $($(this).data("collapse-master"));
        console.log(collapseMaster);
        console.log(collapseMaster.find(".glyphicon-plus"));
        if(collapseMaster.find(".glyphicon-plus").toggleClass("glyphicon-plus").toggleClass("glyphicon-minus").length === 0) {
            collapseMaster.find(".glyphicon-minus").toggleClass("glyphicon-plus").toggleClass("glyphicon-minus");
        }
    };
    var collapse = $(".collapse");
    collapse.on('show.bs.collapse', collapseIconToggle);
    collapse.on('hide.bs.collapse', collapseIconToggle);
})