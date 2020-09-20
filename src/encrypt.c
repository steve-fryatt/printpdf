/* Copyright 2005-2017, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: encrypt.c
 *
 * Encryption dialogue implementation.
 */

/* ANSI C header files */

#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/event.h"
#include "sflib/icons.h"
#include "sflib/ihelp.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "encrypt.h"

#include "pmenu.h"


/* Encryption Window icons. */

#define ENCRYPT_ICON_CANCEL 0
#define ENCRYPT_ICON_OK 1
#define ENCRYPT_ICON_OWNER_PW 3
#define ENCRYPT_ICON_ACCESS_PW 5
#define ENCRYPT_ICON_PRINT 8
#define ENCRYPT_ICON_MODS 9
#define ENCRYPT_ICON_EXTRACT 10
#define ENCRYPT_ICON_FORMS 11
#define ENCRYPT_ICON_FULL_PRINT 12
#define ENCRYPT_ICON_FULL_EXTRACT 13
#define ENCRYPT_ICON_ANNOTATE 14
#define ENCRYPT_ICON_ASSEMBLY 15

#define ENCRYPT_ICON_SHADE_BASE 4
#define ENCRYPT_ICON_SHADE_MAX2 11
#define ENCRYPT_ICON_SHADE_MAX3 15

/* Access permission bitmasks */

#define ACCESS_REV2_BASE		0xffffffc0
#define ACCESS_REV2_PRINT		0x004
#define ACCESS_REV2_MODIFY		0x008
#define ACCESS_REV2_COPY		0x010
#define ACCESS_REV2_ANNOTATE		0x020

#define ACCESS_REV3_BASE		0xfffff0c0
#define ACCESS_REV3_PRINT		0x004
#define ACCESS_REV3_MODIFY		0x008
#define ACCESS_REV3_COPYALL		0x010
#define ACCESS_REV3_ANNOTATE		0x020
#define ACCESS_REV3_FORMS		0x100
#define ACCESS_REV3_COPYACCESS		0x200
#define ACCESS_REV3_ASSEMBLE		0x400
#define ACCESS_REV3_PRINTFULL		0x800


static wimp_w	encrypt_window2 = NULL;
static wimp_w	encrypt_window3 = NULL;

static void	(*encrypt_dialogue_close_callback)(void) = NULL;


static void		encrypt_click_handler(wimp_pointer *pointer);
static osbool		encrypt_keypress_handler(wimp_key *key);
static void		encrypt_shade_dialogue(wimp_w window);


/**
 * Initialise the encryption dialogue.
 */

void encrypt_initialise(void)
{
	encrypt_window2 = templates_create_window("Security2");
	ihelp_add_window(encrypt_window2, "Security2", NULL);
	event_add_window_mouse_event(encrypt_window2, encrypt_click_handler);
	event_add_window_key_event(encrypt_window2, encrypt_keypress_handler);

	encrypt_window3 = templates_create_window("Security3");
	ihelp_add_window(encrypt_window3, "Security3", NULL);
	event_add_window_mouse_event(encrypt_window3, encrypt_click_handler);
	event_add_window_key_event(encrypt_window3, encrypt_keypress_handler);
}


/**
 * Initialise the values in an encryption settings structure.
 *
 * \param *params		The encryption params struct to be initialised.
 */

void encrypt_initialise_settings(encrypt_params *params)
{
	string_copy(params->owner_password, config_str_read("OwnerPasswd"), MAX_PASSWORD);
	string_copy(params->access_password, config_str_read("UserPasswd"), MAX_PASSWORD);

	params->allow_print = config_opt_read("AllowPrint");
	params->allow_full_print = config_opt_read("AllowFullPrint");
	params->allow_extraction = config_opt_read("AllowExtraction");
	params->allow_full_extraction = config_opt_read("AllowFullExtraction");
	params->allow_forms = config_opt_read("AllowForms");
	params->allow_annotation = config_opt_read("AllowAnnotation");
	params->allow_modifications = config_opt_read("AllowModifications");
	params->allow_assembly = config_opt_read("AllowAssembly");
}


/**
 * Save the settings from an encryption settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The encryption params struct to be saved.
 */

void encrypt_save_settings(encrypt_params *params)
{
	config_str_set("OwnerPasswd", params->owner_password);
	config_str_set("UserPasswd", params->access_password);

	config_opt_set("AllowPrint", params->allow_print);
	config_opt_set("AllowFullPrint", params->allow_full_print);
	config_opt_set("AllowExtraction", params->allow_extraction);
	config_opt_set("AllowFullExtraction", params->allow_full_extraction);
	config_opt_set("AllowForms", params->allow_forms);
	config_opt_set("AllowAnnotation", params->allow_annotation);
	config_opt_set("AllowModifications", params->allow_modifications);
	config_opt_set("AllowAssembly", params->allow_assembly);
}


/**
 * Set a callback handler to be called when the OK button of the
 * encryption dialogue is clicked.
 *
 * \param callback		The callback function to use, or NULL.
 */

void encrypt_set_dialogue_callback(void (*callback)(void))
{
	encrypt_dialogue_close_callback = callback;
}


/**
 * Process mouse clicks in the encryption dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void encrypt_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case ENCRYPT_ICON_CANCEL:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;

	case ENCRYPT_ICON_OK:
		if (encrypt_dialogue_close_callback != NULL)
			encrypt_dialogue_close_callback();
		break;
	}
}


/**
 * Process keypresses in the encryption window.
 *
 * \param *key		The keypress event block to handle.
 * \return		TRUE if the event was handled; else FALSE.
 */

static osbool encrypt_keypress_handler(wimp_key *key)
{
	if (key == NULL)
		return FALSE;

	if (key->i == ENCRYPT_ICON_OWNER_PW)
		encrypt_shade_dialogue(NULL);

	switch (key->c) {
	case wimp_KEY_RETURN:
		if (encrypt_dialogue_close_callback != NULL)
			encrypt_dialogue_close_callback();
		break;

	case wimp_KEY_ESCAPE:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}


/**
 * Open the encryption dialogue for the given parameter block.
 *
 * \param *params		The encryption parameter block to be used.
 * \param *extended_opts	TRUE if the extended options should be offered.
 * \param *pointer		The current pointer state.
 */

void encrypt_open_dialogue(encrypt_params *params, osbool extended_opts, wimp_pointer *pointer)
{
	wimp_w				encrypt_win;

	encrypt_win = (extended_opts) ? encrypt_window3 : encrypt_window2;

	icons_strncpy(encrypt_win, ENCRYPT_ICON_OWNER_PW, params->owner_password);
	icons_strncpy(encrypt_win, ENCRYPT_ICON_ACCESS_PW, params->access_password);

	icons_set_selected(encrypt_win, ENCRYPT_ICON_PRINT, params->allow_print);
	icons_set_selected(encrypt_win, ENCRYPT_ICON_EXTRACT, params->allow_extraction);
	icons_set_selected(encrypt_win, ENCRYPT_ICON_FORMS, params->allow_forms);
	icons_set_selected(encrypt_win, ENCRYPT_ICON_MODS, params->allow_modifications);

	if (extended_opts) {
		icons_set_selected(encrypt_win, ENCRYPT_ICON_FULL_PRINT, params->allow_full_print);
		icons_set_selected(encrypt_win, ENCRYPT_ICON_FULL_EXTRACT, params->allow_full_extraction);
		icons_set_selected(encrypt_win, ENCRYPT_ICON_ANNOTATE, params->allow_annotation);
		icons_set_selected(encrypt_win, ENCRYPT_ICON_ASSEMBLY, params->allow_assembly);
	}

	encrypt_shade_dialogue(encrypt_win);

	menus_create_standard_menu((wimp_menu *) encrypt_win, pointer);
}


/**
 * Store the settings from the currently open encryption dialogue box in
 * an encryption parameter block.
 *
 * \param *params		The encryption parameter block to be used.
 */

void encrypt_process_dialogue(encrypt_params *params)
{
	osbool				extended_opts;
	wimp_w				encrypt_win;


	if (windows_get_open(encrypt_window2)) {
		encrypt_win = encrypt_window2;
		extended_opts = FALSE;
	} else {
		encrypt_win = encrypt_window3;
		extended_opts = TRUE;
	}

	icons_copy_text(encrypt_win, ENCRYPT_ICON_OWNER_PW, params->owner_password, MAX_PASSWORD);
	icons_copy_text(encrypt_win, ENCRYPT_ICON_ACCESS_PW, params->access_password, MAX_PASSWORD);

	params->allow_print = icons_get_selected(encrypt_win, ENCRYPT_ICON_PRINT);
	params->allow_extraction = icons_get_selected(encrypt_win, ENCRYPT_ICON_EXTRACT);
	params->allow_forms = icons_get_selected(encrypt_win, ENCRYPT_ICON_FORMS);
	params->allow_modifications = icons_get_selected(encrypt_win, ENCRYPT_ICON_MODS);

	if (extended_opts) {
		params->allow_full_print = icons_get_selected(encrypt_win, ENCRYPT_ICON_FULL_PRINT);
		params->allow_full_extraction = icons_get_selected(encrypt_win, ENCRYPT_ICON_FULL_EXTRACT);
		params->allow_annotation = icons_get_selected(encrypt_win, ENCRYPT_ICON_ANNOTATE);
		params->allow_assembly = icons_get_selected(encrypt_win, ENCRYPT_ICON_ASSEMBLY);
	}

	wimp_create_menu((wimp_menu *) -1, 0, 0);
}


/**
 * Update the shading in the encryption dialogue, based on the current icon
 * selections
 *
 * \param window		The required window handle, or NULL for the open one.
 */

static void encrypt_shade_dialogue(wimp_w window)
{
	static osbool		shaded = FALSE;
	osbool			changed, new_state;
	int			max_icon, icon;
	wimp_w			encrypt_win;

	changed = FALSE;

	if (window != NULL) {
		encrypt_win = window;

		shaded = (strlen(icons_get_indirected_text_addr(encrypt_win, ENCRYPT_ICON_OWNER_PW)) == 0);
		max_icon = (encrypt_win == encrypt_window3) ? ENCRYPT_ICON_ASSEMBLY : ENCRYPT_ICON_FORMS;

		changed = TRUE;
	} else {
		if (windows_get_open(encrypt_window2)) {
			encrypt_win = encrypt_window2;
			new_state = (strlen(icons_get_indirected_text_addr(encrypt_win, ENCRYPT_ICON_OWNER_PW)) == 0);
			max_icon = ENCRYPT_ICON_SHADE_MAX2;

			if (new_state != shaded) {
				shaded = new_state;
				changed = TRUE;
			}
		} else if (windows_get_open(encrypt_window3)) {
			encrypt_win = encrypt_window3;
			new_state = (strlen(icons_get_indirected_text_addr(encrypt_win, ENCRYPT_ICON_OWNER_PW)) == 0);
			max_icon = ENCRYPT_ICON_SHADE_MAX3;

			if (new_state != shaded) {
				shaded = new_state;
				changed = TRUE;
			}
		}
	}

	if (changed) {
		for (icon = ENCRYPT_ICON_SHADE_BASE; icon <= max_icon; icon++)
			icons_set_shaded(encrypt_win, icon, shaded);
	}
}


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given encryption parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The encryption parameter block to use.
 */

void encrypt_fill_field(wimp_w window, wimp_i icon, encrypt_params *params)
{
	char	*token;

	token = (strlen(params->owner_password) == 0) ? "Encrypt0" : "Encrypt1";

	icons_msgs_lookup(window, icon, token);
	wimp_set_icon_state(window, icon, 0, 0);
}


/**
 * Build up a text string in the supplied buffer containing the GS
 * parameters that reflect the contents of the given encryption
 * parameter block.
 *
 * \param *buffer		Buffer to hold the result.
 * \param len			The size of the buffer.
 * \param *params		The encryption parameter block to translate.
 * \param extended_opts		TRUE to use the extended range of options; else FALSE.
 */

void encryption_build_params(char *buffer, size_t len, encrypt_params *params, osbool extended_opts)
{
	char		user[100];
	int		level, permissions = 0;

	if (buffer == NULL || params == NULL)
		return;

	*buffer = '\0';

	if (strlen(params->owner_password) > 0) {
		*user = '\0';

		if (strlen(params->access_password) > 0)
			string_printf(user, sizeof(user), "-sUserPassword=%s ", params->access_password);

		level = (extended_opts) ? 3 : 2;

		if (level == 2) {
			permissions = ACCESS_REV2_BASE;

			if (params->allow_print)
				permissions |= ACCESS_REV2_PRINT;

			if (params->allow_extraction)
				permissions |= ACCESS_REV2_COPY;

			if (params->allow_forms)
				permissions |= ACCESS_REV2_ANNOTATE;

			if (params->allow_modifications)
				permissions |= ACCESS_REV2_MODIFY;
		} else if (level == 3) {
			permissions = ACCESS_REV3_BASE;

			if (params->allow_print)
				permissions |= ACCESS_REV3_PRINT;

			if (params->allow_extraction)
				permissions |= ACCESS_REV3_COPYACCESS;

			if (params->allow_forms)
				permissions |= ACCESS_REV3_FORMS;

			if (params->allow_modifications)
				permissions |= ACCESS_REV3_MODIFY;

			if (params->allow_full_print)
				permissions |= ACCESS_REV3_PRINTFULL;

			if (params->allow_full_extraction)
				permissions |= ACCESS_REV3_COPYALL;

			if (params->allow_annotation)
				permissions |= ACCESS_REV3_ANNOTATE;

			if (params->allow_assembly)
				permissions |= ACCESS_REV3_ASSEMBLE;
		}

		string_printf(buffer, len, "-sOwnerPassword=%s %s-dEncryptionR=%d -dPermissions=%d ",
				params->owner_password, user, level, permissions);
	}
}

