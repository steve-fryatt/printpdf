/* Copyright 2005-2020, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
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
 * \file: convert.c
 *
 * Conversion queue implementation.
 */

/* ANSI C header files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/fileswitch.h"
#include "oslib/osfscontrol.h"
#include "oslib/os.h"
#include "oslib/wimp.h"
#include "oslib/dragasprite.h"
#include "oslib/wimpspriteop.h"
#include "oslib/osspriteop.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/dataxfer.h"
#include "sflib/debug.h"
#include "sflib/errors.h"
#include "sflib/event.h"
#include "sflib/icons.h"
#include "sflib/ihelp.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "convert.h"

#include "bookmark.h"
#include "choices.h"
#include "encrypt.h"
#include "main.h"
#include "optimize.h"
#include "paper.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "popup.h"
#include "version.h"


#define MAX_QUEUE_NAME 32
#define MAX_DISPLAY_NAME 64

#define QUEUE_ICON_HEIGHT 48

#define CONVERT_COMMAND_LENGTH 1024


/* Save PDF Window icons. */

#define SAVE_PDF_ICON_OK             0
#define SAVE_PDF_ICON_CANCEL         1
#define SAVE_PDF_ICON_NAME           2
#define SAVE_PDF_ICON_FILE           3
#define SAVE_PDF_ICON_VERSION_MENU   4
#define SAVE_PDF_ICON_VERSION_FIELD  5
#define SAVE_PDF_ICON_OPT_MENU       7
#define SAVE_PDF_ICON_OPT_FIELD      8
#define SAVE_PDF_ICON_PREPROCESS     10
#define SAVE_PDF_ICON_ENCRYPT_MENU   11
#define SAVE_PDF_ICON_ENCRYPT_FIELD  12
#define SAVE_PDF_ICON_QUEUE          14
#define SAVE_PDF_ICON_PDFMARK_MENU   15
#define SAVE_PDF_ICON_PDFMARK_FIELD  16
#define SAVE_PDF_ICON_USERFILE       18
#define SAVE_PDF_ICON_BOOKMARK_MENU  20
#define SAVE_PDF_ICON_BOOKMARK_FIELD 21
#define SAVE_PDF_ICON_PAPER_MENU     23
#define SAVE_PDF_ICON_PAPER_FIELD    24

/* Defer queue window icons. */

#define QUEUE_ICON_PANE   0
#define QUEUE_ICON_CLOSE  1
#define QUEUE_ICON_CREATE 2

#define QUEUE_PANE_INCLUDE 0
#define QUEUE_PANE_FILE    1
#define QUEUE_PANE_DELETE  2

/* Conversion progress stages. */

enum conversion_state {
	CONVERSION_STOPPED,		/**< No Conversion in action.		*/
	CONVERSION_STARTING,		/**< Conversion ready to start.		*/
	CONVERSION_PS2PS_PENDING,	/**< *ps2ps process is starting.	*/
	CONVERSION_PS2PS,		/**< *ps2ps process is running.		*/
	CONVERSION_PS2PDF_PENDING,	/**< *ps2pdf process is starting.	*/
	CONVERSION_PS2PDF		/**< *ps2pdf process is running.	*/
};

/* Queue entry types. */

enum queue_type {
	PENDING_ATTENTION,
	BEING_PROCESSED,
	HELD_IN_QUEUE,
	DISCARDED,
	DELETED
};

typedef struct queued_file {
	char			filename[MAX_QUEUE_NAME];
	char			display_name[MAX_DISPLAY_NAME];
	enum queue_type		object_type;
	int			include;

	struct queued_file	*next;
} queued_file;

typedef struct conversion_params {
	char			input_filename[CONVERT_MAX_FILENAME];
	char			output_filename[CONVERT_MAX_FILENAME];
	char			pdfmark_userfile[CONVERT_MAX_FILENAME];

	int			preprocess_in_ps2ps;
} conversion_params;

/* Message for remote control operation. */

#define message_PRINTPDF_CONTROL	0x5A480	/**< Wimp Message number */

enum control_reason {
	CONTROL_SET_FILENAME = 0,		/**< Set up filename to save to.	*/
	CONTROL_REPORT_SUCCESS = 1,		/**< Conversion completed correctly.	*/
	CONTROL_REPORT_FAILURE = 2		/**< Conversion failed.			*/
};

typedef struct {
	wimp_MESSAGE_HEADER_MEMBERS
	enum control_reason	reason;
	char			filename[232];
} control_message;


/* ****************************************************************************
 * Function Prototypes
 * ****************************************************************************/

static void		convert_start_held_conversion(void);
static void		convert_open_save_dialogue(void);
static void		convert_save_dialogue_end(char *output_file);
static void		convert_save_dialogue_queue(void);

static osbool		convert_handle_save_icon_drop(wimp_message *message);

static osbool		convert_progress(conversion_params *params);
static osbool		convert_launch_ps2ps(char *file_out);
static osbool		convert_launch_ps2pdf(char *file_out, char *user_pdfmark_file);
static void             convert_notify_completion(osbool success);
static void		convert_cancel_conversion(void);

static void		convert_save_click_handler(wimp_pointer *pointer);
static osbool		convert_save_keypress_handler(wimp_key *key);
static void		convert_save_menu_prepare_handler(wimp_w w, wimp_menu *menu, wimp_pointer *pointer);
static void		convert_save_menu_selection_handler(wimp_w w, wimp_menu *menu, wimp_selection *selection);
static void		convert_save_menu_close_handler(wimp_w w, wimp_menu *menu);
static osbool		convert_immediate_window_save(void);
static void		convert_drag_end_handler(wimp_pointer *pointer, void *data);
static osbool		convert_drag_end_save_handler(char *filename, void *data);

static void		convert_process_pdfmark_dialogue(void);
static void		convert_process_encrypt_dialogue(void);
static void		convert_process_optimize_dialogue(void);
static void		convert_process_paper_dialogue(void);

static void		convert_remove_current_conversion(void);
static void		convert_remove_deleted_files(void);
static void		convert_remove_first_conversion(void);

/* Defer queue manipulation. */

static void		convert_close_queue_window(void);
static void		convert_rebuild_queue_index(void);
static void		convert_reorder_queue_from_index(void);

static void		convert_queue_pane_redraw_handler(wimp_draw *redraw);
static void		convert_queue_click_handler(wimp_pointer *pointer);
static void		convert_queue_pane_click_handler(wimp_pointer *pointer);
static void		convert_start_queue_entry_drag(int line);
static void		convert_terminate_queue_entry_drag(wimp_dragged *drag, void *data);

static osbool		convert_check_for_conversion_start(wimp_message *message);
static osbool		convert_check_for_conversion_end(wimp_message *message);
static osbool		convert_printpdf_control(wimp_message *message);

static void		convert_decode_queue_pane_help(char *buffer, wimp_w w, wimp_i i, os_coord pos, wimp_mouse_state buttons);

/* ==================================================================================================================
 * Global variables.
 */

static queued_file	*queue = NULL;
static osbool		conversion_in_progress = FALSE;
static osbool		files_pending_attention = TRUE;
static wimp_t		conversion_task = 0;

static queued_file	**queue_redraw_list = NULL;
static int		queue_redraw_lines = 0;

static osbool		dragging_sprite;
static int		dragging_start_line;

static wimp_menu	*popup_version;
static wimp_menu	*popup_optimize;
static wimp_menu	*popup_paper;
static wimp_menu	*popup_bookmark;

/* Variables to support other tasks stipulating PDF filename via messages. */

static wimp_t		request_task = NULL;
static int		request_ref = 0;
static char		request_filename[CONVERT_MAX_FILENAME];

/* Conversion parameters. */

static encrypt_params	encryption;
static optimize_params	optimization;
static version_params	version;
static pdfmark_params	pdfmark;
static bookmark_params	bookmark;
static paper_params	paper;

static wimp_w		convert_savepdf_window = NULL;
static wimp_w		convert_queue_window = NULL;
static wimp_w		convert_queue_pane = NULL;
static wimp_window	*convert_queue_pane_def = NULL;


/**
 * Test for the presence of the queue directory, and create it if it is not already there.
 *
 * At present, this assumes that it will be just the leaf directory missing; true assuming an address in the Scrap
 * folder.
 *
 * Also set up the conversion parameters for the first time, for each of the modules.
 */

void convert_initialise(void)
{
	char			*queue_dir;
	fileswitch_object_type	type;

	/* Set up the queue directory */

	queue_dir = config_str_read("FileQueue");

	xosfile_read_no_path(queue_dir, &type, NULL, NULL, NULL, NULL);

	if (type == fileswitch_NOT_FOUND)
		osfile_create_dir(queue_dir, 0);
	else if (type != fileswitch_IS_DIR)
		error_msgs_report_error("NoQueueDir");

	/* Create the windows and menus. */

	popup_version = templates_get_menu("VersionMenu");
	ihelp_add_menu(popup_version, "VersionMenu");
	popup_optimize = templates_get_menu("OptimizeMenu");
	ihelp_add_menu(popup_optimize, "OptimizeMenu");
	popup_paper = templates_get_menu("PaperMenu");
	ihelp_add_menu(popup_paper, "PaperMenu");

	convert_savepdf_window = templates_create_window("SavePDF");
	ihelp_add_window(convert_savepdf_window, "SavePDF", NULL);

	event_add_window_mouse_event(convert_savepdf_window, convert_save_click_handler);
	event_add_window_key_event(convert_savepdf_window, convert_save_keypress_handler);
	event_add_window_menu_prepare(convert_savepdf_window, convert_save_menu_prepare_handler);
	event_add_window_menu_selection(convert_savepdf_window, convert_save_menu_selection_handler);
	event_add_window_menu_close(convert_savepdf_window, convert_save_menu_close_handler);

	event_add_window_icon_popup(convert_savepdf_window, SAVE_PDF_ICON_VERSION_MENU, popup_version, -1, NULL);
	event_add_window_icon_popup(convert_savepdf_window, SAVE_PDF_ICON_OPT_MENU, popup_optimize, -1, NULL);
	event_add_window_icon_popup(convert_savepdf_window, SAVE_PDF_ICON_PAPER_MENU, popup_paper, -1, NULL);
	event_add_window_icon_popup(convert_savepdf_window, SAVE_PDF_ICON_BOOKMARK_MENU, popup_bookmark, -1, NULL);

	convert_queue_window = templates_create_window("Queue");
	ihelp_add_window(convert_queue_window, "Queue", NULL);

	event_add_window_mouse_event(convert_queue_window, convert_queue_click_handler);

	convert_queue_pane_def = templates_load_window("QueuePane");
	convert_queue_pane_def->icon_count = 0;
	convert_queue_pane = wimp_create_window(convert_queue_pane_def);
	ihelp_add_window(convert_queue_pane, "QueuePane", convert_decode_queue_pane_help);

	event_add_window_redraw_event(convert_queue_pane, convert_queue_pane_redraw_handler);
	event_add_window_mouse_event(convert_queue_pane, convert_queue_pane_click_handler);

	event_add_message_handler(message_TASK_INITIALISE, EVENT_MESSAGE_INCOMING, convert_check_for_conversion_start);
	event_add_message_handler(message_TASK_CLOSE_DOWN, EVENT_MESSAGE_INCOMING, convert_check_for_conversion_end);
	event_add_message_handler(message_DATA_LOAD, EVENT_MESSAGE_INCOMING, convert_handle_save_icon_drop);
	event_add_message_handler(message_PRINTPDF_CONTROL, EVENT_MESSAGE_INCOMING, convert_printpdf_control);

	/* Initialise the options. */

	icons_strncpy(convert_savepdf_window, SAVE_PDF_ICON_NAME, config_str_read("FileName"));
	icons_strncpy(convert_savepdf_window, SAVE_PDF_ICON_USERFILE, config_str_read("PDFMarkUserFile"));
	icons_set_selected(convert_savepdf_window, SAVE_PDF_ICON_PREPROCESS, config_opt_read("PreProcess"));

	encrypt_initialise_settings(&encryption);
	optimize_initialise_settings(&optimization);
	version_initialise_settings(&version);
	pdfmark_initialise_settings(&pdfmark);
	bookmark_initialise_settings(&bookmark);
	paper_initialise_settings(&paper);
}


/**
 * Check the location of the 'print file' to see if one has appeared.  If it
 * has, add it to the file queue.
 *
 * Called from NULL poll events.
 */

void convert_check_for_ps_file(void)
{
	fileswitch_object_type		type;
	int				size;
	char				check_file[CONVERT_MAX_FILENAME];

	convert_build_queue_filename(check_file, CONVERT_MAX_FILENAME, CONVERT_QUEUE_FILENAME);

	xosfile_read_stamped_no_path(check_file, &type, NULL, NULL, &size, NULL, NULL);

	if (type == fileswitch_IS_FILE && size > 0 && convert_queue_ps_file(check_file))
		xosfile_delete(check_file, NULL, NULL, NULL, NULL, NULL);

	/* Handle PDFMaker jobs, for compatibility with R-Comp's system.  If PDFMaker: is set up, check
	 * to see if there is a job file called PS on the path.
	 */

	os_read_var_val_size("PDFMaker$Path", 0, 0, &size, NULL);

	if (size != 0) {
		string_copy(check_file, "PDFMaker:PS", CONVERT_MAX_FILENAME);

		xosfile_read_stamped_no_path(check_file, &type, NULL, NULL, &size, NULL, NULL);

		if (type == fileswitch_IS_FILE && size > 0 && convert_queue_ps_file(check_file))
			xosfile_delete(check_file, NULL, NULL, NULL, NULL, NULL);
	}
}


/**
 * Take the file specified, copy it with a timestamp and add it to the queue of files.
 *
 * \param *filename		The file to copy.
 * \return			TRUE if successful; else FALSE.
 */

osbool convert_queue_ps_file(char *filename)
{
	queued_file		*new, **list = NULL;
	char			queued_filename[CONVERT_MAX_FILENAME];
	os_error		*error;
	os_fw			file;

	/* Try and open the file, to see if it is already open.  If we fail for any reason, return with an error to
	 * show that the queuing failed.
	 */

	error = xosfind_openupw(osfind_NO_PATH, filename, NULL, &file);

	if (error != NULL || file == 0)
		return FALSE;

	osfind_closew(file);

	/* Allocate memory and copy the file on to the queue. */

	new = malloc(sizeof(queued_file));

	if (new == NULL)
		return FALSE;

	string_printf(new->filename, MAX_QUEUE_NAME, "%x", (int) os_read_monotonic_time());
	*(new->display_name) = '\0';
	new->object_type = PENDING_ATTENTION;
	new->next = NULL;

	list = &queue;

	while (*list != NULL)
		list = &((*list)->next);

	*list = new;

	convert_build_queue_filename(queued_filename, CONVERT_MAX_FILENAME, new->filename);
	error = xosfscontrol_copy(filename, queued_filename, osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

	if (error != NULL) {
		*list = NULL;
		free(new);
		return FALSE;
	}

	files_pending_attention = TRUE;

	return TRUE;
}


/**
 * Test to see if there is a file queued and no conversion taking place.  If these are both true, select the next
 * pending file in the queue and open the Save PDF dialogue.
 *
 * Called from NULL poll events.
 */

void convert_check_for_pending_files(void)
{
	queued_file		*list;

	/* We can't start a conversion if:
	 *
	 * - The Choices window is open (the options menus would get confused)
	 * - There is a conversion in progress
	 * - There isn't anything to convert (Duh!)
	 */

	if (choices_window_is_open() || conversion_in_progress || !files_pending_attention || queue == NULL)
		return;

	list = queue;

	files_pending_attention = FALSE;
	conversion_in_progress = FALSE;

	/* Scan thtough the queue.  The first file PENDING_ATTENTION is turned into BEING_PROCESSED.  If there are
	 * more files PENDING_ATTENTION, this is reflected in the files_pending_attention flag to save re-scanning
	 * the queue each NULL Poll.
	 */

	while (list != NULL) {
		if (list->object_type == PENDING_ATTENTION) {
			if (!conversion_in_progress) {
				list->object_type = BEING_PROCESSED;
				conversion_in_progress = TRUE;
			} else {
				files_pending_attention = TRUE;
			}
		}

		list = list->next;
	}

	/* If a file was found to convert, open the Save PDF dialogue. */

	if (conversion_in_progress) {
		if (request_task != NULL)
			convert_save_dialogue_end(request_filename);
		else
			convert_open_save_dialogue();
	}
}


/**
 * Start a conversion on files held in the deferred queue.  This is
 * called by a user action, probably clicking Convert in the queue dialogue.
 */

static void convert_start_held_conversion(void)
{
	queued_file		*list;

	/* We can't start a conversion if:
	 *
	 * - The Choices window is open (the options menus would get confused)
	 * - There is a conversion in progress
	 */

	if (choices_window_is_open() || conversion_in_progress || queue == NULL)
		return;

	list = queue;

	files_pending_attention = FALSE;
	conversion_in_progress = FALSE;

	/* Scan thtough the queue.  The any files HELD_IN_QUEUE are turned into BEING_PROCESSED.  If there are
	 * files PENDING_ATTENTION, this is reflected in the files_pending_attention flag to save re-scanning
	 * the queue each NULL Poll.
	 */

	while (list != NULL) {
		if (list->object_type == PENDING_ATTENTION)
			files_pending_attention = TRUE;

		if (list->object_type == HELD_IN_QUEUE && list->include == TRUE) {
			list->object_type = BEING_PROCESSED;
			conversion_in_progress = TRUE;
		}

		list = list->next;
	}

	/* If a file or files were found to convert, open the Save PDF dialogue. */

	if (conversion_in_progress)
		convert_open_save_dialogue();
}


/**
 * Open the Save PDF dialogue on screen, at the pointer.
 */

static void convert_open_save_dialogue(void)
{
	wimp_pointer		pointer;

	/* Set up and open the conversion window. */

	if (config_opt_read ("ResetParams")) {
		icons_strncpy(convert_savepdf_window, SAVE_PDF_ICON_NAME, config_str_read("FileName"));
		icons_strncpy(convert_savepdf_window, SAVE_PDF_ICON_USERFILE, config_str_read("PDFMarkUserFile"));
		icons_set_selected(convert_savepdf_window, SAVE_PDF_ICON_PREPROCESS, config_opt_read("PreProcess"));
		encrypt_initialise_settings(&encryption);
		optimize_initialise_settings(&optimization);
		version_initialise_settings(&version);
		pdfmark_initialise_settings(&pdfmark);
		bookmark_initialise_settings(&bookmark);
	}

	version_fill_field(convert_savepdf_window, SAVE_PDF_ICON_VERSION_FIELD, &version);
	optimize_fill_field(convert_savepdf_window, SAVE_PDF_ICON_OPT_FIELD, &optimization);
	encrypt_fill_field(convert_savepdf_window, SAVE_PDF_ICON_ENCRYPT_FIELD, &encryption);
	pdfmark_fill_field(convert_savepdf_window, SAVE_PDF_ICON_PDFMARK_FIELD, &pdfmark);
	bookmark_fill_field(convert_savepdf_window, SAVE_PDF_ICON_BOOKMARK_FIELD, &bookmark);

	wimp_get_pointer_info(&pointer);

	windows_open_centred_at_pointer(convert_savepdf_window, &pointer);
	icons_put_caret_at_end(convert_savepdf_window, SAVE_PDF_ICON_NAME);
}


/**
 * Handle the closure of the file save dialogue following a successful data xfer protocol or similar.
 * The settings are retrieved, and a conversion process is started.
 *
 * \param *output_file		The file to save the PDF as.
 */

static void convert_save_dialogue_end(char *output_file)
{
	conversion_params	params;

	/* Sort out the filenames. */

	string_ctrl_copy(params.output_filename, output_file, CONVERT_MAX_FILENAME);
	icons_strncpy(convert_savepdf_window, SAVE_PDF_ICON_NAME, output_file);

	/* Read and store the options from the window. */

	params.preprocess_in_ps2ps = icons_get_selected(convert_savepdf_window, SAVE_PDF_ICON_PREPROCESS);
	string_ctrl_copy(params.pdfmark_userfile, icons_get_indirected_text_addr(convert_savepdf_window, SAVE_PDF_ICON_USERFILE), CONVERT_MAX_FILENAME);

	/* Launch the conversion process. */

	conversion_in_progress = convert_progress(&params);

	if (!conversion_in_progress) {
		conversion_task = 0;
		conversion_in_progress = FALSE;
		convert_remove_current_conversion();
		convert_notify_completion(FALSE);
	}
}


/**
 * Handle the closure of the file save dialogue, following a click on the
 * Queue icon.  Any files in the queue being processed are changed to
 * HELD_IN_QUEUE.
 */

static void convert_save_dialogue_queue(void)
{
	char			*leafname, filename[CONVERT_MAX_FILENAME];
	queued_file		*list;

	/* Sort out the filenames. */

	string_ctrl_copy(filename, icons_get_indirected_text_addr(convert_savepdf_window, SAVE_PDF_ICON_NAME), CONVERT_MAX_FILENAME);
	leafname = string_find_leafname(filename);

	list = queue;

	while (list != NULL) {
		if (list->object_type == BEING_PROCESSED) {
			list->object_type = HELD_IN_QUEUE;

			string_copy(list->display_name, leafname, MAX_DISPLAY_NAME);

			list->include = TRUE;
		}

		list = list->next;
	}

	if (windows_get_open (convert_queue_window)) {
		convert_reorder_queue_from_index();
		convert_rebuild_queue_index();
		windows_redraw(convert_queue_pane);
	}

	conversion_in_progress = FALSE;
}


/**
 * Process files being saved into the Create PDF window.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show the message was handled; else FALSE.
 */

static osbool convert_handle_save_icon_drop(wimp_message *message)
{
	wimp_full_message_data_xfer	*dataload = (wimp_full_message_data_xfer *) message;
	char				*extension, *leaf, path[212];

	if (dataload->w != convert_savepdf_window)
		return FALSE;

	if (dataload != NULL && dataload->w == convert_savepdf_window && dataload->file_type == dataxfer_TYPE_PRINTPDF) {
		if (bookmark_load_and_select_file(&bookmark, dataload->file_name))
			bookmark_fill_field(convert_savepdf_window, SAVE_PDF_ICON_BOOKMARK_FIELD, &bookmark);
	} else if (dataload != NULL && dataload->w == convert_savepdf_window) {
		switch (dataload->i) {
		case SAVE_PDF_ICON_NAME:
			string_copy(path, dataload->file_name, 212);

			extension = string_find_extension(path);
			leaf = string_strip_extension(path);
			string_find_pathname(path);

			if (string_nocase_strcmp(extension, "pdf") != 0) {
				icons_printf(convert_savepdf_window, SAVE_PDF_ICON_NAME, "%s.%s/pdf", path, leaf);

				icons_replace_caret_in_window (dataload->w);
				wimp_set_icon_state (dataload->w, dataload->i, 0, 0);
			}
			break;

		case SAVE_PDF_ICON_USERFILE:
			if (dataload->file_type == 0xfff) {
				icons_strncpy(convert_savepdf_window, SAVE_PDF_ICON_USERFILE, dataload->file_name);
				icons_replace_caret_in_window(dataload->w);
				wimp_set_icon_state(dataload->w, dataload->i, 0, 0);
			}
			break;
		}
	}

	return TRUE;
}


/**
 * Start or progress a conversion.
 *
 * This function maintains state between calls, so it can be called to move
 * the conversion on from one stage to the next as the child tasks terminate.
 *
 * \param *params		The parameters for the conversion to be launched,
 *				or NULL for none.
 * \return			TRUE if the conversion is complete; else FALSE.
 */

static osbool convert_progress(conversion_params *params)
{
	static enum conversion_state	conversion_state = CONVERSION_STOPPED;
	static char			output_file[CONVERT_MAX_FILENAME];
	static char			pdfmark_file[CONVERT_MAX_FILENAME];
	static int			preprocess_in_ps2ps;

	char				intermediate_file[CONVERT_MAX_FILENAME], *intermediate_leaf="inter";
	queued_file			*list, *new, **end = NULL;
	os_error			*err;

	/* If conversion parameters have been passed in and the conversion is stopped, reset and start a new process.
	 */

	if (conversion_state == CONVERSION_STOPPED && params != NULL) {
		string_copy(output_file, params->output_filename, CONVERT_MAX_FILENAME);
		string_copy(pdfmark_file, params->pdfmark_userfile, CONVERT_MAX_FILENAME);

		preprocess_in_ps2ps = params->preprocess_in_ps2ps;

		conversion_state = CONVERSION_STARTING;
	}

	/* The state machine to handle the steps in the process. */

	switch (conversion_state) {
	case CONVERSION_STARTING:
		err = xosfile_create(output_file, 0xdeaddead, 0xdeaddead, 0);

		if (err == NULL) {
			if (preprocess_in_ps2ps) {
				convert_build_queue_filename(intermediate_file, CONVERT_MAX_FILENAME, intermediate_leaf);
				conversion_state = (convert_launch_ps2ps(intermediate_file)) ? CONVERSION_PS2PS_PENDING : CONVERSION_STOPPED;
			} else {
				conversion_state = (convert_launch_ps2pdf(output_file, pdfmark_file)) ? CONVERSION_PS2PDF_PENDING : CONVERSION_STOPPED;
			}
		} else {
			error_msgs_report_error("FOpenFailed");
			conversion_state = CONVERSION_STOPPED;
		}
		break;

	case CONVERSION_PS2PS_PENDING:
		conversion_state = CONVERSION_PS2PS;
		break;

	case CONVERSION_PS2PS:
		list = queue;

		while (list != NULL) {
			if (list->object_type == BEING_PROCESSED)
				list->object_type = DISCARDED;

			end = &(list->next);
			list = list->next;
		}

		new = malloc(sizeof(queued_file));

		if (new != NULL) {
			string_copy(new->filename, intermediate_leaf, MAX_QUEUE_NAME);
			*(new->display_name) = '\0';
			new->object_type = BEING_PROCESSED;
			new->next = NULL;

			if (end != NULL)
				*end = new;

			conversion_state = (convert_launch_ps2pdf(output_file, pdfmark_file)) ? CONVERSION_PS2PDF_PENDING : CONVERSION_STOPPED;
		} else {
			conversion_state = CONVERSION_STOPPED;
		}
		break;

	case CONVERSION_PS2PDF_PENDING:
		conversion_state = CONVERSION_PS2PDF;
		break;

	case CONVERSION_PS2PDF:
			osfile_set_type(output_file, dataxfer_TYPE_PDF);

			if (config_opt_read("PopUpAfter"))
				popup_open(config_int_read("PopUpTime"));

			conversion_state = CONVERSION_STOPPED;
			break;

	case CONVERSION_STOPPED:
		break;
	}

	/* Exit, signalling FALSE if the process has ended. */

	return (conversion_state != CONVERSION_STOPPED) ? TRUE : FALSE;
}


/**
 * Launch ps2ps on the files listed in the file queue, outputting the resulting
 * PS file to the given filename.
 *
 * \param *file_out		The name of the output file to save to.
 * \return			TRUE if the conversion starts; else FALSE.
 */

static osbool convert_launch_ps2ps(char *file_out)
{
	char		command[CONVERT_COMMAND_LENGTH], taskname[32];
	queued_file	*list;
	FILE		*param_file;
	os_error	*error = NULL;
	wimp_t		started_task;

	msgs_lookup("ChildTaskName", taskname, sizeof(taskname));

	param_file = fopen(config_str_read("ParamFile"), "w");

	if (param_file != NULL) {
		/* Write all the conversion options and filename details to the gs parameters file. */

		fprintf(param_file, "-dSAFER -q -dNOPAUSE -dBATCH -sDEVICE=pswrite -sOutputFile=%s", file_out);

		list = queue;

		while (list != NULL) {
			if (list->object_type == BEING_PROCESSED)
				fprintf(param_file, " %s.%s", config_str_read("FileQueue"), list->filename);

			list = list->next;
		}

		fclose(param_file);

		/* Write all the taskwindow command line details to the command string. */

		string_printf(command, CONVERT_COMMAND_LENGTH, "TaskWindow \"gs @%s\" %dk -name \"%s\" -quit",
				config_str_read("ParamFile"), config_int_read("TaskMemory"), taskname);

		/* Launch the conversion task. */

		error = xwimp_start_task(command, &started_task);
	}

	return (error == NULL && started_task != 0) ? TRUE : FALSE;
}


/**
 * Launch ps2pdf, using the file at the top of the queue and the filename retrieved from the Save dialogue.
 * This will either be at a drag end, or as a result of the user clicking 'OK' on a full filename.
 *
 * To get around command line length restrictions on RISC OS 3.x, we dump the bulk of the parameters into a file
 * in PipeFS and pass this in to gs as a parameters file using the @ parameter.
 *
 * \param *file_out		The file to save the PDF as.
 * \param *user_pdfmark_file	A user-supplied PDFMark file's pathname, if required.
 * \return			TRUE if the conversion started; else FALSE.
 */

static osbool convert_launch_ps2pdf(char *file_out, char *user_pdfmark_file)
{
	char		command[CONVERT_COMMAND_LENGTH], taskname[32], encrypt_buf[1024], optimize_buf[1024], version_buf[1024], paper_buf[1024], queue_path[4096];
	queued_file	*list;
	FILE		*param_file, *pdfmark_file;
	int		queue_left;
	os_error	*error = NULL;
	wimp_t		started_task;

	/* Get a canonicalised version of the queue pathname. */

	error = xosfscontrol_canonicalise_path(config_str_read("FileQueue"), queue_path, NULL, NULL, 4096, &queue_left);
	if (error != NULL || queue_left < 0)
		return FALSE;

	/* Find the name to use for the child task. */

	msgs_lookup("ChildTaskName", taskname, sizeof(taskname));

	/* Start to write the parameters file. */

	param_file = fopen(config_str_read("ParamFile"), "w");
	if (param_file != NULL) {
		/* Generate a PDFMark file if necessary. */

		if (pdfmark_data_available(&pdfmark) || bookmark_data_available(&bookmark)) {
			pdfmark_file = fopen (config_str_read ("PDFMarkFile"), "w");

			if (pdfmark_file != NULL) {
				pdfmark_write_docinfo_file(pdfmark_file, &pdfmark);
				bookmarks_write_pdfmark_out_file(pdfmark_file, &bookmark);

				fclose(pdfmark_file);
			}
		}

		/* Write all the conversion options and filename details to the gs parameters file. */

		version_build_params(version_buf, sizeof(version_buf), &version);
		optimize_build_params(optimize_buf, sizeof(optimize_buf), &optimization);
		encryption_build_params(encrypt_buf, sizeof(encrypt_buf), &encryption, version.standard_version >= 2);
		paper_build_params(paper_buf, sizeof(paper_buf), &paper);

		fprintf(param_file, "-dSAFER %s%s%s%s -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite "
				"-sOutputFile=%s -c .setpdfwrite save pop -f",
				version_buf, optimize_buf, encrypt_buf, paper_buf, file_out);

		list = queue;

		while (list != NULL) {
			if (list->object_type == BEING_PROCESSED)
				fprintf(param_file, " %s.%s", queue_path, list->filename);

			list = list->next;
		}

		/* If there is a PDFMark file, pass that in too. */

		if (osfile_read_stamped_no_path(config_str_read("PDFMarkFile"), NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_FILE)
			fprintf(param_file, " %s", config_str_read("PDFMarkFile"));

		/* If there is a PDFMark User File, pass that in too. */

		if (*user_pdfmark_file != '\0' &&
				osfile_read_stamped_no_path(user_pdfmark_file, NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_FILE)
			fprintf(param_file, " %s", user_pdfmark_file);

		fclose(param_file);

		/* Write all the taskwindow command line details to the command string. */

		string_printf(command, CONVERT_COMMAND_LENGTH, "TaskWindow \"gs @%s\" %dk -name \"%s\" -quit",
				config_str_read("ParamFile"), config_int_read("TaskMemory"), taskname);

		#ifdef DEBUG
		debug_printf("Command (length %d): '%s'", strlen(command), command);
		#endif

		/* Launch the conversion task. */

		error = xwimp_start_task(command, &started_task);
	}

	return (error == NULL && started_task != 0) ? TRUE : FALSE;
}


/**
 * Process Message_TaskInitialise, to see if the task that has started has the name
 * of our child task. If it has, make note of its handle and move the conversion
 * state machine on a step.
 *
 * \param *message		The message data block.
 * \return			FALSE to allow other claimants to see the message.
 */

static osbool convert_check_for_conversion_start(wimp_message *message)
{
	char	taskname[32];
	wimp_full_message_task_initialise *task_initialise = (wimp_full_message_task_initialise *) message;

	/* Find the name to use for the child task. */

	msgs_lookup("ChildTaskName", taskname, sizeof(taskname));

	if (task_initialise == NULL || strcmp(task_initialise->task_name, taskname) != 0)
		return FALSE;

	if (convert_progress(NULL)) {
		conversion_task = task_initialise->sender;
	} else {
		conversion_task = 0;
		conversion_in_progress = FALSE;
		convert_remove_current_conversion();
		convert_notify_completion(FALSE);
//FIXME - conversion failed? Or could it have completed here?
	}

	return FALSE;
}


/**
 * Process Message_TaskCloseDown, to see if the task that ended had the same task handle as the current
 * conversion task.  If it did, establish what kind of conversion was underway:
 *
 * - If it was *ps2ps, take the intermediate file and pass it on to *ps2pdf.
 * - If it was *ps2pdf, reset the flags and take the original queued object from the queue head.
 *
 * \param *message		The message data block.
 * \return			FALSE to allow other claimants to see the message.
 */

static osbool convert_check_for_conversion_end(wimp_message *message)
{
	if (message != NULL && message->sender == conversion_task && !convert_progress(NULL)) {
		conversion_task = 0;
		conversion_in_progress = FALSE;
		convert_remove_current_conversion();
		convert_notify_completion(TRUE);
//FIXME conversion finished. Is it successful? send message if necessary
	}

	return FALSE;
}


/**
 * Process PrintPDF_Control which allows another task to set the file to save
 * the next job to.
 *
 * \param *message		The message data block.
 * \return			FALSE to allow other claimants to see the message.
 */

static osbool convert_printpdf_control(wimp_message *message)
{
	control_message *control = (control_message *) message;

	if (control != NULL && control->reason == CONTROL_SET_FILENAME) {
//FIXME: Would need to check for null filename. What if conversion in progress?
		if (request_task == NULL) {
			request_task = control->sender;
			request_ref = control->my_ref;
			string_copy(request_filename, control->filename, CONVERT_MAX_FILENAME);

			control->reason = CONTROL_REPORT_SUCCESS;
			control->size = 24;
		} else {
			control->reason = CONTROL_REPORT_FAILURE;
			control->size = 24;
		}

		message->your_ref = message->my_ref;
		wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

		return TRUE;
	}

	return FALSE;
}


/**
 * Called when conversion completed. Checks if there is a requesting task and
 * notifies the task of the result.
 */

static void convert_notify_completion(osbool success)
{
	if (request_task != NULL) {

		control_message control;

		control.your_ref = request_ref;
		control.action = message_PRINTPDF_CONTROL;
		control.reason = success ? CONTROL_REPORT_SUCCESS : CONTROL_REPORT_FAILURE;
		control.size = 24;

		wimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &control, request_task);
		request_task = NULL;
		request_ref = 0;
	}
}


/**
 * Called to cancel the conversion that is being set up.  Reset the flags,
 * close the window and remove the item from the queue.
 */

static void convert_cancel_conversion(void)
{
	wimp_close_window(convert_savepdf_window);
	conversion_task = 0;
	conversion_in_progress = FALSE;

	convert_remove_current_conversion();
}


/**
 * Create a full pathname for a file in the processing queue folder.
 *
 * \param *buffer		Pointer to the buffer to hold the pathname.
 * \param len			The size of the supplied buffer.
 * \param *leaf			Pointer to the leafname to use.
 * \return			Pointer to the pathname in the buffer, or NULL.
 */

char *convert_build_queue_filename(char *buffer, size_t len, char *leaf)
{
	if (buffer == NULL || len == 0)
		return NULL;

	if (leaf != NULL)
		string_printf(buffer, len, "%s.%s", config_str_read("FileQueue"), leaf);
	else
		*buffer = '\0';

	return buffer;
}


/**
 * Called by modules to ask the converion system to re-validate its parameters.
 */

void convert_validate_params(void)
{
	if (bookmark_validate_params(&bookmark))
		bookmark_fill_field(convert_savepdf_window, SAVE_PDF_ICON_BOOKMARK_FIELD, &bookmark);
}


/**
 * Process mouse clicks in the Save PDF dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void convert_save_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case SAVE_PDF_ICON_FILE:
		if (pointer->buttons == wimp_DRAG_SELECT)
			dataxfer_save_window_drag(pointer->w, SAVE_PDF_ICON_FILE, convert_drag_end_handler, NULL);
		break;

	case SAVE_PDF_ICON_OK:
		if (pointer->buttons == wimp_CLICK_SELECT)
			convert_immediate_window_save();
		break;

	case SAVE_PDF_ICON_CANCEL:
		if (pointer->buttons == wimp_CLICK_SELECT)
			convert_cancel_conversion();
		break;

	case SAVE_PDF_ICON_QUEUE:
		if (pointer->buttons == wimp_CLICK_SELECT) {
			convert_save_dialogue_queue();
			wimp_close_window(convert_savepdf_window);
		}
		break;

	case SAVE_PDF_ICON_ENCRYPT_MENU:
		encrypt_set_dialogue_callback(convert_process_encrypt_dialogue);
		encrypt_open_dialogue(&encryption, version.standard_version >= 2, pointer);
		break;

//	case SAVE_PDF_ICON_PAPER_MENU:
//		paper_set_dialogue_callback(convert_process_paper_dialogue);
//		paper_open_dialogue(&paper, pointer);
//		break;

	case SAVE_PDF_ICON_PDFMARK_MENU:
		pdfmark_set_dialogue_callback(convert_process_pdfmark_dialogue);
		pdfmark_open_dialogue(&pdfmark, pointer);
		break;
		break;
	}
}


/**
 * Process keypresses in the Save PDF window.
 *
 * \param *key		The keypress event block to handle.
 * \return		TRUE if the event was handled; else FALSE.
 */

static osbool convert_save_keypress_handler(wimp_key *key)
{
	if (key == NULL)
		return FALSE;

	switch (key->c) {
	case wimp_KEY_RETURN:
		convert_immediate_window_save();
		break;

	case wimp_KEY_ESCAPE:
		convert_cancel_conversion();
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}


/**
 * Process menu prepare events in the Save PDF window.
 *
 * \param w		The handle of the owning window.
 * \param *menu		The menu handle.
 * \param *pointer	The pointer position, or NULL for a re-open.
 */

static void convert_save_menu_prepare_handler(wimp_w w, wimp_menu *menu, wimp_pointer *pointer)
{
	if (menu == popup_version)
		version_set_menu(&version, popup_version);
	else if (menu == popup_optimize)
		optimize_set_menu(&optimization, popup_optimize);
	else if (menu == popup_paper)
		paper_set_menu(&paper, popup_paper);
	else if (menu == popup_bookmark) {
		popup_bookmark = bookmark_build_menu(&bookmark);
		event_set_menu_block(popup_bookmark);
		ihelp_add_menu(popup_bookmark, "BookmarkListMenu");
	}
}


/**
 * Process menu selection events in the Save PDF window.
 *
 * \param w		The handle of the owning window.
 * \param *menu		The menu handle.
 * \param *selection	The menu selection details.
 */

static void convert_save_menu_selection_handler(wimp_w w, wimp_menu *menu, wimp_selection *selection)
{
	if (menu == popup_version) {
		version_process_menu(&version, popup_version, selection);
		version_fill_field(w, SAVE_PDF_ICON_VERSION_FIELD, &version);
	} else if (menu == popup_optimize) {
		optimize_set_dialogue_callback(convert_process_optimize_dialogue);
		optimize_process_menu(&optimization, popup_optimize, selection);
		optimize_fill_field(w, SAVE_PDF_ICON_OPT_FIELD, &optimization);
	} else if (menu == popup_paper) {
		paper_set_dialogue_callback(convert_process_paper_dialogue);
		paper_process_menu(&paper, popup_paper, selection);
		paper_fill_field(w, SAVE_PDF_ICON_PAPER_FIELD, &paper);
	} else if (menu == popup_bookmark) {
		bookmark_process_menu(&bookmark, selection);
		bookmark_fill_field(w, SAVE_PDF_ICON_BOOKMARK_FIELD, &bookmark);
	}
}


/**
 * Process monu close events in the SavePDF window.
 *
 * \param w		The handle of the owning window.
 * \param *menu		The menu handle.
 */

static void convert_save_menu_close_handler(wimp_w w, wimp_menu *menu)
{
	if (menu == popup_bookmark) {
		ihelp_remove_menu(popup_bookmark);
	}
}

/**
 * Try to save in response to a click on 'OK' in the Save dialogue.
 *
 * \return 		TRUE if the process completed OK; else FALSE.
 */

static osbool convert_immediate_window_save(void)
{
	char			*filename;

	filename = icons_get_indirected_text_addr(convert_savepdf_window, SAVE_PDF_ICON_NAME);

	/* Test if the filename is valid. */

	if (strchr (filename, '.') == NULL) {
		error_msgs_report_info("DragSave");
		return FALSE;
	}

	convert_save_dialogue_end(filename);

	wimp_close_window(convert_savepdf_window);

	return TRUE;
}


/**
 * Process the termination of icon drags from the Convert dialogue.
 *
 * \param *pointer		The pointer location at the end of the drag.
 * \param *data			Data pointer (unused).
 */

static void convert_drag_end_handler(wimp_pointer *pointer, void *data)
{
	char			*leafname;

	leafname = string_find_leafname(icons_get_indirected_text_addr(convert_savepdf_window, SAVE_PDF_ICON_NAME));

	dataxfer_start_save(pointer, leafname, 0, dataxfer_TYPE_PDF, 0, convert_drag_end_save_handler, NULL);
}


/**
 * Callback handler for DataSave completion on PDF save drags: start the
 * conversion using the filename returned.
 *
 * \param *filename		The filename returned by the DataSave protocol.
 * \param *data			Data pointer (unused).
 * \return			TRUE if the save was started OK; else FALSE.
 */

static osbool convert_drag_end_save_handler(char *filename, void *data)
{
	convert_save_dialogue_end(filename);
	wimp_close_window(convert_savepdf_window);

	return TRUE;
}


/**
 * Callback to respond to clicks on the OK button of the PDFMark
 * dialogue.
 */

static void convert_process_pdfmark_dialogue(void)
{
	pdfmark_process_dialogue(&pdfmark);
	pdfmark_fill_field(convert_savepdf_window, SAVE_PDF_ICON_PDFMARK_FIELD, &pdfmark);
}


/**
 * Callback to respond to clicks on the OK button of the encryption
 * dialogue.
 */

static void convert_process_encrypt_dialogue(void)
{
	encrypt_process_dialogue(&encryption);
	encrypt_fill_field(convert_savepdf_window, SAVE_PDF_ICON_ENCRYPT_FIELD, &encryption);
}


/**
 * Callback to respond to clicks on the OK button of the optimization
 * dialogue.
 */

static void convert_process_optimize_dialogue(void)
{
	optimize_process_dialogue(&optimization);
	optimize_fill_field(convert_savepdf_window, SAVE_PDF_ICON_OPT_FIELD, &optimization);
}


/**
 * Callback to respond to clicks on the OK button of the paper
 * dialogue.
 */

static void convert_process_paper_dialogue(void)
{
	paper_process_dialogue(&paper);
	paper_fill_field(convert_savepdf_window, SAVE_PDF_ICON_PAPER_FIELD, &paper);
}


/**
 * Remove the current item or items from the queue, deleting them from the
 * Scrap directory.
 */

static void convert_remove_current_conversion(void)
{
	queued_file		**list, *old;
	char			old_file[CONVERT_MAX_FILENAME];

	list = &queue;

	while (*list != NULL) {
		if ((*list)->object_type == BEING_PROCESSED || (*list)->object_type == DISCARDED) {
			old = (*list);
			convert_build_queue_filename(old_file, CONVERT_MAX_FILENAME, old->filename);
			xosfile_delete(old_file, NULL, NULL, NULL, NULL, NULL);

			*list = ((*list)->next);

			free(old);
		} else {
			list = &((*list)->next);
		}
	}
}


/**
 * Remove deleted items from the queue, deleting them from the Scrap directory.
 */

static void convert_remove_deleted_files(void)
{
	queued_file	**list, *old;
	char		old_file[CONVERT_MAX_FILENAME];

	list = &queue;

	while (*list != NULL) {
		if ((*list)->object_type == DELETED) {
			old = (*list);
			convert_build_queue_filename(old_file, CONVERT_MAX_FILENAME, old->filename);
			xosfile_delete(old_file, NULL, NULL, NULL, NULL, NULL);

			*list = ((*list)->next);

			free(old);
		} else {
			list = &((*list)->next);
		}
	}
}


/**
 * Remove the first item from the queue, deleting it from the Scrap directory.
 */

void convert_remove_first_conversion(void)
{
	queued_file	*old;
	char		old_file[CONVERT_MAX_FILENAME];

	old = queue;
	convert_build_queue_filename(old_file, CONVERT_MAX_FILENAME, old->filename);
	xosfile_delete(old_file, NULL, NULL, NULL, NULL, NULL);

	queue = old->next;
	free(old);
}


/**
 * Remove all the items from the queue, and delete their files.
 */

void convert_remove_all_remaining_conversions(void)
{
	while (queue != NULL)
		convert_remove_first_conversion();
}


/**
 * Return an indication that a conversion is underway.
 *
 * \return		TRUE if a conversion is in progress; else FALSE.
 */

osbool convert_pdf_conversion_in_progress(void)
{
	return conversion_in_progress;
}


/**
 * Return an indication of whether any queued files are left.
 *
 * \return			TRUE if there are files to be saved; else FALSE.
 */

osbool convert_pending_files_in_queue(void)
{
	wimp_error_box_selection	button = wimp_ERROR_BOX_SELECTED_NOTHING;

	if (queue != NULL)
		button = error_msgs_report_question("PendingJobs", "PendingJobsB");

	return (button == 4) ? TRUE : FALSE;
}


/**
 * Open the queue dialogue at the pointer.
 *
 * \param *pointer		The wimp pointer data.
 */

void convert_open_queue_window(wimp_pointer *pointer)
{
	windows_open_with_pane_centred_at_pointer(convert_queue_window, convert_queue_pane, QUEUE_ICON_PANE, 40, pointer);
	convert_rebuild_queue_index();
}


/**
 * Close the queue dialogue.
 */

static void convert_close_queue_window(void)
{
	convert_reorder_queue_from_index();
	convert_remove_deleted_files();
	wimp_close_window(convert_queue_window);
}


/**
 * Rebuild the queue dialogue index from the queue data and resize the
 * dialogue pane to suit.
 *
 * \TODO -- We need to fix this so that memory allocation is done correctly.
 *          This code should also resize the queue window pane.
 */

static void convert_rebuild_queue_index(void)
{
	wimp_window_state	state;
	os_box			extent;
	queued_file		*list;
	int			length, visible_extent, new_extent, new_scroll;

	/* Get the length of the queue. */

	length = 0;
	list = queue;

	while (list != NULL) {
		length++;
		list = list->next;
	}

	/* If there is already a redraw list, free it... */

	if (queue_redraw_list != NULL)
		free(queue_redraw_list);

	/* ... and then allocate memory to store all the entries. */

	queue_redraw_list = (queued_file **) malloc(length * sizeof(queued_file **));

	/* Populate the list. */

	list = queue;
	queue_redraw_lines = 0;

	while (list != NULL) {
		if (list->object_type == HELD_IN_QUEUE || list->object_type == DELETED)
			queue_redraw_list[queue_redraw_lines++] = list;

		list = list->next;
	}

	/* Set the window extent. */

	state.w = convert_queue_pane;
	wimp_get_window_state(&state);

	visible_extent = state.yscroll + (state.visible.y0 - state.visible.y1);

	new_extent = -QUEUE_ICON_HEIGHT * length;

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

	wimp_set_extent(convert_queue_pane, &extent);
}


/**
 * Re-order the linked list so that entries from the queue appear in the same
 * order that they do in the queue list window.
 */

static void convert_reorder_queue_from_index(void)
{
	queued_file	**list = NULL;
	int		line;

	if(queue_redraw_lines <= 0)
		return;

	for (line=queue_redraw_lines - 1; line >= 0; line--) {
		list = &queue;

		while (*list != NULL && *list != queue_redraw_list[line])
			list = &((*list)->next);

		if (*list != NULL) {
			*list = (*list)->next;
			(queue_redraw_list[line])->next = queue;
			queue = queue_redraw_list[line];
		}
	}
}


/**
 * Process redraw requests for the Queue dialogue pane.
 *
 * \param *redraw		The redraw event block to handle.
 */

static void convert_queue_pane_redraw_handler(wimp_draw *redraw)
{
	int			ox, oy, top, base, y;
	osbool			more;
	wimp_icon		*icon;

	/* Perform the redraw if a window was found. */

	if (redraw->w != convert_queue_pane)
		return;

	more = wimp_redraw_window(redraw);

	ox = redraw->box.x0 - redraw->xscroll;
	oy = redraw->box.y1 - redraw->yscroll;

	icon = convert_queue_pane_def->icons;

	while (more) {
		top = (oy - redraw->clip.y1) / QUEUE_ICON_HEIGHT;
		if (top < 0)
			top = 0;

		base = (QUEUE_ICON_HEIGHT + (QUEUE_ICON_HEIGHT / 2) + oy - redraw->clip.y0) / QUEUE_ICON_HEIGHT;

		for (y = top; y < queue_redraw_lines && y <= base; y++) {
			icon[QUEUE_PANE_INCLUDE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
			icon[QUEUE_PANE_INCLUDE].extent.y0 = icon[QUEUE_PANE_INCLUDE].extent.y1 - QUEUE_ICON_HEIGHT;
			icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.id =
					(osspriteop_id) (((queue_redraw_list[y])->include) ? "opton" : "optoff");
			icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.area = (osspriteop_area *) 1;
			icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.size = 12;

			wimp_plot_icon(&(icon[QUEUE_PANE_INCLUDE]));

			icon[QUEUE_PANE_FILE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
			icon[QUEUE_PANE_FILE].extent.y0 = icon[QUEUE_PANE_FILE].extent.y1 - QUEUE_ICON_HEIGHT;
			icon[QUEUE_PANE_FILE].data.indirected_text_and_sprite.text = (queue_redraw_list[y])->display_name;
			icon[QUEUE_PANE_FILE].data.indirected_text_and_sprite.size = MAX_DISPLAY_NAME;

			wimp_plot_icon(&(icon[QUEUE_PANE_FILE]));

			icon[QUEUE_PANE_DELETE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
			icon[QUEUE_PANE_DELETE].extent.y0 = icon[QUEUE_PANE_DELETE].extent.y1 - QUEUE_ICON_HEIGHT;
			icon[QUEUE_PANE_DELETE].data.indirected_sprite.id =
					(osspriteop_id) (((queue_redraw_list[y])->object_type == DELETED) ? "del1" : "del0");
			icon[QUEUE_PANE_DELETE].data.indirected_sprite.area = main_wimp_sprites;
			icon[QUEUE_PANE_DELETE].data.indirected_sprite.size = 12;

			wimp_plot_icon(&(icon[QUEUE_PANE_DELETE]));
		}

		more = wimp_get_rectangle (redraw);
	}
}


/**
 * Process mouse clicks in the Queue dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void convert_queue_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case QUEUE_ICON_CLOSE:
		convert_close_queue_window();
		break;

	case QUEUE_ICON_CREATE:
		convert_close_queue_window();
		convert_start_held_conversion();
		break;
	}
}


/**
 * Process mouse clicks in the Queue dialogue pane.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void convert_queue_pane_click_handler(wimp_pointer *pointer)
{
	int			line, column, xpos;
	wimp_window_state	window;
	wimp_icon		*icon;

	window.w = pointer->w;
	wimp_get_window_state(&window);

	icon = convert_queue_pane_def->icons;

	line = ((window.visible.y1 - pointer->pos.y) - window.yscroll) / QUEUE_ICON_HEIGHT;

	if (line < 0 || line >= queue_redraw_lines)
		line = -1;

	column = -1;

	if (line > -1) {
		xpos = (pointer->pos.x - window.visible.x0) + window.xscroll;

		if (icon[QUEUE_PANE_INCLUDE].extent.x0 <= xpos && icon[QUEUE_PANE_INCLUDE].extent.x1 >= xpos)
			column = QUEUE_PANE_INCLUDE;
		else if (icon[QUEUE_PANE_FILE].extent.x0 <= xpos && icon[QUEUE_PANE_FILE].extent.x1 >= xpos)
			column = QUEUE_PANE_FILE;
		else if (icon[QUEUE_PANE_DELETE].extent.x0 <= xpos && icon[QUEUE_PANE_DELETE].extent.x1 >= xpos)
			column = QUEUE_PANE_DELETE;
	}

	if (pointer->buttons == wimp_CLICK_SELECT && column == QUEUE_PANE_INCLUDE && line != -1) {
		(queue_redraw_list[line])->include = !(queue_redraw_list[line])->include;
		wimp_force_redraw(pointer->w,
				icon[QUEUE_PANE_INCLUDE].extent.x0, -((line + 1)* QUEUE_ICON_HEIGHT),
				icon[QUEUE_PANE_INCLUDE].extent.x1, -(line * QUEUE_ICON_HEIGHT));
	} else if (pointer->buttons == wimp_CLICK_SELECT && column == QUEUE_PANE_DELETE && line != -1) {
		if ((queue_redraw_list[line])->object_type == HELD_IN_QUEUE)
			(queue_redraw_list[line])->object_type = DELETED;
		else
			(queue_redraw_list[line])->object_type = HELD_IN_QUEUE;
		 wimp_force_redraw (pointer->w,
				icon[QUEUE_PANE_DELETE].extent.x0, -((line + 1)* QUEUE_ICON_HEIGHT),
				icon[QUEUE_PANE_DELETE].extent.x1, -(line * QUEUE_ICON_HEIGHT));
	} else if (pointer->buttons == wimp_DRAG_SELECT && column == QUEUE_PANE_FILE && line != -1) {
		convert_start_queue_entry_drag(line);
	}
}


/**
 * Start dragging a line in the queue dialogue pane.
 *
 * \param		The line of the pane to be dragged.
 */

static void convert_start_queue_entry_drag(int line)
{
	wimp_window_state	window;
	wimp_auto_scroll_info	auto_scroll;
	wimp_drag		drag;
	int			ox, oy;

	/* Get the basic information about the window. */

	window.w = convert_queue_pane;
	wimp_get_window_state(&window);

	ox = window.visible.x0 - window.xscroll;
	oy = window.visible.y1 - window.yscroll;

	/* Set up the drag parameters. */

	drag.w = convert_queue_pane;
	drag.type = wimp_DRAG_USER_FIXED;

	drag.initial.x0 = ox;
	drag.initial.y0 = oy + -(line * QUEUE_ICON_HEIGHT + QUEUE_ICON_HEIGHT);
	drag.initial.x1 = ox + (window.visible.x1 - window.visible.x0);
	drag.initial.y1 = oy + -(line * QUEUE_ICON_HEIGHT);

	drag.bbox.x0 = window.visible.x0;
	drag.bbox.y0 = window.visible.y0;
	drag.bbox.x1 = window.visible.x1;
	drag.bbox.y1 = window.visible.y1;

	/* Read CMOS RAM to see if solid drags are required. */

	dragging_sprite = ((osbyte2(osbyte_READ_CMOS, osbyte_CONFIGURE_DRAG_ASPRITE, 0) &
			osbyte_CONFIGURE_DRAG_ASPRITE_MASK) != 0) ? TRUE : FALSE;

	if (FALSE && dragging_sprite) /* This is never used, though it could be... */
		dragasprite_start (dragasprite_HPOS_CENTRE | dragasprite_VPOS_CENTRE | dragasprite_NO_BOUND |
				dragasprite_BOUND_POINTER | dragasprite_DROP_SHADOW, wimpspriteop_AREA,
				"", &(drag.initial), &(drag.bbox));
	else
		wimp_drag_box(&drag);

	/* Initialise the autoscroll. */

	if (xos_swi_number_from_string("Wimp_AutoScroll", NULL) == NULL) {
		auto_scroll.w = convert_queue_pane;
		auto_scroll.pause_zone_sizes.x0 = 0;
		auto_scroll.pause_zone_sizes.y0 = AUTO_SCROLL_MARGIN;
		auto_scroll.pause_zone_sizes.x1 = 0;
		auto_scroll.pause_zone_sizes.y1 = AUTO_SCROLL_MARGIN;
		auto_scroll.pause_duration = 0;
		auto_scroll.state_change = (void *) 1;

		wimp_auto_scroll(wimp_AUTO_SCROLL_ENABLE_VERTICAL, &auto_scroll);
	}

	dragging_start_line = line;
	event_set_drag_handler(convert_terminate_queue_entry_drag, NULL, NULL);
}

/**
 * Callback handler for queue window drag termination.
 *
 * \param  *drag		The Wimp poll block from termination.
 * \param  *data		NULL (unused).
 */

static void convert_terminate_queue_entry_drag(wimp_dragged *drag, void *data)
{
	wimp_pointer		pointer;
	wimp_window_state	window;
	int			line, i;
	queued_file		*moved;

	/* Terminate the drag and end the autoscroll. */

	if (xos_swi_number_from_string ("Wimp_AutoScroll", NULL) == NULL)
		wimp_auto_scroll (0, NULL);

	if (dragging_sprite)
		dragasprite_stop ();

	/* Get the line at which the drag ended. */

	wimp_get_pointer_info (&pointer);

	window.w = convert_queue_pane;
	wimp_get_window_state (&window);

	line = ((window.visible.y1 - pointer.pos.y) - window.yscroll) / QUEUE_ICON_HEIGHT;

	if (line < 0)
		line = 0;
	if (line >= queue_redraw_lines)
		line = queue_redraw_lines - 1;

	moved = queue_redraw_list[dragging_start_line];

	if (dragging_start_line < line)
		for (i=dragging_start_line; i<line; i++)
			queue_redraw_list[i] = queue_redraw_list[i+1];
	else if (dragging_start_line > line)
		for (i=dragging_start_line; i>line; i--)
			queue_redraw_list[i] = queue_redraw_list[i-1];

	queue_redraw_list[line] = moved;

	windows_redraw(convert_queue_pane);
}


/**
 * Callback to decode interactive help in the queue dialogue pane window.
 *
 * \param  *buffer			Buffer to take the help token.
 * \param  w				The wimp window handle.
 * \param  i				The wimp icon handle.
 * \param  pos				The pointer coordinates.
 * \param  buttons			The mouse button state.
 */

static void convert_decode_queue_pane_help(char *buffer, wimp_w w, wimp_i i, os_coord pos, wimp_mouse_state buttons)
{
	int			xpos, ypos, column;
	wimp_window_state	window;
	wimp_icon		*icon;

	icon = convert_queue_pane_def->icons;

	*buffer = '\0';
	column = 0;

	window.w = w;
	wimp_get_window_state(&window);

	xpos = (pos.x - window.visible.x0) + window.xscroll;
	ypos = (window.visible.y1 - pos.y) - window.yscroll;

	if (ypos / QUEUE_ICON_HEIGHT < queue_redraw_lines) {
		if (icon[QUEUE_PANE_INCLUDE].extent.x0 <= xpos && icon[QUEUE_PANE_INCLUDE].extent.x1 >= xpos)
			column = QUEUE_PANE_INCLUDE;
		else if (icon[QUEUE_PANE_FILE].extent.x0 <= xpos && icon[QUEUE_PANE_FILE].extent.x1 >= xpos)
			column = QUEUE_PANE_FILE;
		else if (icon[QUEUE_PANE_DELETE].extent.x0 <= xpos && icon[QUEUE_PANE_DELETE].extent.x1 >= xpos)
			column = QUEUE_PANE_DELETE;

		string_printf(buffer, IHELP_INAME_LEN, "Col%d", column);
	}
}



#ifdef DEBUG
static void traverse_queue(void)
{
	queued_file		*list;
	char			*status;

	list = queue;

	debug_printf("\\BQueue Contents");

	while (list != NULL) {
		switch (list->object_type) {
		case PENDING_ATTENTION:
			status = "Pending";
			break;

		case BEING_PROCESSED:
			status = "Process";
			break;

		case HELD_IN_QUEUE:
			status = "Held";
			break;

		case DISCARDED:
			status = "Discard";
			break;

		case DELETED:
			status = "Deleted";
			break;

		default:
			status = NULL;
		}

		if (status != NULL)
			debug_printf("%7s %15s %s", status, list->filename, list->display_name);

		list = list->next;
	}

	debug_printf("\\rEnd of queue");
}
#endif

