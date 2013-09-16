/* Copyright 2013, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.1 only (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 *
 * You may obtain a copy of the Licence at:
 *
 *   http://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

/**
 * \file: ps2paper.c
 *
 * Postscript 2 paper dialogue implementation.
 */

/* ANSI C header files */

#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/fileswitch.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/debug.h"
#include "sflib/errors.h"
#include "sflib/event.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/windows.h"

/* Application header files */

#include "ps2paper.h"

#include "ihelp.h"
#include "templates.h"

#define PS2PAPER_NAME_LEN 128
#define PS2PAPER_SOURCE_LEN 32
#define PS2PAPER_ICON_HEIGHT 48

/* Paper Window icons. */

#define PS2PAPER_ICON_PANE 0

#define PS2PAPER_PANE_NAME 0
#define PS2PAPER_PANE_X 1
#define PS2PAPER_PANE_Y 2
#define PS2PAPER_PANE_SOURCE 3
#define PS2PAPER_PANE_FILE 4
#define PS2PAPER_PANE_STATUS 5

#define PS2PAPER_ICON_CLOSE 1
#define PS2PAPER_ICON_UPDATE 2
#define PS2PAPER_ICON_INCH 3
#define PS2PAPER_ICON_MM 4
#define PS2PAPER_ICON_POINT 5
//#define PAPER_ICON_SIZE_X 3
//#define PAPER_ICON_SIZE_Y 4

enum ps2paper_status {
	PS2PAPER_STATUS_MISSING,				/**< There is no file for the paper size.	*/
	PS2PAPER_STATUS_UNKNOWN,				/**< There is a file, but it's not one of ours.	*/
	PS2PAPER_STATUS_CORRECT,				/**< There is a file, and it matches the paper.	*/
	PS2PAPER_STATUS_INCORRECT				/**< There is a file, but the size is wrong.	*/
};

enum ps2paper_units {
	PS2PAPER_UNITS_MM = 0,
	PS2PAPER_UNITS_INCH = 1,
	PS2PAPER_UNITS_POINT = 2
};

struct ps2paper_size {
	char			name[PS2PAPER_NAME_LEN];	/**< The Printers name for the paper		*/
	int			width;				/**< The Printers width of the paper		*/
	int			height;				/**< The Printers height of the paper		*/
	char			source[PS2PAPER_SOURCE_LEN];	/**< The name of the source file		*/
	char			ps2_file[PS2PAPER_NAME_LEN];	/**< The associated PS2 Paper file, or ""	*/
	enum ps2paper_status	ps2_file_status;		/**< Indicate the status of the Paper File.	*/

	struct ps2paper_size	*next;				/**< Link to the next paper size.		*/
};

static struct ps2paper_size	*paper_sizes = NULL;		/**< Linked list of paper sizes.		*/
static unsigned			paper_count = 0;		/**< Number of defined paper sizes.		*/
static struct ps2paper_size	**redraw_list = NULL;		/**< Redraw list for paper sizes.		*/

static wimp_w			ps2paper_window = NULL;		/**< The paper list window.			*/
static wimp_w			ps2paper_pane = NULL;		/**< The paper list pane.			*/
static wimp_window		*ps2paper_pane_def = NULL;	/**< The paper list pane defintion.		*/

static enum ps2paper_units	page_display_unit = PS2PAPER_UNITS_MM;

static void	ps2paper_close_window(void);
static void	ps2paper_pane_redraw_handler(wimp_draw *redraw);
static void	ps2paper_click_handler(wimp_pointer *pointer);
static void	ps2paper_clear_definitions(void);
static void	ps2paper_read_definitions(void);
static osbool	ps2paper_read_def_file(char *file, char *source);
static osbool	ps2paper_update_files(void);
static enum ps2paper_status	ps2paper_read_pagesize(struct ps2paper_size *paper, char *file);
static osbool	ps2paper_write_pagesize(struct ps2paper_size *paper, char *file_path);


/**
 * Initialise the ps2paper dialogue.
 */

void ps2paper_initialise(void)
{
	ps2paper_window = templates_create_window("PsPaper");
	ihelp_add_window(ps2paper_window, "PsPaper", NULL);

	ps2paper_pane_def = templates_load_window("PsPaperPane");
	ps2paper_pane_def->icon_count = 0;
	ps2paper_pane = wimp_create_window(ps2paper_pane_def);
	ihelp_add_window(ps2paper_pane, "PsPaperPane", NULL /*convert_decode_queue_pane_help*/);

	event_add_window_redraw_event(ps2paper_pane, ps2paper_pane_redraw_handler);
	//event_add_window_mouse_event(ps2paper_pane convert_queue_pane_click_handler);

	paper_count = 5;

	event_add_window_mouse_event(ps2paper_window, ps2paper_click_handler);
	//event_add_window_key_event(paper_window, paper_keypress_handler);

	event_add_window_icon_radio(ps2paper_window, PS2PAPER_ICON_MM, FALSE);
	event_add_window_icon_radio(ps2paper_window, PS2PAPER_ICON_INCH, FALSE);
	event_add_window_icon_radio(ps2paper_window, PS2PAPER_ICON_POINT, FALSE);
}


/**
 * Open the PS2 Paper dialogue.
 *
 * \param *pointer	The pointer location at which to open the window.
 */

void ps2paper_open_window(wimp_pointer *pointer)
{
	icons_set_selected(ps2paper_window, PS2PAPER_ICON_MM, (page_display_unit == PS2PAPER_UNITS_MM) ? TRUE : FALSE);
	icons_set_selected(ps2paper_window, PS2PAPER_ICON_INCH, (page_display_unit == PS2PAPER_UNITS_INCH) ? TRUE : FALSE);
	icons_set_selected(ps2paper_window, PS2PAPER_ICON_POINT, (page_display_unit == PS2PAPER_UNITS_POINT) ? TRUE : FALSE);

	windows_open_with_pane_centred_at_pointer(ps2paper_window, ps2paper_pane, PS2PAPER_ICON_PANE, 40, pointer);
	ps2paper_read_definitions();
}


/**
 * Close the PS2 Paper dialogue.
 */

static void ps2paper_close_window(void)
{
	wimp_close_window(ps2paper_window);
	ps2paper_clear_definitions();
}


/**
 * Process redraw requests for the PS2 Paper dialogue pane.
 *
 * \param *redraw		The redraw event block to handle.
 */

static void ps2paper_pane_redraw_handler(wimp_draw *redraw)
{
	int			ox, oy, top, base, y;
	osbool			more;
	wimp_icon		*icon;
	char			buffer[128], *status, *unit_format;
	double			unit_scale;

	/* Perform the redraw if a window was found. */

	if (redraw->w != ps2paper_pane)
		return;

	more = wimp_redraw_window(redraw);

	ox = redraw->box.x0 - redraw->xscroll;
	oy = redraw->box.y1 - redraw->yscroll;

	icon = ps2paper_pane_def->icons;

	switch (page_display_unit) {
	case PS2PAPER_UNITS_MM:
		unit_scale = 2834.64567;
		unit_format = "%.1f";
		break;
	case PS2PAPER_UNITS_INCH:
		unit_scale = 72000.0;
		unit_format = "%.3f";
		break;
	case PS2PAPER_UNITS_POINT:
	default:
		unit_scale = 1000.0;
		unit_format = "%.1f";
		break;
	}

	while (more) {
		top = (oy - redraw->clip.y1) / PS2PAPER_ICON_HEIGHT;
		if (top < 0)
			top = 0;

		base = (PS2PAPER_ICON_HEIGHT + (PS2PAPER_ICON_HEIGHT / 2) + oy - redraw->clip.y0) / PS2PAPER_ICON_HEIGHT;

		for (y = top; y < paper_count && y <= base; y++) {
			icon[PS2PAPER_PANE_NAME].extent.y1 = -(y * PS2PAPER_ICON_HEIGHT);
			icon[PS2PAPER_PANE_NAME].extent.y0 = icon[PS2PAPER_PANE_NAME].extent.y1 - PS2PAPER_ICON_HEIGHT;
			icon[PS2PAPER_PANE_NAME].data.indirected_text_and_sprite.text = (redraw_list[y])->name;
			icon[PS2PAPER_PANE_NAME].data.indirected_text_and_sprite.size = PS2PAPER_NAME_LEN;

			wimp_plot_icon(&(icon[PS2PAPER_PANE_NAME]));

			snprintf(buffer, sizeof(buffer), unit_format, (double) (redraw_list[y])->width / unit_scale);

			icon[PS2PAPER_PANE_X].extent.y1 = -(y * PS2PAPER_ICON_HEIGHT);
			icon[PS2PAPER_PANE_X].extent.y0 = icon[PS2PAPER_PANE_X].extent.y1 - PS2PAPER_ICON_HEIGHT;
			icon[PS2PAPER_PANE_X].data.indirected_text_and_sprite.text = buffer;
			icon[PS2PAPER_PANE_X].data.indirected_text_and_sprite.size = sizeof(buffer);

			wimp_plot_icon(&(icon[PS2PAPER_PANE_X]));

			snprintf(buffer, sizeof(buffer), unit_format, (double) (redraw_list[y])->height / unit_scale);

			icon[PS2PAPER_PANE_Y].extent.y1 = -(y * PS2PAPER_ICON_HEIGHT);
			icon[PS2PAPER_PANE_Y].extent.y0 = icon[PS2PAPER_PANE_Y].extent.y1 - PS2PAPER_ICON_HEIGHT;
			icon[PS2PAPER_PANE_Y].data.indirected_text_and_sprite.text = buffer;
			icon[PS2PAPER_PANE_Y].data.indirected_text_and_sprite.size = sizeof(buffer);

			wimp_plot_icon(&(icon[PS2PAPER_PANE_Y]));

			icon[PS2PAPER_PANE_SOURCE].extent.y1 = -(y * PS2PAPER_ICON_HEIGHT);
			icon[PS2PAPER_PANE_SOURCE].extent.y0 = icon[PS2PAPER_PANE_SOURCE].extent.y1 - PS2PAPER_ICON_HEIGHT;
			icon[PS2PAPER_PANE_SOURCE].data.indirected_text_and_sprite.text = (redraw_list[y])->source;
			icon[PS2PAPER_PANE_SOURCE].data.indirected_text_and_sprite.size = PS2PAPER_SOURCE_LEN;

			wimp_plot_icon(&(icon[PS2PAPER_PANE_SOURCE]));

			icon[PS2PAPER_PANE_FILE].extent.y1 = -(y * PS2PAPER_ICON_HEIGHT);
			icon[PS2PAPER_PANE_FILE].extent.y0 = icon[PS2PAPER_PANE_FILE].extent.y1 - PS2PAPER_ICON_HEIGHT;
			icon[PS2PAPER_PANE_FILE].data.indirected_text_and_sprite.text = (redraw_list[y])->ps2_file;
			icon[PS2PAPER_PANE_FILE].data.indirected_text_and_sprite.size = PS2PAPER_NAME_LEN;

			if ((redraw_list[y])->ps2_file_status != PS2PAPER_STATUS_MISSING)
				wimp_plot_icon(&(icon[PS2PAPER_PANE_FILE]));

			switch((redraw_list[y])->ps2_file_status) {
			case PS2PAPER_STATUS_MISSING:
				status = "PaperStatMiss";
				break;
			case PS2PAPER_STATUS_UNKNOWN:
				status = "PaperStatUnkn";
				break;
			case PS2PAPER_STATUS_CORRECT:
				status = "PaperStatOK";
				break;
			case PS2PAPER_STATUS_INCORRECT:
				status = "PaperStatNOK";
				break;
			default:
				status = "";
				break;
			}

			msgs_lookup(status, buffer, sizeof(buffer));

			icon[PS2PAPER_PANE_STATUS].extent.y1 = -(y * PS2PAPER_ICON_HEIGHT);
			icon[PS2PAPER_PANE_STATUS].extent.y0 = icon[PS2PAPER_PANE_STATUS].extent.y1 - PS2PAPER_ICON_HEIGHT;
			icon[PS2PAPER_PANE_STATUS].data.indirected_text_and_sprite.text = buffer;
			icon[PS2PAPER_PANE_STATUS].data.indirected_text_and_sprite.size = sizeof(buffer);

			wimp_plot_icon(&(icon[PS2PAPER_PANE_STATUS]));


			/*icon[QUEUE_PANE_INCLUDE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
			icon[QUEUE_PANE_INCLUDE].extent.y0 = icon[QUEUE_PANE_INCLUDE].extent.y1 - QUEUE_ICON_HEIGHT;
			icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.id =
					(osspriteop_id) (((queue_redraw_list[y])->include) ? "opton" : "optoff");
			icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.area = (osspriteop_area *) 1;
			icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.size = 12;

			wimp_plot_icon(&(icon[QUEUE_PANE_INCLUDE])); */


			/*icon[QUEUE_PANE_DELETE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
			icon[QUEUE_PANE_DELETE].extent.y0 = icon[QUEUE_PANE_DELETE].extent.y1 - QUEUE_ICON_HEIGHT;
			icon[QUEUE_PANE_DELETE].data.indirected_sprite.id =
					(osspriteop_id) (((queue_redraw_list[y])->object_type == DELETED) ? "del1" : "del0");
			icon[QUEUE_PANE_DELETE].data.indirected_sprite.area = main_wimp_sprites;
			icon[QUEUE_PANE_DELETE].data.indirected_sprite.size = 12;

			wimp_plot_icon(&(icon[QUEUE_PANE_DELETE])); */
		}

		more = wimp_get_rectangle (redraw);
	}
}


/**
 * Process mouse clicks in the PS2 Paper dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void ps2paper_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case PS2PAPER_ICON_MM:
		page_display_unit = PS2PAPER_UNITS_MM;
		windows_redraw(ps2paper_pane);
		break;

	case PS2PAPER_ICON_INCH:
		page_display_unit = PS2PAPER_UNITS_INCH;
		windows_redraw(ps2paper_pane);
		break;

	case PS2PAPER_ICON_POINT:
		page_display_unit = PS2PAPER_UNITS_POINT;
		windows_redraw(ps2paper_pane);
		break;

	case PS2PAPER_ICON_CLOSE:
		ps2paper_close_window();
		break;

	case PS2PAPER_ICON_UPDATE:
		ps2paper_update_files();
		ps2paper_close_window();
		break;
	}
}


static void ps2paper_read_definitions(void)
{
	struct ps2paper_size	*paper;
	wimp_window_state	state;
	os_box			extent;
	int			i, visible_extent, new_extent, new_scroll;
	char			source[PS2PAPER_SOURCE_LEN];

	ps2paper_clear_definitions();

	msgs_lookup("PaperFileM", source, PS2PAPER_SOURCE_LEN);
	ps2paper_read_def_file("Printers:PaperRO", source);
	msgs_lookup("PaperFileU", source, PS2PAPER_SOURCE_LEN);
	ps2paper_read_def_file("PrinterChoices:PaperRW", source);
	msgs_lookup("PaperFileD", source, PS2PAPER_SOURCE_LEN);
	ps2paper_read_def_file("Printers:ps.Resources.PaperRO", source);

	redraw_list = malloc(paper_count * sizeof(struct ps2paper_size *));
	if (redraw_list == NULL) {
		error_msgs_report_error("PaperNoMem");
		ps2paper_clear_definitions();
		return;
	}

	paper = paper_sizes;
	i = paper_count;

	while (i > 0 && paper != NULL) {
		redraw_list[--i] = paper;
		paper = paper->next;
	}

	/* Set the window extent. */

	state.w = ps2paper_pane;
	wimp_get_window_state(&state);

	visible_extent = state.yscroll + (state.visible.y0 - state.visible.y1);

	new_extent = -PS2PAPER_ICON_HEIGHT * paper_count;

	if (new_extent > (state.visible.y0 - state.visible.y1))
		new_extent = state.visible.y0 - state.visible.y1;

	if (new_extent > visible_extent) {
		/* Calculate the required new scroll offset.  If this is greater than zero, the current window is too
		 * big and will need shrinking down.  Otherwise, just set the new scroll offset.
		 */

		new_scroll = new_extent - (state.visible.y0 - state.visible.y1);

		if (new_scroll > 0) {
			state.visible.y0 += new_scroll;
			state.yscroll = 0;
		} else {
			state.yscroll = new_scroll;
		}

		wimp_open_window((wimp_open *) &state);
	}

	extent.x0 = 0;
	extent.y1 = 0;
	extent.x1 = state.visible.x1 - state.visible.x0;
	extent.y0 = new_extent;

	wimp_set_extent(ps2paper_pane, &extent);


}

static void ps2paper_clear_definitions(void)
{
	struct ps2paper_size	*paper, *current;

	paper = paper_sizes;
	paper_sizes = NULL;
	paper_count = 0;

	while (paper != NULL) {
		current = paper;
		paper = paper->next;
		free(current);
	}
}

/**
 * Process the contents of a Printers paper file, reading the paper definitions
 * and adding them to the list of sizes.
 *
 * \param *file		The name of the file to be read in.
 * \param *source	The "user facing" name for the file.
 */

static osbool ps2paper_read_def_file(char *file, char *source)
{
	FILE			*in;
	char			line[1024], *clean, *data, paper_name[PS2PAPER_NAME_LEN];
	int			i;
	unsigned		paper_width, paper_height;
	struct ps2paper_size	*paper_definition;
	os_error		*error;
	fileswitch_object_type	type;


	if (file == NULL || *file == '\0')
		return FALSE;

	in = fopen(file, "r");

	if (in == NULL)
		return FALSE;

	*paper_name = '\0';
	paper_width = 0;
	paper_height = 0;

	while (fgets(line, sizeof(line), in) != NULL) {
		string_ctrl_zero_terminate(line);
		clean = string_strip_surrounding_whitespace(line);

		if (*clean == '\0' || *clean == '#')
			continue;

		if (strstr(clean, "pn:") == clean) {
			data = string_strip_surrounding_whitespace(clean + 3);
			strncpy(paper_name, data, PS2PAPER_NAME_LEN);
		} else if (strstr(clean, "pw:") == clean) {
			data = string_strip_surrounding_whitespace(clean + 3);
			paper_width = atoi(data);
		} else if (strstr(clean, "ph:") == clean) {
			data = string_strip_surrounding_whitespace(clean + 3);
			paper_height = atoi(data);
		}

		if (paper_name != '\0' && paper_width != 0 && paper_height != 0) {
			paper_definition = malloc(sizeof(struct ps2paper_size));

			if (paper_definition != NULL) {
				strncpy(paper_definition->name, paper_name, PS2PAPER_NAME_LEN);
				strncpy(paper_definition->source, source, PS2PAPER_SOURCE_LEN);
				paper_definition->width = paper_width;
				paper_definition->height = paper_height;

				for (i = 0; i < PS2PAPER_NAME_LEN && paper_name[i] != '\0' && paper_name[i] != ' '; i++)
					paper_definition->ps2_file[i] = paper_name[i];

				paper_definition->ps2_file[i] = '\0';
				paper_definition->ps2_file_status = PS2PAPER_STATUS_MISSING;

				string_tolower(paper_definition->ps2_file);

				if (paper_definition->ps2_file[0] != '\0') {
					snprintf(line, sizeof(line), "Printers:ps.Paper.%s", paper_definition->ps2_file);
					error = xosfile_read_no_path(line, &type, NULL, NULL, NULL, NULL);

					if (error == NULL && type == fileswitch_IS_FILE)
						paper_definition->ps2_file_status = ps2paper_read_pagesize(paper_definition, line);
				}

				paper_definition->next = paper_sizes;
				paper_sizes = paper_definition;

				paper_count++;
			}

			*paper_name = '\0';
			paper_width = 0;
			paper_height = 0;
		}
	}

	fclose(in);

	return TRUE;
}

static osbool ps2paper_update_files(void)
{
	struct ps2paper_size	*paper;
	int			var_len;
	char			file_path[1024];


	*file_path = '\0';

	os_read_var_val_size("Choices$Write", 0, os_VARTYPE_STRING, &var_len, NULL);

	if (var_len == 0)
		return FALSE;

	snprintf(file_path, sizeof(file_path), "<Choices$Write>.Printers");
	if (osfile_read_no_path(file_path, NULL, NULL, NULL, NULL) == fileswitch_NOT_FOUND)
		osfile_create_dir(file_path, 0);

	snprintf(file_path, sizeof(file_path), "<Choices$Write>.Printers.ps");
	if (osfile_read_no_path(file_path, NULL, NULL, NULL, NULL) == fileswitch_NOT_FOUND)
		osfile_create_dir(file_path, 0);

	snprintf(file_path, sizeof(file_path), "<Choices$Write>.Printers.ps.Paper");
	if (osfile_read_no_path(file_path, NULL, NULL, NULL, NULL) == fileswitch_NOT_FOUND)
		osfile_create_dir(file_path, 0);

	paper = paper_sizes;

	while (paper != NULL) {
		if (paper->ps2_file_status == PS2PAPER_STATUS_MISSING || paper->ps2_file_status == PS2PAPER_STATUS_INCORRECT)
			ps2paper_write_pagesize(paper, file_path);

		paper = paper->next;
	}

	return TRUE;
}

static enum ps2paper_status ps2paper_read_pagesize(struct ps2paper_size *paper, char *file)
{
	FILE	*in;
	char	line[1024];
	double	width, height;

	if (file == NULL)
		return PS2PAPER_STATUS_UNKNOWN;

	in = fopen(file, "r");
	if (in == NULL)
		return PS2PAPER_STATUS_UNKNOWN;

	if (fgets(line, sizeof(line), in) == NULL) {
		fclose(in);
		return PS2PAPER_STATUS_UNKNOWN;
	}

	if (strcmp(line, "% Created by PrintPDF\n") != 0) {
		fclose(in);
		return PS2PAPER_STATUS_UNKNOWN;
	}

	if (fgets(line, sizeof(line), in) == NULL) {
		fclose(in);
		return PS2PAPER_STATUS_UNKNOWN;
	}

	if (fgets(line, sizeof(line), in) == NULL) {
		fclose(in);
		return PS2PAPER_STATUS_UNKNOWN;
	}

	sscanf(line, "<< /PageSize [ %lf %lf ] >> setpagedevice \n", &width, &height);

	fclose(in);

	if (width * 1000.0 != paper->width || height * 1000.0 != paper->height)
		return PS2PAPER_STATUS_INCORRECT;

	return PS2PAPER_STATUS_CORRECT;
}

static osbool ps2paper_write_pagesize(struct ps2paper_size *paper, char *file_path)
{
	FILE	*out;
	char	filename[1024];

	if (paper == NULL)
		return FALSE;

	snprintf(filename, sizeof(filename), "%s.%s", file_path, paper->ps2_file);

	out = fopen(filename, "w");
	if (out == NULL)
		return FALSE;

	fprintf(out, "%% Created by PrintPDF\n");
	fprintf(out, "%%%%BeginFeature: PageSize %s\n", paper->name);
	fprintf(out, "<< /PageSize [ %.3f %.3f ] >> setpagedevice\n", (double) paper->width / 1000.0, (double) paper->height / 1000.0);
	fprintf(out, "%%%%EndFeature\n");

	fclose(out);
	osfile_set_type (filename, (bits) 0xff5);

	return TRUE;
}

