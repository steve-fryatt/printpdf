/* Copyright 2007-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: pmenu.c
 *
 * Parameter Menu implementation.
 */

/* ANSI C header files */

#include <stdlib.h>
#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/debug.h"
#include "sflib/msgs.h"
#include "sflib/string.h"

/* Application header files */

#include "pmenu.h"

#define PARAM_MENU_SIZE 10	/**< The number of parameters allowed.			*/
#define PARAM_MENU_LEN  32	/**< The number of characters allowed per parameter.	*/

/**
 * Return an entry from a comma-separated list of parameters in a message
 * token.
 *
 * \param *buffer		Pointer to a buffer to take the entry.
 * \param len			The size of the buffer.
 * \param *param_list		The list message token.
 * \param entry			The number of the item to return.
 * \return			Pointer to the \0 terminator in the buffer.
 */

char *pmenu_list_entry(char *buffer, size_t len, char* param_list, int entry)
{
	char	*menu_def, *item_text;
	int	item;

	menu_def = malloc(sizeof(char) * ((PARAM_MENU_LEN + 1) * PARAM_MENU_SIZE));

	/* Look up the menu definition string and build the menu item by item. */

	msgs_lookup(param_list, menu_def, sizeof(char) * ((PARAM_MENU_LEN + 1) * PARAM_MENU_SIZE));

	#ifdef DEBUG
	debug_printf("Menu def: '%s'", menu_def);
	#endif

	item_text = strtok(menu_def, ",");

	item = 0;

	do {
		if (*item_text == '-')
			item_text++;

		item++;
	} while (item <= entry && (item_text = strtok(NULL, ",")) != NULL);

	*buffer = '\0';

	if (item_text != NULL)
		string_copy(buffer, item_text, len);

	free(menu_def);

	return (buffer + strlen(buffer) + 1);
}

