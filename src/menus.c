/* PrintPDF - menus.c
 *
 * (C) Stephen Fryatt, 2005-2011
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
		menus.version_popup = menu_list[6];
		menus.optimize_popup = menu_list[7];
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
	else if (menus.menu_up == menus.bookmarks)
		strcpy(buffer, "BookmarkMenu");
	else if (menus.menu_up == menus.bookmarks_list)
		strcpy(buffer, "BookmarkListMenu");

	return (buffer);
}

