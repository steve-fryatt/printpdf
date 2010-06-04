/* PrintPDF - pmenu.c
 *
 * (C) Stephen Fryatt, 2007
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

wimp_menu*    param_menu = NULL;
char*         menu_def = NULL;
int           param_ident;

/* ==================================================================================================================
 *
 */


wimp_menu *build_param_menu (char *param_list, int ident, int current)
{
  extern global_menus menus;

  char  *item_text;
  int   item, width;


  /* If a block has not been claimed for the menu, claim it with malloc now.  This will then stay for future use. */

  if (param_menu == NULL)
  {
    param_menu = (wimp_menu *) malloc (sizeof (wimp_menu) + sizeof (wimp_menu_entry) * PARAM_MENU_SIZE);
    menu_def = (char *) malloc (sizeof (char) * ((PARAM_MENU_LEN + 1) * PARAM_MENU_SIZE));
  }

  menus.params = param_menu;

  param_ident = ident;

  /* Look up the menu definition string and build the menu item by item. */

  msgs_lookup (param_list, menu_def, sizeof (char) * ((PARAM_MENU_LEN + 1) * PARAM_MENU_SIZE));

  #ifdef DEBUG
  debug_printf ("Menu def: '%s'", menu_def);
  #endif

  strcpy (menus.params->title_data.text, strtok (menu_def, ","));

  item = 0;
  width = 0;

  while ((item_text = strtok (NULL, ",")) != NULL && item < PARAM_MENU_SIZE)
  {

    if (*item_text == '-')
    {
      item_text++;

      if (item > 0)
      {
        menus.params->entries[item-1].menu_flags =  menus.params->entries[item-1].menu_flags | wimp_MENU_SEPARATE;
      }
    }

    menus.params->entries[item].menu_flags = (item == current) ? wimp_MENU_TICKED : 0;
    menus.params->entries[item].sub_menu = (wimp_menu *) -1;
    menus.params->entries[item].icon_flags = wimp_ICON_TEXT | wimp_ICON_FILLED | wimp_ICON_INDIRECTED |
                                             wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT |
                                             wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT;
    menus.params->entries[item].data.indirected_text.text = item_text;
    menus.params->entries[item].data.indirected_text.validation = NULL;
    menus.params->entries[item].data.indirected_text.size = PARAM_MENU_LEN;

    if (strlen (item_text) > width)
    {
      width = strlen (item_text);
    }

    item++;
  }

  menus.params->entries[item - 1].menu_flags |= wimp_MENU_LAST;

  menus.params->title_fg = wimp_COLOUR_BLACK;
  menus.params->title_bg = wimp_COLOUR_LIGHT_GREY;
  menus.params->work_fg = wimp_COLOUR_BLACK;
  menus.params->work_bg = wimp_COLOUR_WHITE;

  menus.params->width = (width + 1) * 16;
  menus.params->height = 44;
  menus.params->gap = 0;

  return (param_menu);
}

/* ------------------------------------------------------------------------------------------------------------------ */

int param_menu_ident (void)
{
  return (param_ident);
}

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

/* ================================================================================================================== */

int param_menu_len (char *param_list)
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

  while ((item_text = strtok (NULL, ",")) != NULL)
  {
    item++;
  }

  free (menu_def);

  return (item);
}
