#include <gtk/gtk.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "config.h"
#include "common.h"

static GtkBuilder *builder;
static GtkTreeView *filetype_treeview;
static GtkListStore *filetype_liststore;

static GtkWindow *add_extension_win;

void
save_extensions()
{
	GtkTreeIter iter;
	std::string str;
	gchar *filetype_str;

	if (gtk_tree_model_get_iter_first (
		GTK_TREE_MODEL(filetype_liststore), &iter) != TRUE)
		return;

	do {
		gtk_tree_model_get(GTK_TREE_MODEL(filetype_liststore),
		    &iter,
		    1, &filetype_str, -1);

		str += filetype_str;
		str += ";";

		g_free(filetype_str);
	} while (gtk_tree_model_iter_next(
		GTK_TREE_MODEL(filetype_liststore), &iter) == TRUE);

	config::setStr("accepted_filetypes", str.c_str());
}

static void
apply_btn_cb(GtkWidget *widget,
             gpointer   data)
{
	save_extensions();
}

static void
load_filetypes_list(GtkBuilder *builder)
{
	GtkTreeIter it;
	std::vector<std::string> list;
	std::vector<std::string>::iterator type_it;
	int id = 1;

	list = split(config::getStr("accepted_filetypes"), ';');


	for (type_it = list.begin(); type_it != list.end(); type_it ++) {
		int sz;

		gtk_list_store_append(filetype_liststore, &it);
		gtk_list_store_set(filetype_liststore, &it, 
		    0, id++, 1, (*type_it).c_str(), -1);
	}
}

static void
add_extension_btn_cb(GtkWidget *widget,
		     gpointer data)
{
	gtk_widget_show(GTK_WIDGET(add_extension_win));

}


/* Callback for add-button on add_extention_window */
static void
add_extension_add_btn_cb(GtkWidget *widget,
		     gpointer data)
{
	GtkTreeIter it;
	GtkEntry *add_extension_entry;
	int id = 1;

	
	add_extension_entry = GTK_ENTRY(gtk_builder_get_object(builder,
		"add_extension_entry"));

	gtk_list_store_append(filetype_liststore, &it);
	gtk_list_store_set(filetype_liststore, &it, 
	    0, id++, 1, gtk_entry_get_text(add_extension_entry), -1);

	gtk_entry_set_text(add_extension_entry, "");
	gtk_widget_hide(GTK_WIDGET(add_extension_win));
}

/* Callback for close-button on add_extention_window */
static void
add_extension_close_btn_cb(GtkWidget *widget,
		     gpointer data)
{
	gtk_widget_hide(GTK_WIDGET(add_extension_win));

}

static void
rem_extension_btn_cb(GtkWidget *widget,
		    gpointer   data)
{
	GtkTreeSelection *sel;
	GtkTreeModel *model;
	GList *list;

	sel = gtk_tree_view_get_selection(filetype_treeview);
	list = gtk_tree_selection_get_selected_rows(sel, &model);

	for (; list != NULL; list = list->next) {
		GtkTreeIter  iter;

		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(filetype_liststore), &iter,
			(GtkTreePath*)list->data)) {
			gtk_list_store_remove(filetype_liststore, &iter);
		}

	}

	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (list);
}


int
main (int   argc,
      char *argv[])
{
	GObject *window;
	GObject *button;
	GtkTreeSelection *selection;

	gtk_init (&argc, &argv);

	/* Construct a GtkBuilder instance and load our UI description */
	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, "main.ui", NULL);

	/* Connect signal handlers to the constructed widgets. */
	window = gtk_builder_get_object (builder, "main_window");
	add_extension_win = GTK_WINDOW(gtk_builder_get_object (builder, "add_extension_window"));

	gtk_widget_show(GTK_WIDGET(window));

	filetype_treeview = GTK_TREE_VIEW(gtk_builder_get_object (builder,
	    "filetype_treeview"));
	filetype_liststore = GTK_LIST_STORE(gtk_builder_get_object (builder,
	    "filetype_liststore"));

	selection = gtk_tree_view_get_selection(filetype_treeview);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	printf("select mode: %d\n",gtk_tree_selection_get_mode(selection));

	button = gtk_builder_get_object (builder, "apply_btn");
	g_signal_connect (button, "clicked", G_CALLBACK (apply_btn_cb), NULL);

	button = gtk_builder_get_object (builder, "close_btn");
	g_signal_connect (button, "clicked", G_CALLBACK (gtk_main_quit), NULL);

	button = gtk_builder_get_object (builder, "add_extension_btn");
	g_signal_connect (button, "clicked", G_CALLBACK (add_extension_btn_cb), NULL);

	button = gtk_builder_get_object (builder, "rem_extension_btn");
	g_signal_connect (button, "clicked", G_CALLBACK (rem_extension_btn_cb), NULL);

	load_filetypes_list(builder);

	/* Setup callbacks for extension-window */

	button = gtk_builder_get_object (builder, "add_extension_add_btn");
	g_signal_connect (button, "clicked", G_CALLBACK (add_extension_add_btn_cb), NULL);

	button = gtk_builder_get_object (builder, "add_extension_close_btn");
	g_signal_connect (button, "clicked", G_CALLBACK (add_extension_close_btn_cb), NULL);

	gtk_main ();

	return 0;
}
