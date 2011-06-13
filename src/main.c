/* PrintPDF - main.c
 *
 * (c) Stephen Fryatt, 2007-2011
 */

/* ANSI C header files */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "flex.h"

/* OSLib header files */

#include "oslib/wimp.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"
#include "oslib/uri.h"
#include "oslib/hourglass.h"
#include "oslib/pdriver.h"
#include "oslib/help.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/resources.h"
#include "sflib/heap.h"
#include "sflib/windows.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/transfer.h"
#include "sflib/url.h"
#include "sflib/msgs.h"
#include "sflib/debug.h"
#include "sflib/config.h"
#include "sflib/errors.h"
#include "sflib/string.h"
#include "sflib/colpick.h"
#include "sflib/event.h"

/* Application header files */

#include "main.h"

#include "bookmark.h"
#include "choices.h"
#include "convert.h"
#include "dataxfer.h"
#include "encrypt.h"
#include "iconbar.h"
#include "ihelp.h"
#include "optimize.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "popup.h"
#include "taskman.h"
#include "templates.h"
#include "version.h"

/* ------------------------------------------------------------------------------------------------------------------ */

static void	main_poll_loop(void);
static void	main_initialise(void);
static void	main_parse_command_line(int argc, char *argv[]);
static osbool	main_message_quit(wimp_message *message);
static osbool	main_message_prequit(wimp_message *message);


/*
 * Cross file global variables
 */

wimp_t			main_task_handle;
int			main_quit_flag = FALSE;
osspriteop_area		*main_wimp_sprites;


/**
 * Main code entry point.
 */

int main(int argc, char *argv[])
{
	main_initialise();

	main_parse_command_line(argc, argv);

	main_poll_loop();

	terminate_bookmarks();
	msgs_terminate();
	wimp_close_down(main_task_handle);
	convert_remove_all_remaining_conversions();

	return 0;
}


/**
 * Wimp Poll loop.
 */

static void main_poll_loop(void)
{
	os_t			poll_time;
	wimp_event_no		reason;
	wimp_block		blk;


	poll_time = os_read_monotonic_time();

	while (!main_quit_flag) {
		reason = wimp_poll_idle(0, &blk, poll_time, 0);

		/* Events are passed to Event Lib first; only if this fails
		 * to handle them do they get passed on to the internal
		 * inline handlers shown here.
		 */

		if (!event_process_event(reason, &blk, 0)) {
			switch (reason) {
			case wimp_NULL_REASON_CODE:
				popup_test_and_close(poll_time);
				convert_check_for_ps_file();
				convert_check_for_pending_files();
				poll_time += config_int_read("PollDelay");
				break;

			case wimp_OPEN_WINDOW_REQUEST:
				wimp_open_window(&(blk.open));
				break;

			case wimp_CLOSE_WINDOW_REQUEST:
				wimp_close_window(blk.close.w);
				break;
			}
		}
	}
}


/**
 * Application initialisation.
 */

static void main_initialise(void)
{
	static char		task_name[255];
	char			resources[255], res_temp[255], filename[256];
	osspriteop_area		*sprites;

	wimp_MESSAGE_LIST(13)	message_list;
	wimp_version_no		wimp_version;


	hourglass_on();

	strcpy(resources, "<PrintPDF$Dir>.Resources");
	find_resource_path(resources, sizeof (resources));

	/* Load the messages file. */

	snprintf(res_temp, sizeof(res_temp), "%s.Messages", resources);
	msgs_initialise(res_temp);

	/* Initialise the error message system. */

	error_initialise("TaskName", "TaskSpr", NULL);

	/* Initialise with the Wimp. */

	message_list.messages[0]=message_URI_RETURN_RESULT;
	message_list.messages[1]=message_ANT_OPEN_URL;
	message_list.messages[2]=message_DATA_SAVE;
	message_list.messages[3]=message_DATA_SAVE_ACK;
	message_list.messages[4]=message_DATA_LOAD;
	message_list.messages[5]=message_DATA_OPEN;
	message_list.messages[6]=message_MENU_WARNING;
	message_list.messages[7]=message_MENUS_DELETED;
	message_list.messages[8]=message_HELP_REQUEST;
	message_list.messages[9]=message_TASK_INITIALISE;
	message_list.messages[10]=message_TASK_CLOSE_DOWN;
	message_list.messages[11]=message_PRE_QUIT;
	message_list.messages[12]=message_QUIT;

	msgs_lookup("TaskName", task_name, sizeof (task_name));
	main_task_handle = wimp_initialise(wimp_VERSION_RO3, task_name, (wimp_message_list *) &message_list, &wimp_version);

	/* Test to see if any other copies of PrintPDF are running, and set to quit if they are. */

	if (taskman_task_is_running(task_name, main_task_handle))
		main_quit_flag = TRUE;

	event_add_message_handler(message_QUIT, EVENT_MESSAGE_INCOMING, main_message_quit);
	event_add_message_handler(message_PRE_QUIT, EVENT_MESSAGE_INCOMING, main_message_prequit);

	/* Initialise the configuration. */

	config_initialise(task_name, "PrintPDF", "<PrintPDF$Dir>");

	config_str_init("FileQueue", "<Wimp$ScrapDir>.PrintPDF");
	config_str_init("ParamFile", "Pipe:$.PrintPDF");
	config_str_init("PDFMarkFile", "Pipe:$.PrintPDFMark");
	config_str_init("FileName", msgs_lookup ("FileName", filename, sizeof (filename)));
	config_int_init("PollDelay", 500);
	config_int_init("PopUpTime", 200);
	config_int_init("TaskMemory", 8192);
	config_int_init("PDFVersion", 0);
	config_int_init("Optimization", 0);
	config_opt_init("DownsampleMono", FALSE);
	config_int_init("DownsampleMonoType", 0);
	config_int_init("DownsampleMonoResolution", 300);
	config_int_init("DownsampleMonoThreshold", 15);
	config_int_init("DownsampleMonoDepth", -1);
	config_opt_init("DownsampleGrey", FALSE);
	config_int_init("DownsampleGreyType", 0);
	config_int_init("DownsampleGreyResolution", 72);
	config_int_init("DownsampleGreyThreshold", 15);
	config_int_init("DownsampleGreyDepth", -1);
	config_opt_init("DownsampleColour", FALSE);
	config_int_init("DownsampleColourType", 0);
	config_int_init("DownsampleColourResolution", 72);
	config_int_init("DownsampleColourThreshold", 15);
	config_int_init("DownsampleColourDepth", -1);
	config_opt_init("EncodeMono", TRUE);
	config_int_init("EncodeMonoType", 2);
	config_opt_init("EncodeGrey", TRUE);
	config_int_init("EncodeGreyType", 0);
	config_opt_init("EncodeColour", TRUE);
	config_int_init("EncodeColourType", 0);
	config_int_init("AutoPageRotation", 2);
	config_opt_init("CompressPages", TRUE);
	config_str_init("OwnerPasswd", "");
	config_str_init("UserPasswd", "");
	config_opt_init("AllowPrint", TRUE);
	config_opt_init("AllowFullPrint", TRUE);
	config_opt_init("AllowExtraction", TRUE);
	config_opt_init("AllowFullExtraction", TRUE);
	config_opt_init("AllowForms", TRUE);
	config_opt_init("AllowAnnotation", TRUE);
	config_opt_init("AllowModifications", TRUE);
	config_opt_init("AllowAssembly", TRUE);
	config_str_init("PDFMarkTitle", "");
	config_str_init("PDFMarkAuthor", "");
	config_str_init("PDFMarkSubject", "");
	config_str_init("PDFMarkKeywords", "");
	config_str_init("PDFMarkUserFile", "");
	config_opt_init("PreProcess", FALSE);
	config_opt_init("ResetParams", FALSE);
	config_opt_init("IconBarIcon", TRUE);
	config_opt_init("PopUpAfter", TRUE);

	config_load();

	/* Load the menu structure. */

	snprintf(res_temp, sizeof(res_temp), "%s.Menus", resources);
	templates_load_menus(res_temp);

	/* Load the window templates. */

	sprites = load_user_sprite_area("<PrintPDF$Dir>.Sprites");

	main_wimp_sprites = sprites;

	snprintf(res_temp, sizeof(res_temp), "%s.Templates", resources);
	templates_open(res_temp);

	/* Initialise the individual modules. */

	ihelp_initialise();
	taskman_initialise();
	popup_initialise();
	dataxfer_initialise();
	choices_initialise();
	encrypt_initialise();
	optimize_initialise();
	pdfmark_initialise();
	iconbar_initialise();
	convert_initialise();
	initialise_bookmarks();
	url_initialise();

	templates_close();

	/* Create an icon-bar icon. */

	iconbar_set_icon(config_opt_read("IconBarIcon") && (main_quit_flag == FALSE));

	hourglass_off();
}


/**
 * Take the command line and parse it for useful arguments.
 */

static void main_parse_command_line(int argc, char *argv[])
{
	int	i;

	if (argc > 1) {
		for (i=1; i<argc; i++) {
			if (strcmp (argv[i], "-file") == 0 && i+1 < argc)
				load_bookmark_file(argv[i+1]);
		}
	}
}


/**
 * Handle incoming Message_Quit.
 */

static osbool main_message_quit(wimp_message *message)
{
	main_quit_flag = TRUE;

	return TRUE;
}


/**
 * Handle incoming Message_PreQuit.
 */

static osbool main_message_prequit(wimp_message *message)
{
	if (!bookmark_files_unsaved() && !convert_pending_files_in_queue())
		return TRUE;

	message->your_ref = message->my_ref;
	wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message, message->sender);

	return TRUE;
}

