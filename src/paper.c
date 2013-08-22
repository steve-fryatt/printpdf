/* Copyright 2007-2013, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: paper.c
 *
 * Paper dialogue implementation.
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
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/windows.h"

/* Application header files */

#include "paper.h"

#include "ihelp.h"
#include "templates.h"


/* Paper Window icons. */

#define PAPER_ICON_CANCEL 0
#define PAPER_ICON_OK 1
#define PAPER_ICON_USE_DOC 2
#define PAPER_ICON_SIZE_MENU 3
#define PAPER_ICON_SIZE 4

/* List of Known (to GhostScript) Paper Sizes
 *
 * This list MUST match the Paper Size pop-up menu order, with
 * an offset back of 1 to allow for the "Custom" entry at the head
 * of the menu. Sizes which do not appear in the menu are added at
 * the end, to potentially allow the screening of the PS file for
 * valid paper sizes.
 */

static char paper_sizes[] = {
	"a0,"		// ISO A0
	"a1,"		// ISO A1
	"a2,"		// ISO A2
	"a3,"		// ISO A3
	"a4,"		// ISO A4
	"a5,"		// ISO A5
	"a6,"		// ISO A6
	"a7,"		// ISO A7
	"a8,"		// ISO A8
	"a9,"		// ISO A9
	"a10,"		// ISO A10
	"isob0,"	// ISO B0
	"isob1,"	// ISO B1
	"isob2,"	// ISO B2
	"isob3,"	// ISO B3
	"isob4,"	// ISO B4
	"isob5,"	// ISO B5
	"isob6,"	// ISO B6
	"c0,"		// ISO C0
	"c1,"		// ISO C1
	"c2,"		// ISO C2
	"c3,"		// ISO C3
	"c4,"		// ISO C4
	"c5,"		// ISO C5
	"c6,"		// ISO C6
	"11x17,"	// US 11 x 17
	"ledger,"	// US Ledger (17 x 11)
	"legal,"	// US Legal
	"letter,"	// US Letter
	"halfletter,"	// US Half Letter
	"lettersmall,"	// US Small Letter
	"archE,"	// Arch E
	"archD,"	// Arch D
	"archC,"	// Arch C
	"archB,"	// Arch B
	"archA,"	// Arch A
	"flsa,"		// US Foolscap
	"flse,"		// European Foolscap
	"jisb0,"	// JIS B0
	"jisb1,"	// JIS B1
	"jisb2,"	// JIS B2
	"jisb3,"	// JIS B3
	"jisb4,"	// JIS B4
	"jisb5,"	// JIS B5
	"jisb6,"	// JIS B6
	"b0,"		// ISO or JIS B0	* Not In Menu *
	"b1,"		// ISO or JIS B1	* Not In Menu *
	"b2,"		// ISO or JIS B2	* Not In Menu *
	"b3,"		// ISO or JIS B3	* Not In Menu *
	"b4,"		// ISO or JIS B4	* Not In Menu *
	"b5,"		// ISO or JIS B5	* Not In Menu *
	"a4small,"	// A4 Small		* Not In Menu *
};


static wimp_w		paper_window = NULL;				/**< The paper dialogue handle.			*/

static void		(*paper_dialogue_close_callback)(void) = NULL;	/**< Callback function for when dialogue is updated.	*/

/* Function Prototypes. */

static void		paper_click_handler(wimp_pointer *pointer);
static osbool		paper_keypress_handler(wimp_key *key);
static void		paper_shade_dialogue(void);


/**
 * Initialise the paper dialogue.
 */

void paper_initialise(void)
{
	paper_window = templates_create_window("Paper");
	ihelp_add_window(paper_window, "Paper", NULL);

	event_add_window_mouse_event(paper_window, paper_click_handler);
	event_add_window_key_event(paper_window, paper_keypress_handler);
	
//	event_add_window_icon_popup(paper_window, PAPER_ICON_SIZE_MENU, paper_size_menu, PAPER_ICON_SIZE, NULL);
}


/**
 * Initialise the values in a paper settings structure.
 *
 * \param *params		The paper params struct to be initialised.
 */

void paper_initialise_settings(paper_params *params)
{
	params->override_document = config_opt_read("PaperOverride");
	
	params->width = 288;
	params->height = 576;
}


/**
 * Save the settings from a paper settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The paper params struct to be saved.
 */

void paper_save_settings(paper_params *params)
{
	config_opt_set("PaperOverride", params->override_document);
}


/**
 * Build and open the paper size menu.
 *
 * \param *params		The paper parameter block to use for the menu.
 * \param *menu			The paper menu block.
 */

void paper_set_menu(paper_params *params, wimp_menu *menu)
{
	int		i;

	for (i = 0; i < OPTIMIZE_MENU_LENGTH; i++)
		menus_tick_entry(menu, i, i == optimize_tick_menu(params));
}


/**
 * Handle selections from the paper menu.
 *
 * \param *params		The paper parameter block for the menu.
 * \param *menu			The paper menu block.
 * \param *selection		The menu selection details.
 */

void paper_process_menu(paper_params *params, wimp_menu *menu, wimp_selection *selection)
{
	wimp_pointer		pointer;

	if (selection->items[0] == OPTIMIZE_MENU_LENGTH - 1) {
		wimp_get_pointer_info(&pointer);
		optimize_open_dialogue(params, &pointer);
	} else {
		params->standard_preset = selection->items[0];
	}
}


/**
 * Return the number of the menu item which should be ticked based on the
 * supplied parameter block.
 *
 * \param *params		The paper parameter block to be used.
 * \return			The menu entry to be ticked.
 */

static int paper_tick_menu(paper_params *params)
{
	int		item;

	if (params->standard_preset == -1)
		item = OPTIMIZE_MENU_LENGTH - 1;
	else
		item = params->standard_preset;

	return item;
}


/**
 * Set a callback handler to be called when the OK button of the
 * paper dialogue is clicked.
 *
 * \param callback		The callback function to use, or NULL.
 */

void paper_set_dialogue_callback(void (*callback)(void))
{
	paper_dialogue_close_callback = callback;
}


/**
 * Process mouse clicks in the paper dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void paper_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case PAPER_ICON_CANCEL:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;

	case PAPER_ICON_OK:
		if (paper_dialogue_close_callback != NULL)
			paper_dialogue_close_callback();
		break;
	}
}


/**
 * Process keypresses in the paper window.
 *
 * \param *key		The keypress event block to handle.
 * \return		TRUE if the event was handled; else FALSE.
 */

static osbool paper_keypress_handler(wimp_key *key)
{
	if (key == NULL)
		return FALSE;

	switch (key->c) {
	case wimp_KEY_RETURN:
		if (paper_dialogue_close_callback != NULL)
			paper_dialogue_close_callback();
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
 * Open the paper dialogue for the given parameter block.
 *
 * \param *params		The paper parameter block to be used.
 * \param *pointer		The current pointer state.
 */

void paper_open_dialogue(paper_params *params, wimp_pointer *pointer)
{
	icons_set_selected(paper_window, PAPER_ICON_USE_DOC, !params->override_document);

	//strcpy(icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_TITLE), params->title);
	//strcpy(icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_AUTHOR), params->author);
	//strcpy(icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_SUBJECT), params->subject);
	//strcpy(icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_KEYWORDS), params->keywords);

	paper_shade_dialogue();

	windows_open_transient_centred_at_pointer(paper_window, pointer);
}


/**
 * Store the settings from the currently open paper dialogue box in
 * a paper parameter block.
 *
 * \param *params		The paper parameter block to be used.
 */

void paper_process_dialogue(paper_params *params)
{
	params->override_document = !icons_get_selected(paper_window, PAPER_ICON_USE_DOC);

	//strcpy(params->title, icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_TITLE));
	//strcpy(params->author, icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_AUTHOR));
	//strcpy(params->subject, icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_SUBJECT));
	//strcpy(params->keywords, icons_get_indirected_text_addr(pdfmark_window, PDFMARK_ICON_KEYWORDS));

	wimp_create_menu((wimp_menu *) -1, 0, 0);
}


/**
 * Update the shading in the paper dialogue, based on the current icon
 * selections
 */

static void paper_shade_dialogue(void)
{
	icons_replace_caret_in_window(paper_window);
}


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given paper parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The paper parameter block to use.
 */

void paper_fill_field(wimp_w window, wimp_i icon, paper_params *params)
{
	char		token[20];

	if (params->override_document)
		msgs_lookup("PaperCust", icons_get_indirected_text_addr(window, icon), 20);
	else
		msgs_lookup("PaperDoc", icons_get_indirected_text_addr(window, icon), 20);

	//snprintf(token, sizeof(token), "Version%d", params->standard_version);
	//msgs_lookup(token, icons_get_indirected_text_addr(window, icon), 20);
	wimp_set_icon_state(window, icon, 0, 0);
}


/**
 * Build up a text string in the supplied buffer containing the GS
 * parameters that reflect the contents of the given paper
 * parameter block.
 *
 * \param *buffer		Buffer to hold the result.
 * \param len			The size of the buffer.
 * \param *params		The optimization parameter block to translate.
 */

void paper_build_params(char *buffer, size_t len, paper_params *params)
{
	*buffer = '\0';

	if (!params->override_document)
		return;

	snprintf(buffer, len, "-dFIXEDMEDIA -dDEVICEWIDTHPOINTS=%d -dDEVICEHEIGHTPOINTS=%d", params->width, params->height);
}

