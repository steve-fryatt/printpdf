/* Copyright 2005-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: templates.h
 *
 * Window and menu template support.
 */

#ifndef PRINTPDF_TEMPLATES
#define PRINTPDF_TEMPLATES

/**
 * The menus defined in the menus template, in the order that they
 * appear.
 */

enum templates_menus {
	TEMPLATES_MENU_ICONBAR = 0,						/**< The iconbar menu.			*/
	TEMPLATES_MENU_BOOKMARKS,
	TEMPLATES_MENU_BOOKMARKS_FILE,
	TEMPLATES_MENU_BOOKMARKS_VIEW,
	TEMPLATES_MENU_BOOKMARKS_LEVEL,
	TEMPLATES_MENU_BOOKMARKS_INSERT,
	TEMPLATES_MENU_VERSION,
	TEMPLATES_MENU_OPTIMIZATION,
	TEMPLATES_MENU_MAX_EXTENT						/**< Determine the number of entries.	*/
};


/**
 * Open the window templates file for processing.
 *
 * \param *file		The template file to open.
 */

void templates_open(char *file);


/**
 * Close the window templates file.
 */

void templates_close(void);


/**
 * Load a window definition from the current templates file.
 *
 * \param *name		The name of the template to load.
 * \return		Pointer to the loaded definition, or NULL.
 */

wimp_window *templates_load_window(char *name);


/**
 * Create a window from the current templates file.
 *
 * \param *name		The name of the window to create.
 * \return		The window handle if the new window.
 */


wimp_w templates_create_window(char *name);

/**
 * Load the menu file into memory and link in any dialogue boxes.  The
 * templates file must be open when this function is called.
 *
 * \param *file		The filename of the menu file.
 */

void templates_load_menus(char *file);


/**
 * Link a dialogue box to the menu block once the template has been
 * loaded into memory.
 *
 * \param *dbox		The dialogue box name in the menu tree.
 * \param w		The window handle to link.
 */

void templates_link_menu_dialogue(char *dbox, wimp_w w);


/**
 * Return a menu handle based on a menu file index value.
 *
 * \param menu		The menu file index of the required menu.
 * \return		The menu block pointer, or NULL for an invalid index.
 */

wimp_menu *templates_get_menu(enum templates_menus menu);


/**
 * Return a pointer to the name of the current menu.
 *
 * \param *buffer	Pointer to a buffer to hold the menu name.
 * \return		Pointer to the returned name.
 */

char *templates_get_current_menu_name(char *buffer);

#endif

