/* Copyright 2007-2015, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: version.c
 *
 * Version menu implementation.
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
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "version.h"

#include "pmenu.h"


#define VERSION_MENU_LENGTH 3


/* Function Prototypes. */

static int	version_tick_menu(version_params *params);


/**
 * Initialise the values in a version settings structure.
 *
 * \param *params		The version params struct to be initialised.
 */

void version_initialise_settings(version_params *params)
{
	params->standard_version = config_int_read("PDFVersion");
}

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
	int	i, tick;
	
	tick = version_tick_menu(params);

	for (i = 0; i < VERSION_MENU_LENGTH; i++)
		menus_tick_entry(menu, i, i == tick);
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


/**
 * Build up a text string in the supplied buffer containing the GS
 * parameters that reflect the contents of the given version
 * parameter block.
 *
 * \param *buffer		Buffer to hold the result.
 * \param len			The size of the buffer.
 * \param *params		The version parameter block to translate.
 */

void version_build_params(char *buffer, size_t len, version_params *params)
{
	char		level[100];

	pmenu_list_entry(level, "VersionList", params->standard_version);

	*buffer = '\0';

	snprintf(buffer, len, "-dCompatibilityLevel=%s ", level);
}

