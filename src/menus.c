/* PrintPDF - menus.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "sflib/menus.h"
#include "sflib/icons.h"
#include "sflib/msgs.h"
#include "sflib/event.h"
#include "sflib/debug.h"
#include "sflib/config.h"
#include "sflib/windows.h"
#include "sflib/heap.h"

/* Application header files */

#include "menus.h"

#include "bookmark.h"
#include "windows.h"
#include "choices.h"
#include "convert.h"
#include "pmenu.h"

/* ==================================================================================================================
 * Static global variables
 */

global_menus		menus;
wimp_w			popup_window;
wimp_i			popup_icon;

/* ==================================================================================================================
 * General
 */

/**
 * Load the menu definitions and link in dialogue boxes.
 *
 * Param  *menu_file		Pointer to the filename of the Menus file.
 */

void load_menu_definitions(char *menu_file)
{
	wimp_menu		*menu_list[20];
	menu_template		menu_defs;

	extern global_windows windows;

	menu_defs = load_menus (menu_file, NULL, menu_list);

	if (menu_defs != NULL) {
		load_menus_dbox(menu_defs, "ProgInfo", windows.prog_info);
		load_menus_dbox(menu_defs, "FileInfo", windows.file_info);
		load_menus_dbox(menu_defs, "SaveAs", windows.save_as);

		/* The order that menus are stored in the array depends on the
		 * order that they are defined in the menudefs file!
		 */

		menus.icon_bar = menu_list[0];
		menus.bookmarks = menu_list[1];
		menus.bookmarks_sub_view = menu_list[3];
		menus.bookmarks_sub_level = menu_list[4];
		menus.bookmarks_sub_insert = menu_list[5];
	}

	/* Register the current_menu pointer with eventlib. */

	event_set_menu_pointer(&(menus.menu_up));
}

/* ================================================================================================================== */

/**
 * Return a pointer to the name of the current menu.
 *
 * Param  *buffer		Pointer to a buffer to hold the menu name.
 * Return			Pointer to the returned name.
 */

char *get_current_menu_name(char *buffer)
{
	extern			global_menus menus;

	*buffer = '\0';

	if (menus.menu_up == menus.icon_bar)
		strcpy(buffer, "IconBarMenu");
	else if (menus.menu_up == menus.params)
		strcpy(buffer, "ParamsMenu");
	else if (menus.menu_up == menus.bookmarks)
		strcpy(buffer, "BookmarkMenu");
	else if (menus.menu_up == menus.bookmarks_list)
		strcpy(buffer, "BookmarksListMenu");

	return (buffer);
}

/* ==================================================================================================================
 * Iconbar menu
 */

/* Set and open the icon bar menu. */

void set_iconbar_menu (void)
{
  extern global_menus   menus;


  shade_menu_item (menus.icon_bar, ICONBAR_MENU_CHOICES, pdf_conversion_in_progress ());
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_iconbar_menu (wimp_pointer *pointer)
{
  extern global_menus   menus;


  set_iconbar_menu ();

  menus.menu_up = create_iconbar_menu (menus.icon_bar, pointer, 5, 2);
}


/* ------------------------------------------------------------------------------------------------------------------ */

/* Decode the menu selections. */

void decode_iconbar_menu (wimp_selection *selection, wimp_pointer *pointer)
{
  extern int            quit_flag;


  if (selection->items[0] == ICONBAR_MENU_HELP) /* Help */
  {
    os_cli ("%Filer_Run <PrintPDF$Dir>.!Help");
  }
  if (selection->items[0] == ICONBAR_MENU_QUEUE) /* Queue... */
  {
    open_queue_window (pointer); /* Call this first so that the pane is sized before we try and set its extent. */
    rebuild_queue_index ();
  }
  if (selection->items[0] == ICONBAR_MENU_CHOICES) /* Choices... */
  {
    open_choices_window (pointer);
  }
  else if (selection->items[0] == ICONBAR_MENU_QUIT) /* Quit */
  {
    if (!bookmark_files_unsaved() && !pending_files_in_queue())
      quit_flag = TRUE;
  }

  set_iconbar_menu ();
}

/* ==================================================================================================================
 * Parameter menus
 */

void open_param_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon)
{
  extern global_menus menus;

  popup_window = window;
  popup_icon = icon;

  menus.menu_up = create_popup_menu (menus.params, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Decode the menu selections. */

void decode_param_menu (wimp_selection *selection, wimp_pointer *pointer)
{
  switch (param_menu_ident ())
  {
    case PARAM_MENU_DEFAULT:
/*      set_param_value (popup_name, selection->items[0]); */
/*      read_param_name (indirected_icon_text (popup_window, popup_icon), popup_name); */
/*      wimp_set_icon_state (popup_window, popup_icon, 0, 0); */
/*      build_param_menu (popup_name, PARAM_MENU_DEFAULT); */
      break;

    case PARAM_MENU_CONVERT_OPTIMIZE:
      process_convert_optimize_menu (selection);
      break;

    case PARAM_MENU_CONVERT_VERSION:
      process_convert_version_menu (selection);
      break;

    case PARAM_MENU_CHOICES_OPTIMIZE:
      process_choices_optimize_menu (selection);
      break;

    case PARAM_MENU_CHOICES_VERSION:
      process_choices_version_menu (selection);
      break;
  }
}
