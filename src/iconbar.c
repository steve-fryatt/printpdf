/* PrintPDF - iconbar.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/debug.h"
#include "sflib/errors.h"
#include "sflib/event.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/windows.h"

/* Application header files */

#include "iconbar.h"

#include "bookmark.h"
#include "choices.h"
#include "convert.h"
#include "menus.h"

/* *****************************************************************************
 * Function Prototypes
 * *****************************************************************************/

static void	iconbar_click_handler(wimp_pointer *pointer);
static void	iconbar_menu_prepare(wimp_w w, wimp_menu *menu, wimp_pointer *pointer);
static void	iconbar_menu_selection(wimp_w w, wimp_menu *menu, wimp_selection *selection);

/* ==================================================================================================================
 * Global variables.
 */

/* ==================================================================================================================
 *
 */

/**
 * Initialise the iconbar icon.
 */

void initialise_iconbar(void)
{
}


/**
 * Create or delete the icon on the iconbar, as appropriate.
 *
 * \param new_state		TRUE to create an icon; FALSE to remove it.
 */

void set_iconbar_icon(int new_state)
{
	static int		icon_present = FALSE;
	static wimp_i		icon_handle = (wimp_i) -1;

	extern global_menus	menus;

	wimp_icon_create	icon_bar;


	if (new_state != icon_present) {
		if (new_state == TRUE) {
			icon_bar.w = wimp_ICON_BAR_RIGHT;
			icon_bar.icon.extent.x0 = 0;
			icon_bar.icon.extent.x1 = 68;
			icon_bar.icon.extent.y0 = 0;
			icon_bar.icon.extent.y1 = 69;
			icon_bar.icon.flags = wimp_ICON_SPRITE | (wimp_BUTTON_CLICK << wimp_ICON_BUTTON_TYPE_SHIFT);
			msgs_lookup ("TaskSpr", icon_bar.icon.data.sprite, osspriteop_NAME_LIMIT);
			icon_handle = wimp_create_icon (&icon_bar);

			event_add_window_mouse_event(wimp_ICON_BAR, iconbar_click_handler);
			event_add_window_menu(wimp_ICON_BAR, menus.icon_bar, TRUE);
			event_add_window_menu_prepare(wimp_ICON_BAR, iconbar_menu_prepare);
			event_add_window_menu_selection(wimp_ICON_BAR, iconbar_menu_selection);

			icon_present = TRUE;
		} else {
			wimp_delete_icon (wimp_ICON_BAR, icon_handle);

			event_delete_window(wimp_ICON_BAR);

			icon_present = FALSE;
		}
	}
}


/**
 * Handle mouse clicks on the iconbar icon.
 *
 * \param *pointer		The Wimp mouse click event data.
 */

static void iconbar_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch (pointer->buttons) {
	case wimp_CLICK_SELECT:
		create_new_bookmark_window();
		break;

	case wimp_CLICK_ADJUST:
		open_queue_window(pointer); /* Call this first so that the pane is sized before we try and set its extent. */
		rebuild_queue_index();
		break;
	}
}


/**
 * Prepare the iconbar menu for (re)-opening.
 *
 * \param  w			The handle of the menu's parent window.
 * \param  *menu		Pointer to the menu being opened.
 * \param  *pointer		Pointer to the Wimp Pointer event block.
 */

static void iconbar_menu_prepare(wimp_w w, wimp_menu *menu, wimp_pointer *pointer)
{
	extern global_menus	menus;

	shade_menu_item(menus.icon_bar, ICONBAR_MENU_CHOICES, pdf_conversion_in_progress());
}


/**
 * Handle selections from the iconbar menu.
 *
 * \param  w			The window to which the menu belongs.
 * \param  *menu		Pointer to the menu itself.
 * \param  *selection		Pointer to the Wimp menu selction block.
 */

static void iconbar_menu_selection(wimp_w w, wimp_menu *menu, wimp_selection *selection)
{
	wimp_pointer		pointer;
	extern int		quit_flag;

	wimp_get_pointer_info(&pointer);

	switch(selection->items[0]) {
	case ICONBAR_MENU_HELP:
		os_cli("%Filer_Run <PrintPDF$Dir>.!Help");
		break;

	case ICONBAR_MENU_QUEUE:
		open_queue_window(&pointer); /* Call this first so that the pane is sized before we try and set its extent. */
 		rebuild_queue_index();
 		break;

 	case ICONBAR_MENU_CHOICES:
 		open_choices_window(&pointer);
 		break;

 	case ICONBAR_MENU_QUIT:
		if (!bookmark_files_unsaved() && !pending_files_in_queue())
			quit_flag = TRUE;
		break;
	}
}

