/* Copyright 2007-2016, Stephen Fryatt (info@stevefryatt.org.uk)
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
#include "sflib/ihelp.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "paper.h"


#define PAPER_MENU_LENGTH 7

#define PAPER_MENU_DOCUMENT 0
#define PAPER_MENU_CUSTOM (PAPER_MENU_LENGTH - 1)

/* Paper Window icons. */

#define PAPER_ICON_CANCEL 0
#define PAPER_ICON_OK 1
#define PAPER_ICON_SIZE_X 3
#define PAPER_ICON_SIZE_Y 4
#define PAPER_ICON_MM 5
#define PAPER_ICON_INCH 6
#define PAPER_ICON_POINT 7


struct paper_definition {
	const char	*gsname;	/**< Ghostscript Paper Name			*/
	const char	*token;		/**< Display Token Name				*/
	int		submenu;	/**< Submenu containing paper size, or -1	*/
	int		menuitem;	/**< Menu Entry containing paper size, or -1	*/
};

/* List of Known (to GhostScript) Paper Sizes
 */

static const struct paper_definition paper_sizes[] = {
	{"a0",		"PaperIA0",	1,	0},	// ISO A0
	{"a1",		"PaperIA1",	1,	1},	// ISO A1
	{"a2",		"PaperIA2",	1,	2},	// ISO A2
	{"a3",		"PaperIA3",	1,	3},	// ISO A3
	{"a4",		"PaperIA4",	1,	4},	// ISO A4
	{"a5",		"PaperIA5",	1,	5},	// ISO A5
	{"a6",		"PaperIA6",	1,	6},	// ISO A6
	{"a7",		"PaperIA7",	1,	7},	// ISO A7
	{"a8",		"PaperIA8",	1,	8},	// ISO A8
	{"a9",		"PaperIA9",	1,	9},	// ISO A9
	{"a10",		"PaperIA10",	1,	10},	// ISO A10
	{"isob0",	"PaperIB0",	1,	11},	// ISO B0
	{"isob1",	"PaperIB1",	1,	12},	// ISO B1
	{"isob2",	"PaperIB2",	1,	13},	// ISO B2
	{"isob3",	"PaperIB3",	1,	14},	// ISO B3
	{"isob4",	"PaperIB4",	1,	15},	// ISO B4
	{"isob5",	"PaperIB5",	1,	16},	// ISO B5
	{"isob6",	"PaperIB6",	1,	17},	// ISO B6
	{"c0",		"PaperIC0",	1,	18},	// ISO C0
	{"c1",		"PaperIC1",	1,	19},	// ISO C1
	{"c2",		"PaperIC2",	1,	20},	// ISO C2
	{"c3",		"PaperIC3",	1,	21},	// ISO C3
	{"c4",		"PaperIC4",	1,	22},	// ISO C4
	{"c5",		"PaperIC5",	1,	23},	// ISO C5
	{"c6",		"PaperIC6",	1,	24},	// ISO C6
	{"11x17",	"PaperTab",	2,	0},	// US 11 x 17
	{"ledger",	"PaperLed",	2,	1},	// US Ledger (17 x 11)
	{"legal",	"PaperLeg",	2,	2},	// US Legal
	{"letter",	"PaperLet",	2,	3},	// US Letter
	{"halfletter",	"PaperHLt",	2,	4},	// US Half Letter
	{"lettersmall",	"PaperSLt",	2,	5},	// US Small Letter
	{"archE",	"PaperAE",	3,	0},	// Arch E
	{"archD",	"PaperAD",	3,	1},	// Arch D
	{"archC",	"PaperAC",	3,	2},	// Arch C
	{"archB",	"PaperAB",	3,	3},	// Arch B
	{"archA",	"PaperAA",	3,	4},	// Arch A
	{"flsa",	"PaperUSF",	5,	0},	// US Foolscap
	{"flse",	"PaperEUF",	5,	1},	// European Foolscap
	{"jisb0",	"PaperJB0",	4,	0},	// JIS B0
	{"jisb1",	"PaperJB1",	4,	1},	// JIS B1
	{"jisb2",	"PaperJB2",	4,	2},	// JIS B2
	{"jisb3",	"PaperJB3",	4,	3},	// JIS B3
	{"jisb4",	"PaperJB4",	4,	4},	// JIS B4
	{"jisb5",	"PaperJB5",	4,	5},	// JIS B5
	{"jisb6",	"PaperJB6",	4,	6},	// JIS B6
	{"b0",		NULL,		-1,	-1},	// ISO or JIS B0
	{"b1",		NULL,		-1,	-1},	// ISO or JIS B1
	{"b2",		NULL,		-1,	-1},	// ISO or JIS B2
	{"b3",		NULL,		-1,	-1},	// ISO or JIS B3
	{"b4",		NULL,		-1,	-1},	// ISO or JIS B4
	{"b5",		NULL,		-1,	-1},	// ISO or JIS B5
	{"a4small",	NULL,		-1,	-1},	// A4 Small
	{NULL,		NULL,		-1,	-1}
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
	
	event_add_window_icon_radio(paper_window, PAPER_ICON_MM, TRUE);
	event_add_window_icon_radio(paper_window, PAPER_ICON_INCH, TRUE);
	event_add_window_icon_radio(paper_window, PAPER_ICON_POINT, TRUE);
}


/**
 * Initialise the values in a paper settings structure.
 *
 * \param *params		The paper params struct to be initialised.
 */

void paper_initialise_settings(paper_params *params)
{
	params->override_document = config_opt_read("PaperOverride");
	params->preset_size = config_int_read("PaperPreset");
	
	params->width = config_int_read("PaperWidth");
	params->height = config_int_read("PaperHeight");
	params->units = config_int_read("PaperUnits");
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
	config_int_set("PaperPreset", params->preset_size);
	config_int_set("PaperWidth", params->width);
	config_int_set("PaperHeight", params->height);
	config_int_set("PaperUnits", params->units);
}


/**
 * Build and open the paper size menu.
 *
 * \param *params		The paper parameter block to use for the menu.
 * \param *menu			The paper menu block.
 */

void paper_set_menu(paper_params *params, wimp_menu *menu)
{
	int		i, menulen[PAPER_MENU_LENGTH];
	
	/* Get the submenu lengths and clear the top-level menu's ticks. */
	
	for (i = 0; i < PAPER_MENU_LENGTH; i++) {
		if (menu->entries[i].sub_menu != wimp_NO_SUB_MENU)
			menulen[i] = menus_get_entries(menu->entries[i].sub_menu);
		else
			menulen[i] = 0;
		
		menus_tick_entry(menu, i, FALSE);
	}

	/* Scan the paper list, setting/clearing submenu flags as we go. */

	for (i = 0; paper_sizes[i].gsname != NULL; i++) {
		if (paper_sizes[i].submenu == -1 || paper_sizes[i].menuitem == -1 ||
				paper_sizes[i].submenu >= PAPER_MENU_LENGTH ||
				paper_sizes[i].menuitem >= menulen[paper_sizes[i].submenu])
			continue;
		
		menus_tick_entry(menu->entries[paper_sizes[i].submenu].sub_menu, paper_sizes[i].menuitem,
				(params->override_document && i == params->preset_size) ? TRUE : FALSE);

		/* If the submenu contains the selected paper, tick the parent. */

		if (params->override_document && i == params->preset_size)
			menus_tick_entry(menu, paper_sizes[i].submenu, TRUE);
	}

	/* Set ticks for Document and Preset. */

	menus_tick_entry(menu, PAPER_MENU_DOCUMENT, !params->override_document);
	menus_tick_entry(menu, PAPER_MENU_CUSTOM, params->override_document && (params->preset_size == -1));
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
	wimp_pointer	pointer;
	int		i;

	if (selection->items[0] == PAPER_MENU_CUSTOM) {
		wimp_get_pointer_info(&pointer);
		paper_open_dialogue(params, &pointer);
	} else if (selection->items[0] == PAPER_MENU_DOCUMENT) {
		params->override_document = FALSE;
	} else {
		for (i = 0; paper_sizes[i].gsname != NULL &&
				(paper_sizes[i].submenu != selection->items[0] || paper_sizes[i].menuitem != selection->items[1]);
				i++);
	
		if (paper_sizes[i].gsname != NULL) {
			params->override_document = TRUE;
			params->preset_size = i;
		}
	}
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
	sprintf(icons_get_indirected_text_addr(paper_window, PAPER_ICON_SIZE_X),
			"%.2f", (double) params->width / 100.0);

	sprintf(icons_get_indirected_text_addr(paper_window, PAPER_ICON_SIZE_Y),
			"%.2f", (double) params->height / 100.0);

	icons_set_radio_group_selected(paper_window, params->units, 3,
			PAPER_ICON_MM, PAPER_ICON_INCH, PAPER_ICON_POINT);

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
	params->override_document = TRUE;
	params->preset_size = -1;

	params->width = (int) 100.0 * atof(icons_get_indirected_text_addr(paper_window, PAPER_ICON_SIZE_X));
	params->height = (int) 100.0 * atof(icons_get_indirected_text_addr(paper_window, PAPER_ICON_SIZE_Y));

	params->units = icons_get_radio_group_selected(paper_window, 3,
			PAPER_ICON_MM, PAPER_ICON_INCH, PAPER_ICON_POINT);

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
	const char	*token;
	int		i;

	if (!params->override_document)
		token = "PaperDoc";
	else if (params->preset_size == -1)
		token = "PaperCust";
	else {
		for (i = 0; paper_sizes[i].gsname != NULL && params->preset_size != i; i++);
		
		token = paper_sizes[i].token;
	}

	if (token == NULL)
		return;

	msgs_lookup((char *) token, icons_get_indirected_text_addr(window, icon), 20);
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
	int	i;
	double	width, height;
	
	*buffer = '\0';

	if (!params->override_document)
		return;
	
	if (params->preset_size == -1) {
		width = (double) params->width / 100.0;
		height = (double) params->height / 100.0;
		
		switch (params->units) {
		case PAPER_UNITS_MM:
			width *= 2.83464567;
			height *= 2.83464567;
			break;
		case PAPER_UNITS_INCH:
			width *= 72;
			height *= 72;
			break;
		default:
			break;
		}
		
		snprintf(buffer, len, "-dFIXEDMEDIA -dDEVICEWIDTHPOINTS=%.0f -dDEVICEHEIGHTPOINTS=%.0f", width, height);
	} else {
		for (i = 0; paper_sizes[i].gsname != NULL && params->preset_size != i; i++);
		
		if (paper_sizes[i].gsname != NULL)
			snprintf(buffer, len, "-dFIXEDMEDIA -sPAPERSIZE=%s", paper_sizes[i].gsname);
	}
}

