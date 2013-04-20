/*
 * cashflo-qp.c - Calculate interest lost vs interest gaine
 *
 * Copyright (C) 2012-2013	OpenTech Labs
 * 				Andrew Clayton <andrew@digital-domain.net>
 *
 * Released under the GNU General Public License version 2.
 * See COPYING
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include <glib.h>

#include <gtk/gtk.h>

#define VERSION		"001"

struct widgets {
	GtkWidget *window;
	GtkWidget *about;
	GtkWidget *readme;
	GtkWidget *amount_entry;
	GtkWidget *ir_entry;
	GtkWidget *itype;
	GtkWidget *pt_entry;
	GtkWidget *apt_entry;
	GtkWidget *disc_entry;
	GtkTextBuffer *buffer;
	GtkTextBuffer *readme_buffer;
};

static void do_calc(struct widgets *widgets)
{
	double amount;
	double ir;
	double disc;
	float interest;
	float cpt;
	int period;
	int aperiod;
	const char *itype;
	const char *fmt;
	char line[128];
	GtkTextIter iter;

	amount = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->amount_entry)),
			NULL);
	ir = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->ir_entry)), NULL);
	itype = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(
				widgets->itype));
	period = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->pt_entry)));
	aperiod = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->apt_entry)));
	disc = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->disc_entry)),
			NULL);

	if (strcmp(itype, "EAR") == 0) {
		/*
		 * Calculate EAR interest and the crossover point with the MSC
		 * We divide by 365, to get the daily interest rate
		 */
		interest = amount * (ir / 365 / 100.0) * (period - aperiod);
		cpt = (365 * disc) / ir;
		fmt = " %.1f ";
	} else {
		int day = 0;
		double aint = 0.0;
		double charge = amount * disc / 100.0;
		double adir;

		/* Calculate APR interest */
		adir = ir / 365 / 100.0; /* daily interest rate */
		interest = amount * pow(1 + adir, (period - aperiod)) - amount;
		/* Approximate the MSC/interest convergence point */
		while (aint < charge)
			aint = amount * pow(1 + adir, ++day) - amount;
		cpt = day;
		fmt = " ~%.0f ";
	}
	/* Calculate the actual discount amount */
	disc = amount*disc / 100.0;

	/* Display discount amount */
	gtk_text_buffer_set_text(widgets->buffer,
			"With the given values.\n\n", -1);
	gtk_text_buffer_insert_at_cursor(widgets->buffer, "\tYou'd receive a "
			"discount of ", -1);
	snprintf(line, sizeof(line), "%.2f\n", disc);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(widgets->buffer), &iter);
	gtk_text_buffer_insert_with_tags_by_name(widgets->buffer, &iter,
			line, -1, "bold", NULL);

	/* Display lost interest */
	gtk_text_buffer_insert_at_cursor(widgets->buffer, "\tYou'd loose ",
			-1);
	snprintf(line, sizeof(line), "%.2f", interest);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(widgets->buffer), &iter);
	gtk_text_buffer_insert_with_tags_by_name(widgets->buffer, &iter,
			line, -1, "bold", NULL);
	gtk_text_buffer_insert_at_cursor(widgets->buffer, " in interest.\n\n",
			-1);

	/* Display difference */
	gtk_text_buffer_insert_at_cursor(widgets->buffer, "\tA difference of ",
			-1);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(widgets->buffer), &iter);
	gtk_text_buffer_insert_with_tags_by_name(widgets->buffer, &iter,
			"(discount - interest) ", -1, "light", "italic", NULL);
	snprintf(line, sizeof(line), "%.2f\n\n", disc - interest);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(widgets->buffer), &iter);
	gtk_text_buffer_insert_with_tags_by_name(widgets->buffer, &iter, line,
			-1, (disc - interest > 0.0) ? "green" : "red",
			NULL);

	/* Display convergence */
	gtk_text_buffer_insert_at_cursor(widgets->buffer, "After", -1);
	snprintf(line, sizeof(line), fmt, cpt);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(widgets->buffer), &iter);
	gtk_text_buffer_insert_with_tags_by_name(widgets->buffer, &iter,
			line, -1, "bold", NULL);
	gtk_text_buffer_insert_at_cursor(widgets->buffer, "days, the "
			"interest lost would be equal to the discount "
			"received.", -1);
}

G_MODULE_EXPORT void cb_quit(void)
{
	gtk_main_quit();
}

G_MODULE_EXPORT void cb_about(GtkWidget *button, struct widgets *widgets)
{
	gtk_dialog_run(GTK_DIALOG(widgets->about));
	gtk_widget_hide(widgets->about);
}

G_MODULE_EXPORT void cb_show_readme(GtkWidget *button, struct widgets *widgets)
{
	gtk_widget_show(widgets->readme);
}

G_MODULE_EXPORT void cb_hide_readme(GtkWidget *button, struct widgets *widgets)
{
	gtk_widget_hide(widgets->readme);
}

G_MODULE_EXPORT void cb_runit(GtkWidget *widget, struct widgets *widgets)
{
	const char *itype;

	if (strlen(gtk_entry_get_text(GTK_ENTRY(widgets->amount_entry))) == 0)
		return;

	if (strlen(gtk_entry_get_text(GTK_ENTRY(widgets->ir_entry))) == 0)
		return;
	itype = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(
				widgets->itype));
	if (strstr(itype, "Select"))
		return;

	if (strlen(gtk_entry_get_text(GTK_ENTRY(widgets->pt_entry))) == 0)
		return;

	if (strlen(gtk_entry_get_text(GTK_ENTRY(widgets->apt_entry))) == 0)
		return;

	if (strlen(gtk_entry_get_text(GTK_ENTRY(widgets->disc_entry))) == 0)
		return;

	do_calc(widgets);
}

static void gather_widgets(GtkBuilder *builder, struct widgets *widgets)
{
	widgets->window = GTK_WIDGET(gtk_builder_get_object(builder,
				"window"));
	widgets->about = GTK_WIDGET(gtk_builder_get_object(builder,
				"about_cashflo-qp"));
	widgets->readme = GTK_WIDGET(gtk_builder_get_object(builder,
				"readme"));
	widgets->amount_entry = GTK_WIDGET(gtk_builder_get_object(builder,
				"amount_entry"));
	widgets->ir_entry = GTK_WIDGET(gtk_builder_get_object(builder,
				"ir_entry"));
	widgets->itype = GTK_WIDGET(gtk_builder_get_object(builder,
				"itype"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->itype), 0);
	widgets->pt_entry = GTK_WIDGET(gtk_builder_get_object(builder,
				"pt_entry"));
	widgets->apt_entry = GTK_WIDGET(gtk_builder_get_object(builder,
				"apt_entry"));
	widgets->disc_entry = GTK_WIDGET(gtk_builder_get_object(builder,
				"disc_entry"));
	widgets->buffer = GTK_TEXT_BUFFER(gtk_builder_get_object(builder,
				"result_buffer"));
	widgets->readme_buffer = GTK_TEXT_BUFFER(gtk_builder_get_object(
				builder, "readme_buffer"));

	gtk_text_buffer_create_tag(widgets->buffer, "bold", "weight",
			PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(widgets->buffer, "light", "weight",
			PANGO_WEIGHT_LIGHT, NULL);
	gtk_text_buffer_create_tag(widgets->buffer, "italic", "style",
			PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(widgets->buffer, "red", "foreground", "red",
			NULL);
	gtk_text_buffer_create_tag(widgets->buffer, "green", "foreground",
			"green", NULL);
}

static void create_readme(struct widgets *widgets)
{
	char path[FILENAME_MAX];
	char *buf;
	struct stat st;
	int ret;
	int fd;

	ret = stat("README", &st);
	if (ret == 0) {
		sprintf(path, "README");
	} else {
		snprintf(path, sizeof(path),
				"/usr/share/doc/cashflo-qp-%s/README",
				VERSION);
		stat(path, &st);
	}

	buf = malloc(st.st_size);
	fd = open(path, O_RDONLY);
	read(fd, buf, st.st_size);
	gtk_text_buffer_set_text(widgets->readme_buffer, buf, st.st_size);
	close(fd);
	free(buf);
}

int main(int argc, char **argv)
{
	GtkBuilder *builder;
	GError *error = NULL;
	struct widgets *widgets;
	const char *glade_path;
	struct stat st;
	int ret;

	gtk_init(&argc, &argv);

	ret = stat("cashflo-qp.glade", &st);
	if (ret == 0)
		glade_path = "cashflo-qp.glade";
	else
		glade_path = "/usr/share/cashflo/cashflo-qp.glade";

	builder = gtk_builder_new();
	if (!gtk_builder_add_from_file(builder, glade_path, &error)) {
		g_warning("%s", error->message);
		exit(EXIT_FAILURE);
	}

	widgets = g_slice_new(struct widgets);
	gather_widgets(builder, widgets);
	gtk_builder_connect_signals(builder, widgets);
	g_object_unref(G_OBJECT(builder));

	create_readme(widgets);
	gtk_widget_show(widgets->window);
	gtk_main();

	g_slice_free(struct widgets, widgets);

	exit(EXIT_SUCCESS);
}
