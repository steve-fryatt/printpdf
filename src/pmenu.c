/* PrintPDF - pmenu.c
 *
 * (C) Stephen Fryatt, 2007-2011
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

/* Application header files */

#include "pmenu.h"

#include "menus.h"

/* ==================================================================================================================
 * Global variables.
 */


char*         menu_def = NULL;
int           param_ident;

/* ================================================================================================================== */

char *param_menu_entry (char *buffer, char* param_list, int entry)
{
  char  *menu_def, *item_text;
  int   item;

  menu_def = (char *) malloc (sizeof (char) * ((PARAM_MENU_LEN + 1) * PARAM_MENU_SIZE));

  /* Look up the menu definition string and build the menu item by item. */

  msgs_lookup (param_list, menu_def, sizeof (char) * ((PARAM_MENU_LEN + 1) * PARAM_MENU_SIZE));

  #ifdef DEBUG
  debug_printf ("Menu def: '%s'", menu_def);
  #endif

  item_text = strtok (menu_def, ",");

  item = 0;

  do
  {
    if (*item_text == '-')
    {
      item_text++;
    }

    item++;
  }
  while (item <= entry && (item_text = strtok (NULL, ",")) != NULL);

  *buffer = '\0';

  if (item_text != NULL)
  {
    strcpy (buffer, item_text);
  }

  free (menu_def);

  return (buffer + strlen (buffer) + 1);
}

