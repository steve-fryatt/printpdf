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
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/windows.h"

/* Application header files */

#include "paper.h"

#include "pmenu.h"
#include "templates.h"


//#define VERSION_MENU_LENGTH 3


/* Function Prototypes. */

//static int	version_tick_menu(version_params *params);


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

#if 0
/**
 * Save the settings from a version settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The version params struct to be saved.
 */

void version_save_settings(version_params *params)
{
	config_int_set("PDFVersion", params->standard_version);
}


/**
 * Build and open the PDF version menu.
 *
 * \param *params		The version parameter block to use for the menu.
 * \param *menu			The version menu block.
 */

void version_set_menu(version_params *params, wimp_menu *menu)
{
	int		i;

	for (i = 0; i < VERSION_MENU_LENGTH; i++)
		menus_tick_entry(menu, i, i == version_tick_menu(params));
}


/**
 * Handle selections from the PDF version menu.
 *
 * \param *params		The version parameter block for the menu.
 * \param *menu			The version menu block.
 * \param *selection		The menu selection details.
 */

void version_process_menu(version_params *params, wimp_menu *menu, wimp_selection *selection)
{
	params->standard_version = selection->items[0];
}


/**
 * Return the number of the menu item which should be ticked based on the
 * supplied parameter block.
 *
 * \param *params		The version parameter block to be used.
 * \return			The menu entry to be ticked.
 */

static int version_tick_menu(version_params *params)
{
	return params->standard_version;
}


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given PDF version parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The version parameter block to use.
 */

void version_fill_field(wimp_w window, wimp_i icon, version_params *params)
{
	char		token[20];

	snprintf(token, sizeof(token), "Version%d", params->standard_version);
	msgs_lookup(token, icons_get_indirected_text_addr(window, icon), 20);
	wimp_set_icon_state(window, icon, 0, 0);
}
#endif

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

