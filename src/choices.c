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
#include "ihelp.h"
#include "main.h"
#include "optimize.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "templates.h"
#include "version.h"


/* Global variables */

static encrypt_params		encryption;
static optimize_params		optimization;
static version_params		version;
static pdfmark_params		pdfmark;

static wimp_w			choices_window = NULL;

static wimp_menu		*popup_version;
static wimp_menu		*popup_optimize;


static void	choices_close_window(void);
static void	choices_set_window(void);
static void	choices_read_window(void);
static void	choices_redraw_window(void);

static void	choices_click_handler(wimp_pointer *pointer);
static osbool	choices_keypress_handler(wimp_key *key);
static void	choices_menu_prepare_handler(wimp_w w, wimp_menu *menu, wimp_pointer *pointer);
static void	choices_menu_selection_handler(wimp_w w, wimp_menu *menu, wimp_selection *selection);

static osbool	handle_choices_icon_drop(wimp_message *message);

static void	choices_process_encrypt_dialogue(void);
static void	choices_process_pdfmark_dialogue(void);
static void	choices_process_optimize_dialogue(void);


/**
 * Initialise the Choices module.
 */

void choices_initialise(void)
{
	popup_version = templates_get_menu(TEMPLATES_MENU_VERSION);
	popup_optimize = templates_get_menu(TEMPLATES_MENU_OPTIMIZATION);

	choices_window = templates_create_window("Choices");
	ihelp_add_window(choices_window, "Choices", NULL);

	event_add_window_mouse_event(choices_window, choices_click_handler);
	event_add_window_key_event(choices_window, choices_keypress_handler);
	event_add_window_menu_prepare(choices_window, choices_menu_prepare_handler);
	event_add_window_menu_selection(choices_window, choices_menu_selection_handler);

	event_add_window_icon_popup(choices_window, CHOICE_ICON_VERSION_MENU, popup_version, -1);
	event_add_window_icon_popup(choices_window, CHOICE_ICON_OPTIMIZE_MENU, popup_optimize, -1);
	event_add_message_handler(message_DATA_LOAD, EVENT_MESSAGE_INCOMING, handle_choices_icon_drop);
}


/**
 * Open the Choices window at the mouse pointer.
 *
 * \param *pointer		The details of the pointer to open the window at.
 */

void choices_open_window(wimp_pointer *pointer)
{
	choices_set_window();

	windows_open_centred_at_pointer(choices_window, pointer);

	icons_put_caret_at_end(choices_window, CHOICE_ICON_DEFFILE);
}


/**
 * Close thye choices window.
 */

static void choices_close_window(void)
{
	wimp_close_window(choices_window);
}


/**
 * Set the contents of the Choices window to reflect the current settings.
 */

static void choices_set_window(void)
{
	sprintf(icons_get_indirected_text_addr(choices_window, CHOICE_ICON_DEFFILE), "%s", config_str_read("FileName"));

	encrypt_initialise_settings(&encryption);
	optimize_initialise_settings(&optimization);
	version_initialise_settings(&version);
	pdfmark_initialise_settings(&pdfmark);

	icons_set_selected(choices_window, CHOICE_ICON_RESETEVERY, config_opt_read("ResetParams"));
	icons_set_selected(choices_window, CHOICE_ICON_IBAR, config_opt_read("IconBarIcon"));
	icons_set_selected(choices_window, CHOICE_ICON_POPUP, config_opt_read("PopUpAfter"));
	icons_set_selected(choices_window, CHOICE_ICON_PREPROCESS, config_opt_read("PreProcess"));

	sprintf(icons_get_indirected_text_addr (choices_window, CHOICE_ICON_MEMORY), "%d", config_int_read("TaskMemory"));

	version_fill_field(choices_window, CHOICE_ICON_VERSION, &version);
	optimize_fill_field(choices_window, CHOICE_ICON_OPTIMIZE, &optimization);
	encrypt_fill_field(choices_window, CHOICE_ICON_ENCRYPT, &encryption);
	pdfmark_fill_field(choices_window, CHOICE_ICON_INFO, &pdfmark);
}


/**
 * Update the configuration settings from the values in the Choices window.
 */

static void choices_read_window(void)
{
	/* Read the main window. */

	config_str_set("FileName", icons_get_indirected_text_addr(choices_window, CHOICE_ICON_DEFFILE));

	config_opt_set("ResetParams", icons_get_selected(choices_window, CHOICE_ICON_RESETEVERY));
	config_opt_set("IconBarIcon", icons_get_selected(choices_window, CHOICE_ICON_IBAR));
	config_opt_set("PopUpAfter", icons_get_selected(choices_window, CHOICE_ICON_POPUP));
	config_opt_set("PreProcess", icons_get_selected(choices_window, CHOICE_ICON_PREPROCESS));

	config_int_set("TaskMemory", atoi(icons_get_indirected_text_addr(choices_window, CHOICE_ICON_MEMORY)));

	version_save_settings(&version);
	encrypt_save_settings(&encryption);
	optimise_save_settings(&optimization);
	pdfmark_save_settings(&pdfmark);

	/* Make any immediate changes that depend on the choices. */

	iconbar_set_icon(config_opt_read("IconBarIcon"));
}


/**
 * Refresh the Choices dialogue, to reflech changed icon states.
 */

static void choices_redraw_window(void)
{
	wimp_set_icon_state(choices_window, CHOICE_ICON_DEFFILE, 0, 0);
	wimp_set_icon_state(choices_window, CHOICE_ICON_VERSION, 0, 0);
	wimp_set_icon_state(choices_window, CHOICE_ICON_OPTIMIZE, 0, 0);
	wimp_set_icon_state(choices_window, CHOICE_ICON_MEMORY, 0, 0);

	icons_replace_caret_in_window(choices_window);
}


/**
 * Process mouse clicks in the Choices dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void choices_click_handler(wimp_pointer *pointer)
{
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

	case CHOICE_ICON_ENCRYPT_MENU:
		encrypt_set_dialogue_callback(choices_process_encrypt_dialogue);
		encrypt_open_dialogue(&encryption, version.standard_version >= 2, pointer);
		break;

	case CHOICE_ICON_INFO_MENU:
		pdfmark_set_dialogue_callback(choices_process_pdfmark_dialogue);
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
 * Process menu prepare events in the Choices window.
 *
 * \param w		The handle of the owning window.
 * \param *menu		The menu handle.
 * \param *pointer	The pointer position, or NULL for a re-open.
 */

static void choices_menu_prepare_handler(wimp_w w, wimp_menu *menu, wimp_pointer *pointer)
{
	if (menu == popup_version)
		version_set_menu(&version, popup_version);
	else if (menu == popup_optimize)
		optimize_set_menu(&optimization, popup_optimize);
}


/**
 * Process menu selection events in the Choices window.
 *
 * \param w		The handle of the owning window.
 * \param *menu		The menu handle.
 * \param *selection	The menu selection details.
 */

static void choices_menu_selection_handler(wimp_w w, wimp_menu *menu, wimp_selection *selection)
{
	if (menu == popup_version) {
		version_process_menu(&version, popup_version, selection);
		version_fill_field(w, CHOICE_ICON_VERSION, &version);
	} else if (menu == popup_optimize) {
		optimize_set_dialogue_callback(choices_process_optimize_dialogue);
		optimize_process_menu(&optimization, popup_version, selection);
		optimize_fill_field(w, CHOICE_ICON_OPTIMIZE, &optimization);
	}
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

	/* If it isn't our window, don't claim the message as someone else
	 * might want it.
	 */

	if (datasave == NULL || datasave->w != choices_window)
		return FALSE;

	/* If it is our window, but not the icon we care about, claim
	 * the message.
	 */

	if (datasave->i != CHOICE_ICON_DEFFILE)
		return TRUE;

	/* It's our window and the correct icon, so process the filename. */

	strcpy(path, datasave->file_name);

	extension = string_find_extension(path);
	leaf = string_strip_extension(path);
	string_find_pathname(path);

	sprintf(icons_get_indirected_text_addr(choices_window, CHOICE_ICON_DEFFILE), "%s.%s/pdf", path, leaf);

	icons_replace_caret_in_window(datasave->w);
	wimp_set_icon_state(datasave->w, datasave->i, 0, 0);

	return TRUE;
}


/**
 * Callback to respond to clicks on the OK button of the encryption
 * dialogue.
 */

static void choices_process_encrypt_dialogue(void)
{
	encrypt_process_dialogue(&encryption);
	encrypt_fill_field(choices_window, CHOICE_ICON_ENCRYPT, &encryption);
}


/**
 * Callback to respond to clicks on the OK button of the PDFMark
 * dialogue.
 */

static void choices_process_pdfmark_dialogue(void)
{
	pdfmark_process_dialogue(&pdfmark);
	pdfmark_fill_field(choices_window, CHOICE_ICON_INFO, &pdfmark);
}


/**
 * Callback to respond to clicks on the OK button of the optimization
 * dialogue.
 */

static void choices_process_optimize_dialogue(void)
{
	optimize_process_dialogue(&optimization);
	optimize_fill_field(choices_window, CHOICE_ICON_OPTIMIZE, &optimization);
}


/**
 * Identify whether the Choices window is currently open.
 *
 * \return		TRUE if the window is open; else FALSE.
 */

osbool choices_window_is_open(void)
{
	return windows_get_open(choices_window);
}

