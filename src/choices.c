/* PrintPDF - choices.c
 * (c) Stephen Fryatt, 2007-2011
 */

/* ANSI C Header files. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Acorn C Header files. */


/* OSLib Header files. */

#include "oslib/wimp.h"

/* SF-Lib Header files. */

#include "sflib/config.h"
#include "sflib/event.h"
#include "sflib/icons.h"
#include "sflib/windows.h"
#include "sflib/debug.h"
#include "sflib/string.h"

/* Application header files. */

#include "choices.h"

#include "encrypt.h"
#include "iconbar.h"
#include "main.h"
#include "optimize.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "version.h"
#include "windows.h"


/* Global variables */

static encrypt_params		encryption;
static optimize_params		optimization;
static version_params		version;
static pdfmark_params		pdfmark;


static void	choices_close_window(void);
static void	choices_set_window(void);
static void	choices_read_window(void);
static void	choices_redraw_window(void);

static void	choices_click_handler(wimp_pointer *pointer);
static osbool	choices_keypress_handler(wimp_key *key);

static osbool	handle_choices_icon_drop(wimp_message *message);

static void	process_choices_encrypt_dialogue(void);
static void	process_choices_pdfmark_dialogue(void);
static void	process_choices_optimize_dialogue(void);


/**
 * Initialise the Choices module.
 */

void choices_initialise(void)
{
	extern global_windows		windows;

	event_add_window_mouse_event(windows.choices, choices_click_handler);
	event_add_window_key_event(windows.choices, choices_keypress_handler);
	event_add_message_handler(message_DATA_LOAD, EVENT_MESSAGE_INCOMING, handle_choices_icon_drop);
}


/**
 * Open the Choices window at the mouse pointer.
 *
 * \param *pointer		The details of the pointer to open the window at.
 */

void choices_open_window(wimp_pointer *pointer)
{
	extern global_windows		windows;

	choices_set_window();

	open_window_centred_at_pointer(windows.choices, pointer);

	put_caret_at_end(windows.choices, CHOICE_ICON_DEFFILE);
}


/**
 * Close thye choices window.
 */

static void choices_close_window(void)
{
	extern global_windows		windows;

	wimp_close_window(windows.choices);
}


/**
 * Set the contents of the Choices window to reflect the current settings.
 */

static void choices_set_window(void)
{
	extern global_windows		windows;

	/* Set the main window up. */

	sprintf(indirected_icon_text(windows.choices, CHOICE_ICON_DEFFILE), "%s", config_str_read("FileName"));

	encrypt_initialise_settings(&encryption);
	optimize_initialise_settings(&optimization);
	version_initialise_settings(&version);
	pdfmark_initialise_settings(&pdfmark);

	set_icon_selected(windows.choices, CHOICE_ICON_RESETEVERY, config_opt_read("ResetParams"));
	set_icon_selected(windows.choices, CHOICE_ICON_IBAR, config_opt_read("IconBarIcon"));
	set_icon_selected(windows.choices, CHOICE_ICON_POPUP, config_opt_read("PopUpAfter"));
	set_icon_selected(windows.choices, CHOICE_ICON_PREPROCESS, config_opt_read("PreProcess"));

	sprintf(indirected_icon_text (windows.choices, CHOICE_ICON_MEMORY), "%d", config_int_read("TaskMemory"));

	version_fill_field(windows.choices, CHOICE_ICON_VERSION, &version);
	optimize_fill_field(windows.choices, CHOICE_ICON_OPTIMIZE, &optimization);
	encrypt_fill_field(windows.choices, CHOICE_ICON_ENCRYPT, &encryption);
	pdfmark_fill_field(windows.choices, CHOICE_ICON_INFO, &pdfmark);
}


/**
 * Update the configuration settings from the values in the Choices window.
 */

static void choices_read_window(void)
{
	extern global_windows		windows;

	/* Read the main window. */

	config_str_set("FileName", indirected_icon_text(windows.choices, CHOICE_ICON_DEFFILE));

	config_opt_set("ResetParams", read_icon_selected(windows.choices, CHOICE_ICON_RESETEVERY));
	config_opt_set("IconBarIcon", read_icon_selected(windows.choices, CHOICE_ICON_IBAR));
	config_opt_set("PopUpAfter", read_icon_selected(windows.choices, CHOICE_ICON_POPUP));
	config_opt_set("PreProcess", read_icon_selected(windows.choices, CHOICE_ICON_PREPROCESS));

	config_int_set("TaskMemory", atoi(indirected_icon_text(windows.choices, CHOICE_ICON_MEMORY)));

	version_save_settings(&version);
	encrypt_save_settings(&encryption);
	optimise_save_settings(&optimization);
	pdfmark_save_settings(&pdfmark);

	/* Make any immediate changes that depend on the choices. */

	set_iconbar_icon(config_opt_read("IconBarIcon"));
}


/**
 * Refresh the Choices dialogue, to reflech changed icon states.
 */

static void choices_redraw_window(void)
{
	extern global_windows		windows;

	wimp_set_icon_state(windows.choices, CHOICE_ICON_DEFFILE, 0, 0);
	wimp_set_icon_state(windows.choices, CHOICE_ICON_VERSION, 0, 0);
	wimp_set_icon_state(windows.choices, CHOICE_ICON_OPTIMIZE, 0, 0);
	wimp_set_icon_state(windows.choices, CHOICE_ICON_MEMORY, 0, 0);

	replace_caret_in_window(windows.choices);
}


/**
 * Process mouse clicks in the Choices dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void choices_click_handler(wimp_pointer *pointer)
{
	extern global_windows		windows;

	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case CHOICE_ICON_APPLY:
		if (pointer->buttons == wimp_CLICK_SELECT || pointer->buttons == wimp_CLICK_ADJUST) {
			choices_read_window();

			if (pointer->buttons == wimp_CLICK_SELECT)
				choices_close_window();
		}
		break;

	case CHOICE_ICON_SAVE:
		if (pointer->buttons == wimp_CLICK_SELECT || pointer->buttons == wimp_CLICK_ADJUST) {
			choices_read_window();
			config_save();

			if (pointer->buttons == wimp_CLICK_SELECT)
				choices_close_window();
		}
		break;

	case CHOICE_ICON_CANCEL:
		if (pointer->buttons == wimp_CLICK_SELECT) {
			choices_close_window();
		} else if (pointer->buttons == wimp_CLICK_ADJUST) {
			choices_set_window();
			choices_redraw_window();
		}
		break;

	case CHOICE_ICON_VERSION_MENU:
		version_open_menu(&version, pointer, windows.choices, CHOICE_ICON_VERSION, PARAM_MENU_CHOICES_VERSION);
		break;

	case CHOICE_ICON_OPTIMIZE_MENU:
		optimize_set_dialogue_callback(process_choices_optimize_dialogue);
		optimize_open_menu(&optimization, pointer, windows.choices, CHOICE_ICON_OPTIMIZE, PARAM_MENU_CHOICES_OPTIMIZE);
		break;

	case CHOICE_ICON_ENCRYPT_MENU:
		encrypt_set_dialogue_callback(process_choices_encrypt_dialogue);
		encrypt_open_dialogue(&encryption, version.standard_version >= 2, pointer);
		break;

	case CHOICE_ICON_INFO_MENU:
		pdfmark_set_dialogue_callback(process_choices_pdfmark_dialogue);
		pdfmark_open_dialogue(&pdfmark, pointer);
		break;
	}
}


/**
 * Process keypresses in the Choices window.
 *
 * \param *key		The keypress event block to handle.
 * \return		TRUE if the event was handled; else FALSE.
 */

static osbool choices_keypress_handler(wimp_key *key)
{
	if (key == NULL)
		return FALSE;

	switch (key->c) {
	case wimp_KEY_RETURN:
		choices_read_window();
		config_save();
		choices_close_window();
		break;

	case wimp_KEY_ESCAPE:
		choices_close_window();
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}


/**
 * Check incoming Message_DataSave to see if it's a file being dropped into the
 * the PDF filename icon.
 *
 * \param *message		The incoming message block.
 * \return			TRUE if we claim the message as intended for us; else FALSE.
 */

static osbool handle_choices_icon_drop(wimp_message *message)
{
	wimp_full_message_data_xfer	*datasave = (wimp_full_message_data_xfer *) message;

	char				*extension, *leaf, path[256];
	extern global_windows		windows;

	/* If it isn't our window, don't claim the message as someone else
	 * might want it.
	 */

	if (datasave == NULL || datasave->w != windows.choices)
		return FALSE;

	/* If it is our window, but not the icon we care about, claim
	 * the message.
	 */

	if (datasave->i != CHOICE_ICON_DEFFILE)
		return TRUE;

	/* It's our window and the correct icon, so process the filename. */

	strcpy(path, datasave->file_name);

	extension = find_extension(path);
	leaf = lose_extension(path);
	find_pathname(path);

	sprintf(indirected_icon_text(windows.choices, CHOICE_ICON_DEFFILE), "%s.%s/pdf", path, leaf);

	replace_caret_in_window(datasave->w);
	wimp_set_icon_state(datasave->w, datasave->i, 0, 0);

	return TRUE;
}

/* ==================================================================================================================
 * Handle pop-up menus from the dialogue.
 */


/* ------------------------------------------------------------------------------------------------------------------ */

void process_choices_version_menu (wimp_selection *selection)
{
  extern global_windows windows;

  version_process_menu (&version, selection);

  version_fill_field (windows.choices, CHOICE_ICON_VERSION, &version);
}


/* ------------------------------------------------------------------------------------------------------------------ */

void process_choices_optimize_menu (wimp_selection *selection)
{
  extern global_windows windows;

  optimize_process_menu(&optimization, selection);

  optimize_fill_field (windows.choices, CHOICE_ICON_OPTIMIZE, &optimization);
}



/**
 * Callback to respond to clicks on the OK button of the encryption
 * dialogue.
 */

static void process_choices_encrypt_dialogue(void)
{
	extern global_windows		windows;

	encrypt_process_dialogue(&encryption);
	encrypt_fill_field(windows.choices, CHOICE_ICON_ENCRYPT, &encryption);
}


/**
 * Callback to respond to clicks on the OK button of the PDFMark
 * dialogue.
 */

static void process_choices_pdfmark_dialogue(void)
{
	extern global_windows		windows;

	pdfmark_process_dialogue(&pdfmark);
	pdfmark_fill_field(windows.choices, CHOICE_ICON_INFO, &pdfmark);
}


/**
 * Callback to respond to clicks on the OK button of the optimization
 * dialogue.
 */

static void process_choices_optimize_dialogue(void)
{
	extern global_windows		windows;

	optimize_process_dialogue(&optimization);
	optimize_fill_field(windows.choices, CHOICE_ICON_OPTIMIZE, &optimization);
}

