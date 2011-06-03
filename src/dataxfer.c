/* PrintPDF - dataxfer.c
 *
 * (C) Stephen Fryatt, 2005-2011
 */

/* ANSI C header files */

#include <string.h>
#include <stdio.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"
#include "oslib/dragasprite.h"
#include "oslib/osbyte.h"
#include "oslib/wimpspriteop.h"

/* SF-Lib header files. */

#include "sflib/icons.h"
#include "sflib/string.h"
#include "sflib/windows.h"
#include "sflib/transfer.h"
#include "sflib/debug.h"
#include "sflib/errors.h"
#include "sflib/event.h"
#include "sflib/general.h"
#include "sflib/config.h"

/* Application header files */

#include "dataxfer.h"

#include "bookmark.h"
#include "choices.h"
#include "convert.h"
#include "main.h"
#include "windows.h"

/* ==================================================================================================================
 * Global variables.
 */

/**
 * Boolean to indicate whether DragASprite is in use or not.
 */

static int	dragging_sprite = 0;

/**
 * The type of save drag: to differentiate Save PDF from Save As.
 */

static int	drag_type = 0;

/**
 * Function prototypes.
 */

static int		drag_end_save_pdf(char *filename);
static void		terminate_user_drag(wimp_dragged *drag, void *data);
static osbool		message_data_save_reply(wimp_message *message);
static osbool		message_data_save_ack_reply(wimp_message *message);
static osbool		message_data_load_reply(wimp_message *message);
static osbool		start_data_open_load(wimp_message *message);


/**
 * Initialise the data transfer system.
 */

void dataxfer_initialise(void)
{
	event_add_message_handler(message_DATA_SAVE, EVENT_MESSAGE_INCOMING, message_data_save_reply);
	event_add_message_handler(message_DATA_SAVE_ACK, EVENT_MESSAGE_INCOMING, message_data_save_ack_reply);
	event_add_message_handler(message_DATA_LOAD, EVENT_MESSAGE_INCOMING, message_data_load_reply);
	event_add_message_handler(message_DATA_OPEN, EVENT_MESSAGE_INCOMING, start_data_open_load);
}


/**
 * Start dragging the icon from the save dialogue.  Called in response to an attempt to drag the icon.
 *
 * \param type		The drag type to start.
 */

void start_save_window_drag(int type)
{
	wimp_window_state	window;
	wimp_icon_state		icon;
	wimp_drag		drag;
	int			ox, oy;

	extern global_windows	windows;


	/* Get the basic information about the window and icon. */

	switch (type) {
	case DRAG_SAVE_PDF:
		window.w = windows.save_pdf;
		break;
	case DRAG_SAVE_SAVEAS:
		window.w = windows.save_as;
		break;
	}
	wimp_get_window_state (&window);

	ox = window.visible.x0 - window.xscroll;
	oy = window.visible.y1 - window.yscroll;

	icon.w = window.w;
	switch (type) {
	case DRAG_SAVE_PDF:
		icon.i = SAVE_PDF_ICON_FILE;
		break;
	case DRAG_SAVE_SAVEAS:
		icon.i = SAVEAS_ICON_FILE;
		break;
	}
	wimp_get_icon_state (&icon);


	/* Set up the drag parameters. */

	drag.w = window.w;
	drag.type = wimp_DRAG_USER_FIXED;

	drag.initial.x0 = ox + icon.icon.extent.x0;
	drag.initial.y0 = oy + icon.icon.extent.y0;
	drag.initial.x1 = ox + icon.icon.extent.x1;
	drag.initial.y1 = oy + icon.icon.extent.y1;

	drag.bbox.x0 = 0x80000000;
	drag.bbox.y0 = 0x80000000;
	drag.bbox.x1 = 0x7fffffff;
	drag.bbox.y1 = 0x7fffffff;


	/* Read CMOS RAM to see if solid drags are required. */

	dragging_sprite = ((osbyte2 (osbyte_READ_CMOS, osbyte_CONFIGURE_DRAG_ASPRITE, 0) &
			osbyte_CONFIGURE_DRAG_ASPRITE_MASK) != 0);

	if (dragging_sprite)
		dragasprite_start (dragasprite_HPOS_CENTRE | dragasprite_VPOS_CENTRE |
			dragasprite_NO_BOUND | dragasprite_BOUND_POINTER | dragasprite_DROP_SHADOW,
			wimpspriteop_AREA, icon.icon.data.indirected_text.text, &(drag.initial), &(drag.bbox));
	else
		wimp_drag_box (&drag);

	drag_type = type;
	event_set_drag_handler(terminate_user_drag, NULL, NULL);
}


/**
 * Callback handler for queue window drag termination.
 *
 * Start a data-save dialogue with the application at the other end.
 *
 * \param  *drag		The Wimp poll block from termination.
 * \param  *data		NULL (unused).
 */

static void terminate_user_drag(wimp_dragged *drag, void *data)
{
	wimp_pointer		pointer;
	char			*leafname;

	extern global_windows	windows;

	if (dragging_sprite)
		dragasprite_stop ();

	wimp_get_pointer_info (&pointer);

	switch (drag_type) {
	case DRAG_SAVE_PDF:
		leafname = find_leafname (indirected_icon_text(windows.save_pdf, SAVE_PDF_ICON_NAME));
		send_start_data_save_function (pointer.w, pointer.i, pointer.pos, 0, drag_end_save_pdf, 0, PDF_FILE_TYPE, leafname);
		break;
	case DRAG_SAVE_SAVEAS:
		leafname = find_leafname (indirected_icon_text(windows.save_as, SAVEAS_ICON_NAME));
		send_start_data_save_function (pointer.w, pointer.i, pointer.pos, 0, drag_end_save_saveas, 0, PRINTPDF_FILE_TYPE, leafname);
		break;
	}
}



/**
 * Callback handler for DataSave completion on PDF save drags: start the
 * conversion using the filename returned.
 *
 * \param *filename		The filename returned by the DataSave protocol.
 * \return			0 if the save was started OK.
 */

static int drag_end_save_pdf(char *filename)
{
	extern global_windows		windows;

	conversion_dialogue_end(filename);
	wimp_close_window(windows.save_pdf);

	return 0;
}


/**
 * Try to save in response to a click on 'OK' in the Save dialogue.
 *
 * \return 		0 if the process completed OK.
 */

int immediate_window_save(void)
{
	char			*filename;
	extern global_windows	windows;

	filename = indirected_icon_text(windows.save_pdf, SAVE_PDF_ICON_NAME);

	/* Test if the filename is valid. */

	if (strchr (filename, '.') == NULL) {
		wimp_msgtrans_info_report("DragSave");
		return 1;
	}

	conversion_dialogue_end(filename);

	wimp_close_window(windows.save_pdf);

	return 0;
}


/**
 * Handle the receipt of a Message_DataSaveAck, usually in response to another
 * application trying to save a file to our icon.  Supply the location of the
 * queue head file, which allows postscript printer drivers to be set up by
 * dragging their save icon to our iconbar.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool message_data_save_reply(wimp_message *message)
{
	wimp_full_message_data_xfer	*datasave = (wimp_full_message_data_xfer *) message;
	os_error			*error;

	if (message->sender == main_task_handle) /* We don't want to respond to our own save requests. */
		return TRUE;


	if (datasave->w == wimp_ICON_BAR &&
			(datasave->file_type == PS_FILE_TYPE || datasave->file_type == PRINTPDF_FILE_TYPE)) {
		datasave->your_ref = datasave->my_ref;
		datasave->action = message_DATA_SAVE_ACK;

		switch (datasave->file_type) {
		case PS_FILE_TYPE:
			sprintf(datasave->file_name, "%s.printout/ps", config_str_read("FileQueue"));
			break;

		case PRINTPDF_FILE_TYPE:
			strcpy(datasave->file_name, "<Wimp$Scrap>");
			break;
		}

		datasave->size = WORDALIGN(45 + strlen(datasave->file_name));

		error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) datasave, datasave->sender);
		if (error != NULL)
			wimp_os_error_report(error, wimp_ERROR_BOX_CANCEL_ICON);
	}

	return TRUE;
}


/**
 * Handle the receipt of a Message_DataSaveAck.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show that the message was handled.
 */

static osbool message_data_save_ack_reply(wimp_message *message)
{
	send_reply_data_save_ack(message);

	return TRUE;
}


/**
 * Handle the receipt of a Message_DataLoad, generally as a result of a file
 * being dragged from the Filer to one of our windows or icons.
 *
 * \param *message		The associated Wimp message block.
 * \return			TRUE to show the message was handled.
 */

static osbool message_data_load_reply(wimp_message *message)
{
	wimp_full_message_data_xfer	*dataload = (wimp_full_message_data_xfer *) message;
	os_error			*error;
	char				queue_file[512];

	extern global_windows		windows;


	if (dataload->w == wimp_ICON_BAR &&
			(dataload->file_type == PS_FILE_TYPE || dataload->file_type == PRINTPDF_FILE_TYPE)) {
		switch (dataload->file_type) {
		case PS_FILE_TYPE:
			/* Before responding, test if the file being saved is the queue file.
			 * If it isn't, queue it and respond accordingly.
			 */

			sprintf(queue_file, "%s.printout/ps", config_str_read("FileQueue"));
			if (strcmp(queue_file, dataload->file_name) != 0)
				queue_ps_file(dataload->file_name);
			break;
		case PRINTPDF_FILE_TYPE:
			load_bookmark_file(dataload->file_name);
			break;
		}

		/* Reply with a Message_DataLoadAck. */

		dataload->your_ref = dataload->my_ref;
		dataload->action = message_DATA_LOAD_ACK;

		error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) dataload, dataload->sender);
		if (error != NULL)
			wimp_os_error_report(error, wimp_ERROR_BOX_CANCEL_ICON);
	} else if (dataload->w == windows.choices) {
		handle_choices_icon_drop(dataload);
	} else if (dataload->w == windows.save_pdf) {
		handle_save_icon_drop(dataload);
	}

	return TRUE;
}


/**
 * Handle the receipt of a Message_DataOpen.
 *
 * Param:  *message		The associated Wimp message block.
 * Return:			FALSE if the message failed to handle; else TRUE.
 */

static osbool start_data_open_load(wimp_message *message)
{
	wimp_full_message_data_xfer	*xfer = (wimp_full_message_data_xfer *) message;
	os_error			*error;


	switch (xfer->file_type) {
	case PRINTPDF_FILE_TYPE:
		xfer->your_ref = xfer->my_ref;
		xfer->action = message_DATA_LOAD_ACK;

		error = xwimp_send_message(wimp_USER_MESSAGE, (wimp_message *) xfer, xfer->sender);
		if (error != NULL) {
			wimp_os_error_report(error, wimp_ERROR_BOX_CANCEL_ICON);
			return FALSE;
		}

		load_bookmark_file(xfer->file_name);
		break;
	}

	return TRUE;
}
