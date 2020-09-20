/* Copyright 2010-2017, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
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
 * \file: bookmark.c
 *
 * Bookmark editor implementation.
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/hourglass.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/territory.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/dataxfer.h"
#include "sflib/debug.h"
#include "sflib/errors.h"
#include "sflib/event.h"
#include "sflib/general.h"
#include "sflib/icons.h"
#include "sflib/ihelp.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/saveas.h"
#include "sflib/string.h"
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "bookmark.h"

#include "convert.h"
#include "main.h"
#include "pdfmark.h"
#include "pmenu.h"


typedef struct bookmark_node {
	char			title[MAX_BOOKMARK_LEN];
	int			page;		/*< Destination page number.		*/
	int			yoffset;	/*< Destination Y offset (millipt from top).	*/
	int			level;
	int			count;

	osbool			expanded;

	struct bookmark_node	*next;
} bookmark_node;

typedef struct bookmark_redraw {
	bookmark_node		*node;
	int			selected;
} bookmark_redraw;

/* Not a typedef, as that is done in the header file. */

struct bookmark_block {
	char			name[MAX_BOOKMARK_BLOCK_NAME];
	char			filename[MAX_BOOKMARK_FILENAME];
	char			window_title[MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10];
	os_date_and_time	datestamp;

	osbool			unsaved;

	wimp_w			window;
	wimp_w			toolbar;
	wimp_i			edit_icon;

	bookmark_redraw		*redraw;
	int			lines;

	int			column_pos[BOOKMARK_WINDOW_COLUMNS];
	int			column_width[BOOKMARK_WINDOW_COLUMNS];
	int			caret_row;
	int			caret_col;

	int			menu_row;
	int			drag_row;

	bookmark_node		*root;
	int			nodes;

	osbool			drag_complete;

	struct bookmark_block	*next;
};


/* ****************************************************************************
 * Function prototypes
 * ****************************************************************************/

/* PDF Creation Interface */

/* Bookmark Block Management */

static bookmark_block	*bookmark_create_block(void);
static void		bookmark_delete_block(bookmark_block *bookmark);
static bookmark_node	*bookmark_insert_node(bookmark_block *bm, bookmark_node *before);
static void		bookmark_delete_node(bookmark_block *bm, bookmark_node *node);
static void		bookmark_set_unsaved_state(bookmark_block *bm, osbool unsaved);

static bookmark_block	*bookmark_find_window(wimp_w window);
static bookmark_block	*bookmark_find_toolbar(wimp_w window);
static bookmark_block	*bookmark_find_name(char *name);
static bookmark_block	*bookmark_find_block(bookmark_block *block);

/* Bookmark Window Handling */

static void		bookmark_open_window(bookmark_block *bm);
static void		bookmark_close_window(wimp_close *close);
static void		bookmark_window_decode_help(char *buffer, wimp_w w, wimp_i i, os_coord pos, wimp_mouse_state buttons);
static void		bookmark_redraw_window(wimp_draw *redraw);
static void		bookmark_click_handler(wimp_pointer *pointer);
static osbool		bookmark_key_handler(wimp_key *key);
static void		bookmark_lose_caret_handler(wimp_caret *caret);
static void		bookmark_gain_caret_handler(wimp_caret *caret);
static void		bookmark_scroll_handler(wimp_scroll *scroll);
static void		bookmark_change_edit_row(bookmark_block *bm, int direction, wimp_caret *caret);
static void		bookmark_insert_edit_row_from_keypress(bookmark_block *bm, wimp_caret *caret);
static int		bookmark_insert_edit_row(bookmark_block *bm, bookmark_node *node, int direction);
static void		bookmark_delete_edit_row(bookmark_block *bm, bookmark_node *node);
static void		bookmark_change_edit_row_indentation(bookmark_block *bm, bookmark_node *node, int action);
static void		bookmark_toolbar_set_expansion_icons(bookmark_block *bm, int *expand, int *contract);
static void		bookmark_tree_node_expansion(bookmark_block *bm, osbool expand);
static int		bookmark_place_edit_icon(bookmark_block *bm, int row, int col);
static void		bookmark_remove_edit_icon(void);
static void		bookmark_resync_edit_with_file(void);
static void		bookmark_update_window_title(bookmark_block *bm);
static void		bookmark_force_window_redraw(bookmark_block *bm, int from, int to);
static void		bookmark_set_window_extent(bookmark_block *bm);
static void		bookmark_set_window_columns(bookmark_block *bm);
static void		bookmark_calculate_window_row_start(bookmark_block *bm, int row);
static int		bookmark_calculate_window_click_row(bookmark_block *bm, os_coord *pos, wimp_window_state *state);
static int		bookmark_calculate_window_click_column(bookmark_block *bm, os_coord *pos, wimp_window_state *state);
static void		bookmark_line_drag(bookmark_block *bm, int line);
static void		bookmark_terminate_line_drag(wimp_dragged *drag, void *data);


/* Bookmark Toolbar Handling */

static void		bookmark_toolbar_click_handler(wimp_pointer *pointer);
static osbool		bookmark_toolbar_key_handler(wimp_key *key);

/* Bookmark Window Menu Handling */

static void		bookmark_menu_prepare(wimp_w w, wimp_menu *menu, wimp_pointer *pointer);
static void		bookmark_menu_selection(wimp_w w, wimp_menu *menu, wimp_selection *selection);
static void		bookmark_menu_close(wimp_w w, wimp_menu *menu);
static void		bookmark_menu_warning(wimp_w w, wimp_menu *menu, wimp_message_menu_warning *warning);

/* File Info Dialogue Handling */

static void		bookmark_prepare_file_info_window(bookmark_block *bm);

/* SaveAs Dialogue Handling */

static void		bookmark_prepare_save_window(bookmark_block *bm, wimp_pointer *pointer);
static void		bookmark_start_direct_menu_save(bookmark_block *bm);

/* Bookmark Data Processing */

static osbool		bookmarks_save_file(char *filename, osbool selection, void *data);
static void		bookmark_rebuild_data(bookmark_block *bm);

/* ****************************************************************************
 * Macros
 * ****************************************************************************/

/* Line position calculations. */

#define LINE_BASE(x) (-((x)+1) * BOOKMARK_LINE_HEIGHT - BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_WINDOW_MARGIN)
#define LINE_Y0(x) (LINE_BASE(x) + BOOKMARK_LINE_OFFSET)
#define LINE_Y1(x) (LINE_BASE(x) + BOOKMARK_LINE_OFFSET + BOOKMARK_ICON_HEIGHT)

/* ****************************************************************************
 * Global variables
 * ****************************************************************************/

/* Pointer to bookmark window data list. */

static bookmark_block		*bookmarks_list = NULL;
static int			untitled_number = 1;

static bookmark_block		*bookmarks_edit = NULL;
static char			*bookmarks_edit_buffer = NULL;

static wimp_menu		*bookmarks_list_menu = NULL;
static bookmark_block		**bookmarks_list_menu_links = NULL;
static int			bookmarks_list_menu_size = 0;

static wimp_window		*bookmark_window_def = NULL;
static wimp_window		*bookmark_pane_def = NULL;

static wimp_w			bookmark_window_fileinfo = NULL;

static struct saveas_block	*bookmark_saveas_file = NULL;

static wimp_menu		*bookmark_menu = NULL;
static wimp_menu		*bookmark_menu_insert = NULL;
static wimp_menu		*bookmark_menu_level = NULL;
static wimp_menu		*bookmark_menu_view = NULL;


/* ****************************************************************************
 * Bookmarks System Initialisation and Termination
 * ****************************************************************************/

/**
 * Initialise the bookmarks system.
 */

void bookmarks_initialise(void)
{
	bookmark_menu = templates_get_menu("BookmarksMenu");
	ihelp_add_menu(bookmark_menu, "BookmarkMenu");
	bookmark_menu_insert = templates_get_menu("BookmarksInsertSubmenu");
	bookmark_menu_view = templates_get_menu("BookmarksViewSubmenu");
	bookmark_menu_level = templates_get_menu("BookmarksLevelSubmenu");

	bookmark_window_fileinfo = templates_create_window("FileInfo");
	templates_link_menu_dialogue("FileInfo", bookmark_window_fileinfo);
	ihelp_add_window(bookmark_window_fileinfo, "FileInfo", NULL);

	bookmark_saveas_file = saveas_create_dialogue(FALSE, "file_1d8", bookmarks_save_file);

	bookmark_window_def = templates_load_window("BMark");
	bookmark_window_def->icon_count = 0;

	bookmark_pane_def = templates_load_window("BMarkPane");
}

/**
 * Terminate the bookmarks system, freeing up the resources used.
 */

void bookmarks_terminate(void)
{
	/* Work through the bookmarks list, deleting everything. */

	while (bookmarks_list != NULL)
		bookmark_delete_block(bookmarks_list);
}


/* ****************************************************************************
 * PDF Creation Interface
 * ****************************************************************************/

/**
 * Initialise a bookmarks settings block with default parameters.
 *
 * \param  *params		The parameter block to initialise.
 */

void bookmark_initialise_settings(bookmark_params *params)
{
	params->bookmarks = NULL;
}


/**
 * Handle selection events from the bookmarks pop-up menu.
 *
 * \param  *params		The associated bookmarks parameters.
 * \param  *selection		The Wimp Menu selection block.
 */

void bookmark_process_menu(bookmark_params *params, wimp_selection *selection)
{
	bookmark_block		*bm;

	params->bookmarks = bookmark_find_block(params->bookmarks);

	if (selection->items[0] == 0) {
		bm = bookmark_create_new_window();
		if (bm != NULL)
			params->bookmarks = bm;
	} else if (selection->items[0] > 0 && selection->items[0] < bookmarks_list_menu_size) {
		params->bookmarks = bookmarks_list_menu_links[selection->items[0]];
	}
}


/**
 * Build the bookmarks pop-up menu used for selecting a bookmark set to use, and
 * register it with the global_menus structure.
 *
 * \param  *params		Bookamrks param block to use to set ticks.
 * \return			The menu block, or NULL.
 */

wimp_menu *bookmark_build_menu(bookmark_params *params)
{
	int			count, item, width;
	bookmark_block		*bm;

	params->bookmarks = bookmark_find_block(params->bookmarks);

	/* Count up the entries; we need a menu length one greater, to allow
	 * for the 'None' entry.
	 */

	for (bm = bookmarks_list, count = 2; bm != NULL; bm = bm->next)
		count++;

	/* (Re-)Allocate memory for the menu and block links. */

	if (count != bookmarks_list_menu_size) {
		if (bookmarks_list_menu != NULL)
			free(bookmarks_list_menu);
		if (bookmarks_list_menu_links != NULL)
			free(bookmarks_list_menu_links);

		bookmarks_list_menu = (wimp_menu *) malloc(sizeof (wimp_menu_base) + sizeof (wimp_menu_entry) * count);
		bookmarks_list_menu_links = (bookmark_block **) malloc(sizeof(bookmark_block) * count);
		bookmarks_list_menu_size = count;
	}

	/* If we got the memory, build the menu and links. */

	if (bookmarks_list_menu != NULL && bookmarks_list_menu_links != NULL) {
		msgs_lookup("BMListMenu", bookmarks_list_menu->title_data.text, 12);

		item = 0;

		bookmarks_list_menu->entries[item].menu_flags = wimp_MENU_SEPARATE;
		bookmarks_list_menu->entries[item].sub_menu = (wimp_menu *) -1;
		bookmarks_list_menu->entries[item].icon_flags = wimp_ICON_TEXT | wimp_ICON_FILLED |
				wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT |
				wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT;
		msgs_lookup("BMNew", bookmarks_list_menu->entries[item].data.text, 12);

		bookmarks_list_menu_links[item] = NULL;

		width = strlen(bookmarks_list_menu->entries[item].data.text);

		item++;

		bookmarks_list_menu->entries[item].menu_flags = (count > 2) ? wimp_MENU_SEPARATE : 0;
		bookmarks_list_menu->entries[item].sub_menu = (wimp_menu *) -1;
		bookmarks_list_menu->entries[item].icon_flags = wimp_ICON_TEXT | wimp_ICON_FILLED |
				wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT |
				wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT;
		msgs_lookup("None", bookmarks_list_menu->entries[item].data.text, 12);

		bookmarks_list_menu_links[item] = NULL;

		if (params->bookmarks == NULL)
			bookmarks_list_menu->entries[item].menu_flags |= wimp_MENU_TICKED;

		width = strlen(bookmarks_list_menu->entries[item].data.text);

		for (bm = bookmarks_list; bm != NULL; bm = bm->next) {
			item++;

			bookmarks_list_menu->entries[item].menu_flags = 0;
			bookmarks_list_menu->entries[item].sub_menu = (wimp_menu *) -1;
			bookmarks_list_menu->entries[item].icon_flags = wimp_ICON_TEXT | wimp_ICON_FILLED |
					wimp_ICON_INDIRECTED | wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT |
					wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT;
			bookmarks_list_menu->entries[item].data.indirected_text.text = bm->name;
			bookmarks_list_menu->entries[item].data.indirected_text.validation = NULL;
			bookmarks_list_menu->entries[item].data.indirected_text.size = MAX_BOOKMARK_BLOCK_NAME;

			bookmarks_list_menu_links[item] = bm;

			if (params->bookmarks == bm)
				bookmarks_list_menu->entries[item].menu_flags |= wimp_MENU_TICKED;

			if (strlen(bm->name) > width)
				width = strlen(bm->name);
		}

		bookmarks_list_menu->entries[item].menu_flags |= wimp_MENU_LAST;

		bookmarks_list_menu->title_fg = wimp_COLOUR_BLACK;
		bookmarks_list_menu->title_bg = wimp_COLOUR_LIGHT_GREY;
		bookmarks_list_menu->work_fg = wimp_COLOUR_BLACK;
		bookmarks_list_menu->work_bg = wimp_COLOUR_WHITE;

		bookmarks_list_menu->width = (width + 1) * 16;
		bookmarks_list_menu->height = 44;
		bookmarks_list_menu->gap = 0;
	}

	return bookmarks_list_menu;
}


/**
 * Load a bookmark file and set it as the current conversion file.
 *
 * \param  *params		The bookmark parameters.
 * \param  *filename		The file to load.
 * \return			TRUE if the file loaded OK; else FALSE.
 */

osbool bookmark_load_and_select_file(bookmark_params *params, char *filename)
{
	bookmark_block		*bm;

	bm = bookmarks_load_file(filename);

	if (bm != NULL)
		params->bookmarks = bm;

	return (bm == NULL) ? FALSE : TRUE;
}


/**
 * Fill the Bookmark info field based on the supplied parameters.
 *
 * \param  window		The window the field is in.
 * \param  icon			The icon for the field.
 * \param  *params		The parameters to use.
 */

void bookmark_fill_field(wimp_w window, wimp_i icon, bookmark_params *params)
{
	size_t	length;

	params->bookmarks = bookmark_find_block(params->bookmarks);

	if (params == NULL || params->bookmarks == NULL) {
		icons_msgs_lookup(window, icon, "None");
	} else {
		length = icons_get_indirected_text_length(window, icon);

		icons_strncpy(window, icon, params->bookmarks->name);

		if (strlen(params->bookmarks->name) >= length)
			string_copy(icons_get_indirected_text_addr(window, icon) + length - 4, "...", 4);
	}

	wimp_set_icon_state(window, icon, 0, 0);
}


/**
 * Indicate if the supplied bookmark parameters have data available for a
 * conversion.
 *
 * \param  *params		The parameter block to check.
 * \return			1 if data is available; else 0.
 */

int bookmark_data_available(bookmark_params *params)
{
	params->bookmarks = bookmark_find_block(params->bookmarks);

	return (params != NULL && params->bookmarks != NULL);
}


/**
 * Check the status of the supplied parameter block, and update it if anything
 * is invalid.
 *
 * \param  *params		The parameter block to check.
 * \return			1 if parameters were chnaged; else 0.
 */

int bookmark_validate_params(bookmark_params *params)
{
	if (params != NULL && params->bookmarks != NULL) {
		params->bookmarks = bookmark_find_block(params->bookmarks);

		if (params->bookmarks == NULL)
			return 1;
	}

	return 0;
}

/* ****************************************************************************
 * Bookmark Block Management
 * ****************************************************************************/

/**
 * Create a new bookmark block and initialise it with some standard
 * values.
 *
 * \return		Pointer to block; or NULL if failed.
 */

static bookmark_block *bookmark_create_block(void)
{
	bookmark_block		*new;
	char			name[MAX_BOOKMARK_BLOCK_NAME], number[10];

	new = (bookmark_block *) malloc(sizeof(bookmark_block));

	if (new != NULL) {
		do {
			string_printf(number, 10, "%d", untitled_number++);
			msgs_param_lookup("UntBM", name, MAX_BOOKMARK_BLOCK_NAME,
					number, NULL, NULL, NULL);
		} while (bookmark_find_name(name) != NULL);
		string_copy(new->name, name, MAX_BOOKMARK_BLOCK_NAME);
		string_copy(new->filename, "", MAX_BOOKMARK_FILENAME);
		string_copy(new->window_title, "", MAX_BOOKMARK_FILENAME + MAX_BOOKMARK_BLOCK_NAME + 10);
		new->unsaved = FALSE;
		new->window = NULL;
		new->toolbar = NULL;
		new->redraw = NULL;
		new->root = NULL;
		new->lines = 0;
		new->nodes = 0;
		new->caret_row = -1;
		new->caret_col = -1;
		new->edit_icon = wimp_ICON_WINDOW;
		new->menu_row = -1;
		new->drag_row = -1;
		new->drag_complete = FALSE;

		bookmark_update_window_title(new);

		new->next = bookmarks_list;
		bookmarks_list = new;
	}

	return new;
}

/**
 * Delete a bookmark block and its associated data.
 *
 * ** This does not delete the windows tied to the block! **
 *
 * \param  *bookmark		The block to delete.
 */

static void bookmark_delete_block(bookmark_block *bookmark)
{
	bookmark_block		**bm, *f;
	bookmark_node		*n, *nf;

	bm = &bookmarks_list;

	while ((*bm != NULL) && (*bm != bookmark))
		bm = &((*bm)->next);

	if (*bm != NULL) {
		f = *bm;
		*bm = (*bm)->next;

		n = f->root;
		while (n != NULL) {
			nf = n;
			n = n->next;
			free(nf);
		}

		if (f->redraw != NULL)
			free(f->redraw);

		free(f);

		/* In case the deleted block was the currently selected bookmark
		 * in an active conversion...
		 */

		convert_validate_params();
	}
}


/**
 * Insert a new node into the list.  If before is not NULL, the node is inserted
 * beofre that node in the list; otherwise, it is inserted at the end.
 *
 * \param  *bm			The bookmark block to insert into.
 * \param  *before		The node to insert before (NULL for list end).
 * \return			The inserted node; NULL indicates a failure.
 */

static bookmark_node *bookmark_insert_node(bookmark_block *bm, bookmark_node *before)
{
	bookmark_node		**node, *new = NULL;

	if (bm == NULL)
		return new;

	node = &(bm->root);
	if (node == NULL)
		return new;

	while (*node != before && *node != NULL)
		node = &((*node)->next);

	new = (bookmark_node *) malloc(sizeof(bookmark_node));

	if (new == NULL)
		return new;

	string_copy(new->title, "", MAX_BOOKMARK_LEN);

	new->page = 0;
	new->yoffset = -1;
	new->expanded = TRUE;
	new->level = 1;
	new->count = 0;
	new->next = NULL;

	new->next = *node;
	*node = new;

	return new;
}


/**
 * Delete a bookmark node from a block.
 *
 * \param  *bm			The bookmark block.
 * \param  *node		The node to delete.
 */

static void bookmark_delete_node(bookmark_block *bm, bookmark_node *node)
{
	bookmark_node		*parent = NULL;

	if (bm == NULL || node == NULL || bm->root == NULL)
		return;

	/* Find the parent node; if parent == NULL then either the node is at
	 * the head of the list or it wasn't in the list.
	 */

	if (bm->root != node)
		for (parent = bm->root; parent != NULL && parent->next != node; parent = parent->next);

	/* At this point, give up if the supplied node wasn't in the list. */

	if (parent == NULL && bm->root != node)
		return;

	/* Unlink the node and free its memory. */

	if (parent != NULL)
		parent->next = node->next;
	else
		bm->root = node->next;

	free(node);
}


/**
 * Set the 'unsaved' status of a bookmark block.
 *
 * \param  *bm			The block to update.
 * \param  unsaved		The unsaved status (TRUE = unsaved; FALSE = saved).
 */

static void bookmark_set_unsaved_state(bookmark_block *bm, osbool unsaved)
{
	if (unsaved != bm->unsaved) {
		bm->unsaved = unsaved;
		bookmark_update_window_title(bm);
	}
}


/**
 * Find a bookmark block by its window handle.
 *
 * \param  window		The handle of the window.
 * \return 			The block address, or NULL if it wasn't found.
 */

static bookmark_block *bookmark_find_window(wimp_w window)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && bm->window != window)
		bm = bm->next;

	return bm;
}


/**
 * Find a bookmark block by its toolbar handle.
 *
 * \param  window		The handle of the toolbar.
 * \return 			The block address, or NULL if it wasn't found.
 */

static bookmark_block *bookmark_find_toolbar(wimp_w window)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && bm->toolbar != window)
		bm = bm->next;

	return bm;
}


/**
 * Find a bookmark block by its name (matched case-insensitively).
 *
 * \param  *name		The block name to find.
 * \return 			The block address, or NULL if it wasn't found.
 */

static bookmark_block *bookmark_find_name(char *name)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && string_nocase_strcmp(bm->name, name) != 0)
		bm = bm->next;

	return bm;
}


/**
 * Find a bookmark block by block address.  This is used for validating that
 * an address supplied from outside the module really is a valid block.
 *
 * \param  *block		The block to validate.
 * \return 			The block address, or NULL if it failed.
 */

static bookmark_block *bookmark_find_block(bookmark_block *block)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && bm != block)
		bm = bm->next;

	return bm;
}


/**
 * Check for any unsaved bookmark files and prompt the user if found.
 *
 * \return			TRUE if there are unsaved files to rescue; else FALSE.
 */

osbool bookmark_files_unsaved(void)
{
	wimp_error_box_selection	button = wimp_ERROR_BOX_SELECTED_NOTHING;
	bookmark_block			*bm = bookmarks_list;

	while ((bm != NULL) && !(bm->unsaved))
		bm = bm->next;

	if (bm != NULL)
		button = error_msgs_report_question("FilesNotSaved", "FilesNotSavedB");

	return (button == 4) ? TRUE : FALSE;
}

/* ****************************************************************************
 * Bookmark Window Handling
 * ****************************************************************************/

/**
 * Create and open a new bookmark window.
 *
 * \return			The address of the new block; else NULL.
 */

bookmark_block *bookmark_create_new_window(void)
{
	bookmark_block		*new;

	new = bookmark_create_block();

	if (new != NULL) {
		bookmark_insert_node(new, NULL);
		bookmark_rebuild_data(new);
		bookmark_open_window(new);
	}

	return new;
}

/**
 * Create and open a window for a bookmark block.
 *
 * \param *bm		The block to open the window for.
 */

static void bookmark_open_window(bookmark_block *bm)
{
	static int		open_x_offset = BOOKMARK_WINDOW_STANDOFF;
	static int		open_y_offset = BOOKMARK_WINDOW_STANDOFF;

	int			screen, visible, extent;

	if (bm != NULL && bm->window == NULL && bm->toolbar == NULL) {
		bookmark_window_def->title_data.indirected_text.text = bm->window_title;
		bookmark_window_def->title_data.indirected_text.size = MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10;

		/* Set the X position of the window. */

		screen = general_mode_width();

		visible = BOOKMARK_WINDOW_WIDTH;
		if (visible > (screen - 2*BOOKMARK_WINDOW_STANDOFF - 4*BOOKMARK_WINDOW_OPENSTEP))
			visible = (screen - 2*BOOKMARK_WINDOW_STANDOFF - 4*BOOKMARK_WINDOW_OPENSTEP);

		bookmark_window_def->visible.x0 = open_x_offset;
		bookmark_window_def->visible.x1 = open_x_offset + visible;

		/* Update the new opening position. */

		open_x_offset += BOOKMARK_WINDOW_OPENSTEP;
		if ((open_x_offset + visible) > (screen - BOOKMARK_WINDOW_STANDOFF))
			open_x_offset = BOOKMARK_WINDOW_STANDOFF;

		/* Set the Y position of the window. */

		screen = general_mode_height();

		bookmark_window_def->visible.y1 = screen - open_y_offset;
		bookmark_window_def->visible.y0 = bookmark_window_def->visible.y1 +
			LINE_Y0(BOOKMARK_MIN_LINES) - (BOOKMARK_LINE_HEIGHT-(BOOKMARK_ICON_HEIGHT+BOOKMARK_LINE_OFFSET));

		/* Set the window work area extent. */

		bookmark_window_def->extent.x0 = 0;
		bookmark_window_def->extent.x1 = BOOKMARK_WINDOW_WIDTH;

		extent = LINE_BASE(bm->lines-1) - BOOKMARK_WINDOW_MARGIN;
		if (extent > -(screen - 2*sf_WINDOW_GADGET_HEIGHT))
			extent = -(screen - 2*sf_WINDOW_GADGET_HEIGHT);

		bookmark_window_def->extent.y1 = 0;
		bookmark_window_def->extent.y0 = extent;

		bookmark_pane_def->sprite_area = main_wimp_sprites;

		windows_place_as_toolbar(bookmark_window_def,
				bookmark_pane_def,
				BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_TOOLBAR_OFFSET);

		/* Set the name icon width.  Assuming that the window work area
		 * is measured from 0,0, the x1 coordinate back in from the x1
		 * work area extent by the same amount that the y1 coordinate
		 * is down from the top.
		 */

		bookmark_pane_def->icons[BOOKMARK_TB_NAME].extent.x1 =
				bookmark_pane_def->extent.x1 +
				bookmark_pane_def->icons[BOOKMARK_TB_NAME].extent.y1;

		bookmark_pane_def->icons[BOOKMARK_TB_NAME].data.indirected_text.text = bm->name;
		bookmark_pane_def->icons[BOOKMARK_TB_NAME].data.indirected_text.size = MAX_BOOKMARK_BLOCK_NAME;

		bm->window = wimp_create_window(bookmark_window_def);
		bm->toolbar = wimp_create_window(bookmark_pane_def);

		/* Register the window's event handlers. */

		event_add_window_close_event(bm->window, bookmark_close_window);
		event_add_window_redraw_event(bm->window, bookmark_redraw_window);
		event_add_window_mouse_event(bm->window, bookmark_click_handler);
		event_add_window_key_event(bm->window, bookmark_key_handler);
		event_add_window_scroll_event(bm->window, bookmark_scroll_handler);
		event_add_window_lose_caret_event(bm->window, bookmark_lose_caret_handler);
		event_add_window_gain_caret_event(bm->window, bookmark_gain_caret_handler);
		event_add_window_user_data(bm->window, bm);
		event_add_window_menu(bm->window, bookmark_menu);
		event_add_window_menu_prepare(bm->window, bookmark_menu_prepare);
		event_add_window_menu_selection(bm->window, bookmark_menu_selection);
		event_add_window_menu_close(bm->window, bookmark_menu_close);
		event_add_window_menu_warning(bm->window, bookmark_menu_warning);

		event_add_window_user_data(bm->toolbar, bm);
		event_add_window_mouse_event(bm->toolbar, bookmark_toolbar_click_handler);
		event_add_window_key_event(bm->toolbar, bookmark_toolbar_key_handler);
		event_add_window_menu(bm->toolbar, bookmark_menu);
		event_add_window_menu_prepare(bm->toolbar, bookmark_menu_prepare);
		event_add_window_menu_selection(bm->toolbar, bookmark_menu_selection);
		event_add_window_menu_warning(bm->toolbar, bookmark_menu_warning);

		/* Register for interactive help. */

		ihelp_add_window(bm->window, "Bookmark", bookmark_window_decode_help);
		ihelp_add_window(bm->toolbar, "BookmarkTB", NULL);

		/* Set up the toolbar. */

		bookmark_toolbar_set_expansion_icons(bm, NULL, NULL);
		icons_strncpy(bm->toolbar, BOOKMARK_TB_NAME, bm->name);

		/* Open the window and toolbar. */

		bookmark_set_window_columns(bm);
		windows_open(bm->window);
		windows_open_nested_as_toolbar(bm->toolbar, bm->window,
				BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_TOOLBAR_OFFSET, FALSE);

		/* Place the caret. */

		if (!bookmark_place_edit_icon(bm, 0, BOOKMARK_ICON_TITLE))
			icons_put_caret_at_end(bm->window, bm->edit_icon);
	}
}


/**
 * Close the given bookmark window and delete all of the associated data.
 *
 * \param  *close		The Wimp close data block.
 */

static void bookmark_close_window(wimp_close *close)
{
	bookmark_block			*bm;
	wimp_error_box_selection	button;
	int				shift, len;
	char				*path, *buffer;
	wimp_pointer			pointer;

	bm = bookmark_find_window(close->w);

	if (bm == NULL)
		return;

	/* Get the pointer and keyboard state. */

	shift = (osbyte1(osbyte_IN_KEY, 0xfc, 0xff) == 0xff || osbyte1(osbyte_IN_KEY, 0xf9, 0xff) == 0xff);
	if (xwimp_get_pointer_info(&pointer) != NULL)
		pointer.buttons = 0;

	/* If the file is unsaved, prompt the user. */

	if (bm->unsaved && !(pointer.buttons == wimp_CLICK_ADJUST && shift) &&
			(button = error_msgs_report_question("FileNotSaved", "FileNotSavedB")) >= 4) {
		if (button != 5)
			return;

		/* Re-grab the pointer info, as the pointer has been moved by the Message Box. */

		if (xwimp_get_pointer_info(&pointer) == NULL)
			bookmark_prepare_save_window(bm, &pointer);

		return;
	}

	/* Adjust clicks on the close icon open the parent directory. */

	if (pointer.buttons == wimp_CLICK_ADJUST && (len = strlen(bm->filename)) > 0) {
		path = (char *) malloc(len + 1);
		buffer = (char *) malloc(len + 16);

		if (path != NULL && buffer != NULL) {
			string_copy(path, bm->filename, len + 1);

			string_printf(buffer, len + 16, "%%Filer_OpenDir %s", string_find_pathname(path));
			xos_cli(buffer);
		}

		if (path != NULL)
			free(path);
		if (buffer != NULL)
			free(buffer);
	}

	/* If it was an Adjust click with Shift held down, don't close the file. */

	if (shift && pointer.buttons == wimp_CLICK_ADJUST)
		return;

	/* Delete the window and data structures. */

	wimp_delete_window(bm->window);
	wimp_delete_window(bm->toolbar);
	event_delete_window(bm->window);
	event_delete_window(bm->toolbar);

	ihelp_remove_window(bm->window);
	ihelp_remove_window(bm->toolbar);

	if (bookmarks_edit == bm)
		bookmarks_edit = NULL;

	bookmark_delete_block(bm);

}


/**
 * Callback to decode interactive help in the bookmarks window.
 *
 * \param  *buffer			Buffer to take the help token.
 * \param  w				The wimp window handle.
 * \param  i				The wimp icon handle.
 * \param  pos				The pointer coordinates.
 * \param  buttons			The mouse button state.
 */

static void bookmark_window_decode_help(char *buffer, wimp_w w, wimp_i i, os_coord pos, wimp_mouse_state buttons)
{
	int			col;
	bookmark_block		*bm;
	wimp_window_state	state;

	/* This isn't an event-lib callback, but this should still be OK. */

	bm = (bookmark_block *) event_get_window_user_data(w);

	if (bm == NULL)
		return;

	state.w = w;
	if (xwimp_get_window_state(&state) != NULL)
		return;

	col = bookmark_calculate_window_click_column(bm, &pos, &state);

	if (col != -1)
		string_printf(buffer, IHELP_INAME_LEN, "Col%d", col);
}

/**
 * Callback to handle redraw events on a bookmark window.
 *
 * \param  *redraw		The Wimp redraw event block.
 */

static void bookmark_redraw_window(wimp_draw *redraw)
{
	int			ox, oy, top, bottom, y;
	osbool			more;
	bookmark_node		*node;
	wimp_icon		*icon;
	char			buf[MAX_BOOKMARK_NUM_LEN];
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(redraw->w);

	if (bm == NULL)
		return;


	more = wimp_redraw_window(redraw);

	ox = redraw->box.x0 - redraw->xscroll;
	oy = redraw->box.y1 - redraw->yscroll;

	icon = bookmark_window_def->icons;

	while (more) {
		top = (oy - redraw->clip.y1 - BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;
		if (top < 0)
			top = 0;

		bottom = ((BOOKMARK_LINE_HEIGHT * 1.5) + oy - redraw->clip.y0
				- BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;
		if (bottom > bm->lines)
			bottom = bm->lines;

		for (y = top; y < bottom; y++) {
			bookmark_calculate_window_row_start(bm, y);
			node = bm->redraw[y].node;

			/* Plot the menu highlight. */

			if (y == bm->menu_row || y == bm->drag_row) {
				wimp_set_colour (wimp_COLOUR_VERY_DARK_GREY);
				os_plot(os_MOVE_TO, redraw->clip.x0, oy + LINE_BASE(y) + BOOKMARK_LINE_HEIGHT - 2);
				os_plot(os_PLOT_RECTANGLE + os_PLOT_TO, redraw->clip.x1, oy + LINE_BASE(y));
			}

			/* Set the icons up for plotting. */

			icon[BOOKMARK_ICON_EXPAND].extent.x0 = bm->column_pos[BOOKMARK_ICON_EXPAND];
			icon[BOOKMARK_ICON_EXPAND].extent.x1 = bm->column_pos[BOOKMARK_ICON_EXPAND] + bm->column_width[BOOKMARK_ICON_EXPAND];
			icon[BOOKMARK_ICON_EXPAND].extent.y0 = LINE_Y0(y);
			icon[BOOKMARK_ICON_EXPAND].extent.y1 = LINE_Y1(y);

			icon[BOOKMARK_ICON_TITLE].extent.x0 = bm->column_pos[BOOKMARK_ICON_TITLE];
			icon[BOOKMARK_ICON_TITLE].extent.x1 = bm->column_pos[BOOKMARK_ICON_TITLE] + bm->column_width[BOOKMARK_ICON_TITLE];
			icon[BOOKMARK_ICON_TITLE].extent.y0 = LINE_Y0(y);
			icon[BOOKMARK_ICON_TITLE].extent.y1 = LINE_Y1(y);

			icon[BOOKMARK_ICON_PAGE].extent.x0 = bm->column_pos[BOOKMARK_ICON_PAGE];
			icon[BOOKMARK_ICON_PAGE].extent.x1 = bm->column_pos[BOOKMARK_ICON_PAGE] + bm->column_width[BOOKMARK_ICON_PAGE];
			icon[BOOKMARK_ICON_PAGE].extent.y0 = LINE_Y0(y);
			icon[BOOKMARK_ICON_PAGE].extent.y1 = LINE_Y1(y);

			if (node->page > 0)
				string_printf(buf, MAX_BOOKMARK_NUM_LEN, "%d", node->page);
			else
				*buf = '\0';

			icon[BOOKMARK_ICON_TITLE].data.indirected_text.text = node->title;
			icon[BOOKMARK_ICON_PAGE].data.indirected_text.text = buf;
			icon[BOOKMARK_ICON_EXPAND].data.indirected_sprite.id = (osspriteop_id) ((node->expanded) ? "nodee" : "nodec");
			icon[BOOKMARK_ICON_EXPAND].data.indirected_sprite.area = main_wimp_sprites;
			icon[BOOKMARK_ICON_EXPAND].data.indirected_sprite.size = 6;

			/* Plot the expansion arrow for node heads, which show up
			 * as entries whose count is non-zero.
			 */

			if (node->count > 0)
				wimp_plot_icon(&(icon[BOOKMARK_ICON_EXPAND]));

			/* Plot the column icons, if they aren't replaced by a real
			 * icon for data entry.
			 */

			if (!(bm->caret_col == BOOKMARK_ICON_TITLE && bm->caret_row == y))
				wimp_plot_icon(&(icon[BOOKMARK_ICON_TITLE]));
			if (!(bm->caret_col == BOOKMARK_ICON_PAGE && bm->caret_row == y))
				wimp_plot_icon(&(icon[BOOKMARK_ICON_PAGE]));
		}

		more = wimp_get_rectangle(redraw);
	}
}


/**
 * Handle clicks in the Bookmarks windows.
 *
 * \param  *pointer		The Wimp mouse click event data.
 */

static void bookmark_click_handler(wimp_pointer *pointer)
{
	int			x, y, row, col;
	bookmark_block		*bm;
	bookmark_node		*node;
	wimp_window_state	state;
	os_error		*error;

	bm = (bookmark_block *) event_get_window_user_data(pointer->w);
	if (bm == NULL || bm->window != pointer->w)
		return;

	state.w = pointer->w;
	error = xwimp_get_window_state(&state);
	if (error != NULL)
		return;

	row = bookmark_calculate_window_click_row(bm, &(pointer->pos), &state);
	col = bookmark_calculate_window_click_column(bm, &(pointer->pos), &state);

	x = pointer->pos.x - state.visible.x0 + state.xscroll;
	y = pointer->pos.y - state.visible.y1 + state.yscroll;

	if (row != -1 && col != -1) {
		node = bm->redraw[row].node;

		if (col == BOOKMARK_ICON_EXPAND && pointer->buttons == wimp_CLICK_SELECT &&
				node->count > 0) {
			/* Handle expandion arrow clicks. */

			node->expanded = !node->expanded;
			bookmark_rebuild_data(bm);
			bookmark_force_window_redraw(bm, row, -1);
			bookmark_set_unsaved_state(bm, TRUE);
		} else if (col >= BOOKMARK_ICON_TITLE && pointer->buttons == wimp_CLICK_SELECT) {
			if (bm->drag_complete) {
				bm->drag_complete = FALSE;
			} else {
				if (!bookmark_place_edit_icon(bm, row, col))
					wimp_set_caret_position(bm->window, bm->edit_icon, x, y, -1, -1);
			}
		} else if (col >= BOOKMARK_ICON_TITLE && pointer->buttons == wimp_DRAG_SELECT) {
			bookmark_line_drag(bm, row);
		}
	}
}


/**
 * Callback handler for Wimp Key events.
 *
 * \param  *key			The associated wimp event block.
 * \return			TRUE if the keypress was handled; else FALSE.
 */

static osbool bookmark_key_handler(wimp_key *key)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(key->w);
	if (bm == NULL)
		return FALSE;

	if (bookmarks_edit != NULL && bookmarks_edit->window == key->w && bookmarks_edit->edit_icon == key->i) {
		switch (key->c) {
		case wimp_KEY_RETURN:
			bookmark_insert_edit_row_from_keypress(bm, (wimp_caret *) key);
			break;
		case wimp_KEY_DELETE:
		case wimp_KEY_BACKSPACE:
			bookmark_resync_edit_with_file();
			if (bm->caret_col == BOOKMARK_ICON_TITLE && bm->caret_row > 0 &&
					 strlen(bm->redraw[bm->caret_row].node->title) == 0) {
				bookmark_delete_edit_row(bm, bm->redraw[bm->caret_row].node);
				icons_put_caret_at_end(bm->window, bm->edit_icon);
			}
			break;
		case wimp_KEY_UP:
			bookmark_change_edit_row(bm, BOOKMARK_ABOVE, (wimp_caret *) key);
			break;
		case wimp_KEY_DOWN:
			bookmark_change_edit_row(bm, BOOKMARK_BELOW, (wimp_caret *) key);
			break;
		case wimp_KEY_TAB:
			if (bm->caret_col < BOOKMARK_WINDOW_COLUMNS - 1)
				bookmark_place_edit_icon(bm, bm->caret_row, bm->caret_col + 1);
			else if (bm->caret_row < bm->lines - 1)
				bookmark_place_edit_icon(bm, bm->caret_row + 1, BOOKMARK_ICON_TITLE);
			else {
				bookmark_insert_edit_row_from_keypress(bm, (wimp_caret *) key);
				bookmark_place_edit_icon(bm, bm->caret_row, BOOKMARK_ICON_TITLE);
			}
			if (bm->edit_icon != wimp_ICON_WINDOW)
				icons_put_caret_at_end(bm->window, bm->edit_icon);
			break;
		case wimp_KEY_TAB | wimp_KEY_SHIFT:
			if (bm->caret_col > BOOKMARK_ICON_TITLE)
				bookmark_place_edit_icon(bm, bm->caret_row, bm->caret_col - 1);
			else if (bm->caret_row > 0)
				bookmark_place_edit_icon(bm, bm->caret_row - 1, BOOKMARK_WINDOW_COLUMNS - 1);
			if (bm->edit_icon != wimp_ICON_WINDOW)
				icons_put_caret_at_end(bm->window, bm->edit_icon);
			break;
		default:
			bookmark_resync_edit_with_file();
			break;
		}
	}

	/* Pass on combinations of F12 to the rest of the Wimp.  This is ugly,
	 * but doing it "right" would require working out if the key was used
	 * by the code above -- not easy.
	 */

	if ((key->c == wimp_KEY_F12) ||
			(key->c == (wimp_KEY_F12 | wimp_KEY_SHIFT)) ||
			(key->c == (wimp_KEY_F12 | wimp_KEY_CONTROL)) ||
			(key->c == (wimp_KEY_F12 | wimp_KEY_SHIFT | wimp_KEY_CONTROL)))
		return FALSE;

	return TRUE;
}


/**
 * Callback handler for Wimp Lose Caret events.
 *
 * \param  *caret		The associated wimp event block.
 */

static void bookmark_lose_caret_handler(wimp_caret *caret)
{
	wimp_caret		current;
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(caret->w);
	if (bm == NULL)
		return;

	if (xwimp_get_caret_position(&current) != NULL)
		return;

	if (current.w != bm->window)
		icons_set_group_shaded(bm->toolbar, 1, 4,
				BOOKMARK_TB_DEMOTEG, BOOKMARK_TB_DEMOTE,
				BOOKMARK_TB_PROMOTE, BOOKMARK_TB_PROMOTEG);
}


/**
 * Callback handler for Wimp Gain Caret events.
 *
 * \param  *caret		The associated wimp event block.
 */

static void bookmark_gain_caret_handler(wimp_caret *caret)
{
	bookmark_block		*bm;
	bookmark_node		*node, *parent;

	bm = (bookmark_block *) event_get_window_user_data(caret->w);
	if (bm == NULL)
		return;

	if (bm->caret_row != -1) {
		node = bm->redraw[bm->caret_row].node;
		for (parent = bm->root; parent != NULL && parent->next != node; parent = parent->next);
	} else {
		node = NULL;
		parent = NULL;
	}

	/* Set up the toolbar icons. */

	icons_set_group_shaded(bm->toolbar, node == NULL || parent == NULL || node->level <= 1,
			2, BOOKMARK_TB_DEMOTEG, BOOKMARK_TB_DEMOTE);
	icons_set_group_shaded(bm->toolbar, node == NULL || parent == NULL || node->level > parent->level,
			2, BOOKMARK_TB_PROMOTE, BOOKMARK_TB_PROMOTEG);
}


/**
 * callback handler for Wimp Scroll events.
 *
 * \param  *scroll		The associated wimp event block.
 */

static void bookmark_scroll_handler(wimp_scroll *scroll)
{
	int		width, height, error;

	/* Add in the X scroll offset. */

	width = scroll->visible.x1 - scroll->visible.x0;

	switch (scroll->xmin) {
	case wimp_SCROLL_COLUMN_LEFT:
		scroll->xscroll -= BOOKMARK_HORIZONTAL_SCROLL;
		break;

	case wimp_SCROLL_COLUMN_RIGHT:
		scroll->xscroll += BOOKMARK_HORIZONTAL_SCROLL;
		break;

	case wimp_SCROLL_PAGE_LEFT:
		scroll->xscroll -= width;
		break;

	case wimp_SCROLL_PAGE_RIGHT:
		scroll->xscroll += width;
		break;
	}

	/* Add in the Y scroll offset. */

	height = (scroll->visible.y1 - scroll->visible.y0) - BOOKMARK_TOOLBAR_HEIGHT;

	switch (scroll->ymin) {
	case wimp_SCROLL_LINE_UP:
		scroll->yscroll += BOOKMARK_LINE_HEIGHT;
		if ((error = ((scroll->yscroll) % BOOKMARK_LINE_HEIGHT)))
			scroll->yscroll -= BOOKMARK_LINE_HEIGHT + error;
		break;

	case wimp_SCROLL_LINE_DOWN:
		scroll->yscroll -= BOOKMARK_LINE_HEIGHT;
		if ((error = ((scroll->yscroll - height) % BOOKMARK_LINE_HEIGHT)))
			scroll->yscroll -= error;
		break;

	case wimp_SCROLL_PAGE_UP:
		scroll->yscroll += height;
		if ((error = ((scroll->yscroll) % BOOKMARK_LINE_HEIGHT)))
			scroll->yscroll -= BOOKMARK_LINE_HEIGHT + error;
		break;

	case wimp_SCROLL_PAGE_DOWN:
		scroll->yscroll -= height;
		if ((error = ((scroll->yscroll - height) % BOOKMARK_LINE_HEIGHT)))
			scroll->yscroll -= error;
		break;
	}

	/* Re-open the window; it is assumed that the wimp will deal with
	 * out-of-bounds offsets for us.
	 */

	wimp_open_window ((wimp_open *) scroll);
}


/**
 * Move the caret up or down a row in a bookmark window.
 *
 * \param  *bm			The bookmark window concerned.
 * \param  direction		The direction to move (BOOKMARK_ABOVE or _BELOW).
 * \param  *caret		A block detailing the current caret position.
 */

static void bookmark_change_edit_row(bookmark_block *bm, int direction, wimp_caret *caret)
{
	int			row;

	if (bm == NULL)
		return;

	if (caret->w != bm->window || bm->caret_row == -1 || bm->caret_col == -1)
		return;

	if ((direction == BOOKMARK_ABOVE && bm->caret_row <= 0) ||
			(direction == BOOKMARK_BELOW && (bm->caret_row + 1) >= bm->lines))
		return;

	if (direction == BOOKMARK_ABOVE)
		row = bm->caret_row - 1;
	else if (direction == BOOKMARK_BELOW)
		row = bm->caret_row + 1;
	else
		row = bm->caret_row;

	if (row != bm->caret_row) {
		bookmark_place_edit_icon(bm, row, bm->caret_col);
		wimp_set_caret_position(bm->window, bm->edit_icon, caret->pos.x, LINE_Y0(row)+4, -1, -1);
	}
}


/**
 * Insert a line into the edit window based on a keypress in the edit icon.
 *
 * \param  *bm			The bookmark window concerned.
 * \param  *caret		A block detailing the current caret position.
 */

static void bookmark_insert_edit_row_from_keypress(bookmark_block *bm, wimp_caret *caret)
{
	bookmark_node		*node;
	int			direction;

	if (bm == NULL)
		return;

	if (caret->w != bm->window || bm->caret_row == -1 || bm->caret_col == -1)
		return;

	direction = (caret->index == 0 && bm->caret_col == 1 &&
			strlen(bm->redraw[bm->caret_row].node->title) > 0) ?
			BOOKMARK_ABOVE : BOOKMARK_BELOW;

	node = bm->redraw[bm->caret_row].node;

	bookmark_insert_edit_row(bm, node, direction);

	if (direction == BOOKMARK_BELOW) {
		if (!bookmark_place_edit_icon(bm, bm->caret_row+1, BOOKMARK_ICON_TITLE))
			icons_put_caret_at_end(bm->window, bm->edit_icon);

	}
}


/**
 * Insert a line into the edit window in the specified location and move the
 * caret accordingly.
 *
 * \param  *bm			The bookmark window concerned.
 * \param  *caret		A block detailing the current caret position.
 * \return			0 if a line was inserted; else 1;
 */

static int bookmark_insert_edit_row(bookmark_block *bm, bookmark_node *node, int direction)
{
	bookmark_node		*new;
	int			line, status = 1;

	if (bm == NULL || node == NULL)
		return status;

	for (line = -1; line < bm->lines && bm->redraw[line].node != node; line++);
	if (line == -1 || bm->redraw[line].node != node)
		return status;

	if (direction == BOOKMARK_ABOVE) {
		new = bookmark_insert_node(bm, node);
		if (new != NULL) {
			new->level = node->level;
			bookmark_rebuild_data(bm);
			bookmark_force_window_redraw(bm, line, -1);
			bookmark_set_unsaved_state(bm, TRUE);
			status = 0;
		}
	} else if (direction == BOOKMARK_BELOW) {
		new = bookmark_insert_node(bm, ((line+1) < bm->lines) ? bm->redraw[line+1].node : NULL);
		if (new != NULL) {
			new->level = bm->redraw[line].node->level;
			bookmark_rebuild_data(bm);
			bookmark_force_window_redraw(bm, line + 1, -1);
			bookmark_set_unsaved_state(bm, TRUE);
			status = 0;
		}
	}

	return status;
}


/**
 * Delete a line from the bookmark window.
 *
 * \param  *bm			The window block.
 * \param  *node		The node to be deleted.
 */

static void bookmark_delete_edit_row(bookmark_block *bm, bookmark_node *node)
{
	int			i, line;
	bookmark_node		*n;
	wimp_caret		caret;

	if (bm == NULL || node == NULL)
		return;

	/* If there's only one node, it can't be deleted. */

	if (bm->root == node && node->next == NULL)
		return;

	/* Find the line in the bookmark window. */

	for (line = -1; line < bm->lines && bm->redraw[line].node != node; line++);
	if (line == -1 || bm->redraw[line].node != node)
		return;

	/* If the node was a parent, then drop all the children down a level
	 * to compensate for its deletion.
	 */

	n = node->next;
	i = node->count;

	while (n != NULL && i > 0) {
		n->level--;
		n = n->next;
		i--;
	}

	/* Move the edit line if it is in the line to be deleted. */

	if (bm->caret_row == line) {
		if (xwimp_get_caret_position(&caret) != NULL)
			return;

		if (bm->root != node)
			bookmark_change_edit_row(bm, BOOKMARK_ABOVE, &caret);
		else
			bookmark_change_edit_row(bm, BOOKMARK_BELOW, &caret);
	}

	/* Delete the line and tidy up. */

	bookmark_delete_node(bm, node);
	bookmark_rebuild_data(bm);
	bookmark_force_window_redraw(bm, line, -1);
	bookmark_set_unsaved_state(bm, TRUE);
}


/**
 * Change the indentation level of the current row.
 *
 * \param  *bm			The bookmark window concerned.
 * \param  *node		The bookmark node to adjust.
 * \param  action		The action to carry out, in terms of toolbar icon numbers.
 */

static void bookmark_change_edit_row_indentation(bookmark_block *bm, bookmark_node *node, int action)
{
	bookmark_node		*parent;
	int			redraw_from, redraw_to, base, line;

	if (bm == NULL)
		return;

	/* Find the parent node.  At the end, parent points to the parent node,
	 * or is NULL if the node is the root or isn't found.
	 */

	for (parent = bm->root; parent != NULL && parent->next != node; parent = parent->next);

	/* If there is no parent, then either we have the root node (which can never have
	 * any level other than 1) or the list is broken.  Either way, get out now.
	 */

	if (parent == NULL)
		return;

	for (line = 0; line < bm->lines && bm->redraw[line].node != node; line++);
	if (line == 0 || bm->redraw[line].node != node)
		return;

	redraw_from = -1;
	redraw_to   = -1;

	switch (action) {
	case BOOKMARK_TB_PROMOTE:
		if (node->level <= parent->level) {
			node->level++;
			redraw_from = line-1;
			redraw_to = line;
		}
		break;
	case BOOKMARK_TB_PROMOTEG:
		if (node->level <= parent->level) {
			for (base = node->level; node != NULL && node->level >= base; node = node->next)
				node->level++;
			redraw_from = line-1;
		}
		break;
	case BOOKMARK_TB_DEMOTE:
		if (node->level > 1) {
			node->level--;
			while (node->next != NULL && node->next->level > (node->level + 1)) {
				node = node->next;
				node->level--;
			}
			redraw_from = line-1;
		}
		break;
	case BOOKMARK_TB_DEMOTEG:
		if (node->level > 1) {
			for (base = node->level; node != NULL && node->level >= base; node = node->next)
				node->level--;
			redraw_from = line-1;
		}
		break;
	}

	/* If the redraw limits have been set, assume an update had happened
	 * and perform the recalculation and tidying up.
	 */

	if (redraw_from != -1 || redraw_to != -1) {
		bookmark_rebuild_data(bm);
		bookmark_set_unsaved_state(bm, TRUE);
		bookmark_force_window_redraw(bm, redraw_from, redraw_to);
	}
}


/**
 * Shade or unshade the expansion and contraction icons in the toolbar if
 * expand == NULL and contract == NULL, orreturn the settings for the two actions.
 *
 * \param  *bm			The bookmark window to act on.
 * \param  *expand		A variable to return the expand setting in, or NULL.
 * \param  *contract		A variable to return the contract setting in, or NULL.
 */

static void bookmark_toolbar_set_expansion_icons(bookmark_block *bm, int *expand, int *contract)
{
	bookmark_node		*node;
	int			e=0, c=0;

	if (bm == NULL || bm->toolbar == NULL)
		return;

	for (node = bm->root; node != NULL; node = node->next) {
		if (node->count > 0 && node->expanded == FALSE)
			e = 1;

		if (node->count > 0 && node->expanded == TRUE)
			c = 1;
	}

	if (expand != NULL || contract != NULL) {
		if (expand != NULL)
			*expand = e;
		if (contract != NULL)
			*contract = c;
	} else {
		icons_set_shaded(bm->toolbar, BOOKMARK_TB_EXPAND, !e);
		icons_set_shaded(bm->toolbar, BOOKMARK_TB_CONTRACT, !c);
	}
}



/**
 * Expand or contract all the nodes in a window.
 *
 * \param  *bm			The bookmark window to alter.
 * \param  expand		TRUE to expand the tree; FALSE to contract.
 */

static void bookmark_tree_node_expansion(bookmark_block *bm, osbool expanded)
{
	bookmark_node		*node;

	if (bm == NULL)
		return;

	node = bm->root;

	while (node != NULL) {
		if (node->count > 0)
			node->expanded = expanded;

		node = node->next;
	}

	bookmark_rebuild_data(bm);
	bookmark_set_unsaved_state(bm, TRUE);
	bookmark_force_window_redraw(bm, -1, -1);
}


/**
 * Place the edit icon into a bookmark window at the specified location.
 * Note that this does not place the caret in the icon.
 *
 * \param  *bm			The bookmark window to place the icon in.
 * \param  row			The row to place the icon in.
 * \param  col			The column to place the icon in.
 * \return			0 if icon placed OK; else 1;
 */

static int bookmark_place_edit_icon(bookmark_block *bm, int row, int col)
{
	wimp_window_state		state;
	size_t				buf_len;
	wimp_icon_create		icon;

	if (bm == NULL || (bm == bookmarks_edit && bm->caret_row == row && bm->caret_col == col) ||
			row < 0 || row >= bm->lines ||
			col < BOOKMARK_ICON_TITLE || col >= BOOKMARK_WINDOW_COLUMNS)
		return 1;

	bookmark_remove_edit_icon();

	bookmark_calculate_window_row_start(bm, row);

	memcpy(&(icon.icon), &(bookmark_window_def->icons[col]), sizeof(wimp_icon));

	if (bookmarks_edit_buffer != NULL)
		free(bookmarks_edit_buffer);

	buf_len = (col == BOOKMARK_ICON_TITLE) ? MAX_BOOKMARK_LEN : MAX_BOOKMARK_NUM_LEN;

	bookmarks_edit_buffer = (char *) malloc(buf_len);

	if (bookmarks_edit_buffer == NULL)
		return 1;

	icon.icon.data.indirected_text.text = bookmarks_edit_buffer;
	icon.icon.data.indirected_text.size = buf_len;

	switch (col) {
	case BOOKMARK_ICON_TITLE:
		string_copy(bookmarks_edit_buffer, bm->redraw[row].node->title, buf_len);
		break;
	case BOOKMARK_ICON_PAGE:
		if (bm->redraw[row].node->page > 0)
			string_printf(bookmarks_edit_buffer, buf_len, "%d", bm->redraw[row].node->page);
		else
			*bookmarks_edit_buffer = '\0';
		break;
	}

	icon.w = bm->window;
	icon.icon.extent.x0 = bm->column_pos[col];
	icon.icon.extent.x1 = bm->column_pos[col] + bm->column_width[col];
	icon.icon.extent.y0 = LINE_Y0(row);
	icon.icon.extent.y1 = LINE_Y1(row);
	if (xwimp_create_icon(&icon, &(bm->edit_icon)) == NULL) {
		bookmarks_edit = bm;
		bm->caret_row = row;
		bm->caret_col = col;
	} else {
		bm->edit_icon = wimp_ICON_WINDOW;
		return 1;
	}

	/* Scroll the icon into view, if necessary. */

	state.w = bm->window;
	if (xwimp_get_window_state(&state) != NULL)
		return 0;

	if (icon.icon.extent.y1 > (state.yscroll - BOOKMARK_TOOLBAR_HEIGHT)) {
		/* The icon is off the top of the visible area. */
		state.yscroll = icon.icon.extent.y1 + BOOKMARK_TOOLBAR_HEIGHT;
		xwimp_open_window((wimp_open *) &state);
	} else if (icon.icon.extent.y0 < (state.yscroll + (state.visible.y0-state.visible.y1))) {
		/* The icon if off the bottom of the visible area. */
		state.yscroll = icon.icon.extent.y0 - (state.visible.y0-state.visible.y1);
		xwimp_open_window((wimp_open *) &state);
	}

	return 0;
}


/**
 * Remove the edit icon from a bookmark window.
 */

static void bookmark_remove_edit_icon(void)
{
	if (bookmarks_edit != NULL &&
			bookmarks_edit->edit_icon != wimp_ICON_WINDOW) {
		bookmark_resync_edit_with_file();

		wimp_delete_icon(bookmarks_edit->window, bookmarks_edit->edit_icon);
		bookmarks_edit->edit_icon = wimp_ICON_WINDOW;
		bookmarks_edit->caret_row = -1;
		bookmarks_edit->caret_col = -1;

		bookmarks_edit = NULL;
		if (bookmarks_edit_buffer != NULL) {
			free(bookmarks_edit_buffer);
			bookmarks_edit_buffer = NULL;
		}
	}
}


/**
 * Sync the edit icon contents with the underlying file.
 */

static void bookmark_resync_edit_with_file(void)
{
	int		page;

	if (bookmarks_edit == NULL ||
			bookmarks_edit->edit_icon == wimp_ICON_WINDOW)
		return;

	switch (bookmarks_edit->caret_col) {
	case BOOKMARK_ICON_TITLE:
		if (strcmp(bookmarks_edit->redraw[bookmarks_edit->caret_row].node->title, bookmarks_edit_buffer) != 0) {
			string_copy(bookmarks_edit->redraw[bookmarks_edit->caret_row].node->title, bookmarks_edit_buffer, MAX_BOOKMARK_LEN);
			bookmark_set_unsaved_state(bookmarks_edit, TRUE);
		}
		break;
	case BOOKMARK_ICON_PAGE:
		page = atoi(bookmarks_edit_buffer);

		if (page != bookmarks_edit->redraw[bookmarks_edit->caret_row].node->page) {
			bookmarks_edit->redraw[bookmarks_edit->caret_row].node->page = page;
			bookmark_set_unsaved_state(bookmarks_edit, TRUE);
		}
		break;
	}
}


/**
 * Set the title of the bookmark window.
 *
 * \param  *bm			The block to set the window title for.
 */

static void bookmark_update_window_title(bookmark_block *bm)
{
	char		*asterisk, buf[256];

	asterisk = (bm->unsaved) ? " *" : "";

	if (strlen(bm->filename) > 0) {
		string_printf(bm->window_title, MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10,
				"%s%s", bm->filename, asterisk);
	} else {
		msgs_lookup("USTitle", buf, sizeof(buf));
		string_printf(bm->window_title, MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10,
				"%s%s", buf, asterisk);
	}

	if (bm->window != NULL)
		xwimp_force_redraw_title(bm->window);
}


/**
 * Force part or all of the bookmarks window to redraw.
 *
 * \param  *bm			The window block to be redrawn.
 * \param  from			The row to start the redraw at (-1 = from start)
 * \param  to			The row to end the readraw at (-1 = to end)
 */

static void bookmark_force_window_redraw(bookmark_block *bm, int from, int to)
{
	int			x0, y0, x1, y1;
	wimp_window_info	info;
	os_error		*error;

	if (bm == NULL)
		return;

	info.w = bm->window;
	error = xwimp_get_window_info_header_only(&info);
	if (error != NULL)
		return;

	x0 = info.extent.x0;
	if (to < 0)
		y0 = info.extent.y0;
	else
		y0 = LINE_BASE(to);

	x1 = info.extent.x1;
	if (--from < 0)
		y1 = -BOOKMARK_TOOLBAR_HEIGHT;
	else
		y1 = LINE_BASE(from);

	wimp_force_redraw(bm->window, x0, y0, x1, y1);
}


/**
 * Set the vertical extent of the a bookmarks window to suit the contents.
 *
 * \param  *bm			The window block.
 */

static void bookmark_set_window_extent(bookmark_block *bm)
{
	int			screen_y, new_y, visible_y, new_y_scroll;
	wimp_window_info	info;
	os_error		*error;

	if (bm == NULL || bm->window == NULL)
		return;

	info.w = bm->window;
	error = xwimp_get_window_info_header_only(&info);
	if (error != NULL)
		return;

	screen_y = general_mode_height();

	new_y = LINE_BASE(bm->lines-1) - BOOKMARK_WINDOW_MARGIN;
	if (new_y > -(screen_y - 2*sf_WINDOW_GADGET_HEIGHT))
		new_y = -(screen_y - 2*sf_WINDOW_GADGET_HEIGHT);

	if (new_y > (info.visible.y0 - info.visible.y1))
		new_y = info.visible.y0 - info.visible.y1;

	visible_y = info.yscroll +
			(info.visible.y0 - info.visible.y1);

	if ((info.flags & wimp_WINDOW_OPEN) && (visible_y < new_y)) {
		new_y_scroll = info.yscroll;

		if (visible_y < new_y) {
			new_y_scroll = new_y - (info.visible.y0 - info.visible.y1);

			if (new_y_scroll > 0) {
				info.visible.y0 += new_y_scroll;
				info.yscroll = 0;
			} else {
				info.yscroll = new_y_scroll;
			}

			error = xwimp_open_window((wimp_open *) &info);
			if (error)
				return;
		}
	}

	info.extent.y0 = new_y;

	error = xwimp_set_extent(bm->window, &(info.extent));
}


/**
 * Set the column positions for the given bookmark window.
 *
 * \param  *bm			The bookmark window to set.
 */

static void bookmark_set_window_columns(bookmark_block *bm)
{
	int			i;
	wimp_window_info	info;
	os_error		*error;


	if (bm == NULL)
		return;

	info.w = bm->window;
	error = xwimp_get_window_info_header_only(&info);
	if (error != NULL)
		return;

	/* The position of the left-hand end of the row is set on the fly. */

	bm->column_pos[BOOKMARK_ICON_EXPAND] = 0;
	bm->column_width[BOOKMARK_ICON_EXPAND] = BOOKMARK_ICON_HEIGHT;

	bm->column_pos[BOOKMARK_ICON_TITLE] = 0;
	bm->column_width[BOOKMARK_ICON_TITLE] = 0;

	/* Set the widths of the right-hand end. */

	for (i=BOOKMARK_ICON_PAGE; i<BOOKMARK_WINDOW_COLUMNS; i++)
		bm->column_width[i] = BOOKMARK_COLUMN_WIDTH;

	/* The right-hand position is measured one gutter from the window's
	 * X extent.
	 */

	bm->column_pos[BOOKMARK_WINDOW_COLUMNS-1] = info.extent.x1 - (bm->column_width[BOOKMARK_WINDOW_COLUMNS-1] +
			(BOOKMARK_LINE_HEIGHT-(BOOKMARK_ICON_HEIGHT+BOOKMARK_LINE_OFFSET)) + BOOKMARK_WINDOW_MARGIN);

	/* Any remaining columns (if there are any) are calculated back from this. */

	for (i=BOOKMARK_WINDOW_COLUMNS-2; i>=BOOKMARK_ICON_PAGE; i--)
		bm->column_pos[i] = bm->column_pos[i+1] - (bm->column_width[i] +
			(BOOKMARK_LINE_HEIGHT-BOOKMARK_ICON_HEIGHT));

	// \TODO -- Shift any writable icon, if it is in the wrong place.
}


/**
 * Calculate the column details for the start of the given bookmark window
 * row, and update the column_pos[] and column_width[] arrays.
 *
 * \param  *bm			The window to calculate for.
 * \param  row			The row to calculate.
 */

static void bookmark_calculate_window_row_start(bookmark_block *bm, int row)
{
	bookmark_node		*node;

	if (bm == NULL || row >= bm->lines)
		return;

	node = bm->redraw[row].node;

	bm->column_pos[BOOKMARK_ICON_EXPAND] = BOOKMARK_WINDOW_MARGIN + (node->level - 1) * BOOKMARK_LINE_HEIGHT;
	bm->column_pos[BOOKMARK_ICON_TITLE] = BOOKMARK_WINDOW_MARGIN + node->level * BOOKMARK_LINE_HEIGHT;
	bm->column_width[BOOKMARK_ICON_TITLE] = bm->column_pos[BOOKMARK_ICON_PAGE] - bm->column_pos[BOOKMARK_ICON_TITLE] -
			(BOOKMARK_LINE_HEIGHT-BOOKMARK_ICON_HEIGHT);
}

/**
 * Calculate the row that the mouse was clicked over in a bookmark window.
 *
 * \param  *bm			The bookmark block for the window.
 * \param  *pointer		The Wimp pointer data.
 * \param  *state		The bookmark window state.
 * \return			The row (from 0) or -1 if none.
 */

static int bookmark_calculate_window_click_row(bookmark_block *bm, os_coord *pos, wimp_window_state *state)
{
	int			y, row, row_y_pos;

	if (bm == NULL || state == NULL)
		return -1;

	y = state->visible.y1 - pos->y - state->yscroll;

	row = (y - BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_WINDOW_MARGIN) / BOOKMARK_LINE_HEIGHT;
	row_y_pos = ((y - BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_WINDOW_MARGIN) % BOOKMARK_LINE_HEIGHT);

	if (row >= bm->lines ||
			row_y_pos < (BOOKMARK_LINE_HEIGHT - (BOOKMARK_LINE_OFFSET + BOOKMARK_ICON_HEIGHT)) ||
			row_y_pos > (BOOKMARK_LINE_HEIGHT - BOOKMARK_LINE_OFFSET))
		row = -1;

	return row;
}


/**
 * Calculate the column that the mouse was clicked over in a bookmark window.
 *
 * \param  *bm			The bookmark block for the window.
 * \param  *pointer		The Wimp pointer data.
 * \param  *state		The bookmark window state.
 * \return			The column (from 0) or -1 if none.
 */

static int bookmark_calculate_window_click_column(bookmark_block *bm, os_coord *pos, wimp_window_state *state)
{
	int			i, x, row, col;

	if (bm == NULL || state == NULL)
		return -1;

	row = bookmark_calculate_window_click_row(bm, pos, state);

	if (row < 0 || row >= bm->lines)
		return -1;

	x = pos->x - state->visible.x0 + state->xscroll;

	bookmark_calculate_window_row_start(bm, row);
	col = -1;

	for (i=0; i<BOOKMARK_WINDOW_COLUMNS; i++)
		if (x >= bm->column_pos[i] && x <= bm->column_pos[i]+bm->column_width[i]) {
			col = i;
			break;
		}

	/* If there isn't an expansion icon on this row, set the column to -1
	 * if we're over that area.
	 */

	if (col == BOOKMARK_ICON_EXPAND && bm->redraw[row].node->count == 0)
		col = -1;

	return col;
}


/**
 * Start dragging a line in the bookmark window.
 */

static void bookmark_line_drag(bookmark_block *bm, int line)
{
	wimp_window_state	window;
	wimp_auto_scroll_info	auto_scroll;
	wimp_drag		drag;
	int			ox, oy;

	if (bm == NULL || line < 0 || line >= bm->lines)
		return;

	/* Get the basic information about the window. */

	window.w = bm->window;
	if (xwimp_get_window_state(&window) != NULL)
		return;

	ox = window.visible.x0 - window.xscroll;
	oy = window.visible.y1 - window.yscroll;

	/* Set up the drag parameters. */

	drag.w = bm->window;
	drag.type = wimp_DRAG_USER_FIXED;

	drag.initial.x0 = ox;
	drag.initial.y0 = oy + LINE_Y0(line);
	drag.initial.x1 = ox + (window.visible.x1 - window.visible.x0);
	drag.initial.y1 = oy + LINE_Y1(line);

	drag.bbox.x0 = window.visible.x0;
	drag.bbox.y0 = window.visible.y0;
	drag.bbox.x1 = window.visible.x1;
	drag.bbox.y1 = window.visible.y1;

	/* Read CMOS RAM to see if solid drags are required. */
/*
	dragging_sprite = ((osbyte2(osbyte_READ_CMOS, osbyte_CONFIGURE_DRAG_ASPRITE, 0) &
			osbyte_CONFIGURE_DRAG_ASPRITE_MASK) != 0);
*/
//	if (0 && dragging_sprite) /* This is never used, though it could be... */
/*		dragasprite_start(dragasprite_HPOS_CENTRE | dragasprite_VPOS_CENTRE | dragasprite_NO_BOUND |
				dragasprite_BOUND_POINTER | dragasprite_DROP_SHADOW, wimpspriteop_AREA,
				"", &(drag.initial), &(drag.bbox));
	else
*/		wimp_drag_box(&drag);

	/* Initialise the autoscroll. */

	if (xos_swi_number_from_string("Wimp_AutoScroll", NULL) == NULL) {
		auto_scroll.w = bm->window;
		auto_scroll.pause_zone_sizes.x0 = 0;
		auto_scroll.pause_zone_sizes.y0 = AUTO_SCROLL_MARGIN;
		auto_scroll.pause_zone_sizes.x1 = 0;
		auto_scroll.pause_zone_sizes.y1 = AUTO_SCROLL_MARGIN;
		auto_scroll.pause_duration = 0;
		auto_scroll.state_change = (void *) 1;

		wimp_auto_scroll (wimp_AUTO_SCROLL_ENABLE_VERTICAL, &auto_scroll);
	}

//	dragging_start_line = line;
	bm->drag_row = line;
	bookmark_force_window_redraw(bm, line, line);
	event_set_drag_handler(bookmark_terminate_line_drag, NULL, (void *) bm);
}


/**
 * Callback handler for bookmark window drag termination.
 *
 * \param  *drag		The Wimp poll block from termination.
 * \param  *data		The bookmark block associated with the drag.
 */

static void bookmark_terminate_line_drag(wimp_dragged *drag, void *data)
{
	bookmark_block		*bm;
	bookmark_node		*node, *target, *n;
	wimp_pointer		pointer;
	wimp_window_state	state;
	int			row, i;

	/* Terminate the drag and end the autoscroll. */

	if (xos_swi_number_from_string("Wimp_AutoScroll", NULL) == NULL)
		wimp_auto_scroll(0, NULL);

//	if (dragging_sprite)
//		dragasprite_stop();

	/* Having stopped the drag, validate the data block. */

	bm = (bookmark_block *) data;
	if (bm == NULL)
		return;

	/* Get the line at which the drag ended. */


	wimp_get_pointer_info(&pointer);

	state.w = bm->window;
	wimp_get_window_state(&state);

	/* Caculate the window row manually, as we don't care about 'icons' but
	 * the position *between* them.  Row is the row before which the dragged
	 * item was dropped.
	 */

	row = state.visible.y1 - pointer.pos.y - state.yscroll;
	row = (row - BOOKMARK_TOOLBAR_HEIGHT + (BOOKMARK_LINE_HEIGHT / 2)) / BOOKMARK_LINE_HEIGHT;
	if (row > bm->lines)
		row = bm->lines;

	/* If there is a move to do, carry it out. */

	if (row != bm->drag_row && row != bm->drag_row + 1) {
		node = bm->redraw[bm->drag_row].node;

		if (row < bm->lines)
			target = bm->redraw[row].node;
		else
			target = NULL;

		/* If the node was a parent, then drop all the children down a level
		 * to compensate for its deletion.
		 */

		n = node->next;
		i = node->count;

		while (n != NULL && i > 0) {
			n->level--;
			n = n->next;
			i--;
		}

		/* Unlink the node. */

		if (bm->root == node) {
			bm->root = node->next;
		} else {
			for (n = bm->root; n != NULL && n->next != node; n = n->next);
			if (n != NULL)
				n->next = node->next;
		}

		/* Link the node back in its new home. */

		if (bm->root == target) {
			node->next = bm->root;
			bm->root = node;
		} else {
			for (n = bm->root; n != NULL && n->next != target; n = n->next);
			if (n != NULL) {
				node->next = n->next;
				n->next = node;
			}
		}

		/* Fix the indentation.  At the end of the list, take the previous node's
		 * level, otherwise take the following node's.
		 */

		if (target == NULL)
			node->level = n->level;
		else
			node->level = target->level;

		bookmark_rebuild_data(bm);
		bookmark_set_unsaved_state(bm, TRUE);

		bookmark_force_window_redraw(bm, (row < bm->drag_row) ? row-1 : bm->drag_row-1, -1);
	} else {
		bookmark_force_window_redraw(bm, bm->drag_row, bm->drag_row);
	}

	bm->drag_row = -1;
	bm->drag_complete = TRUE;
}


/* ****************************************************************************
 * Bookmark Toolbar Handling
 * ****************************************************************************/

/**
 * Handle clicks in the Bookmarks toolbars.
 *
 * \param  *pointer		The Wimp mouse click event data.
 */

static void bookmark_toolbar_click_handler(wimp_pointer *pointer)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(pointer->w);
	if (bm == NULL)
		return;

	switch (pointer->i) {
	case BOOKMARK_TB_SAVE:
		
		if (pointer->buttons == wimp_CLICK_SELECT)
			bookmark_prepare_save_window(bm, pointer);
		else if (pointer->buttons == wimp_CLICK_ADJUST)
			bookmark_start_direct_menu_save(bm);
		break;
	case BOOKMARK_TB_PROMOTE:
	case BOOKMARK_TB_PROMOTEG:
	case BOOKMARK_TB_DEMOTE:
	case BOOKMARK_TB_DEMOTEG:
		bookmark_change_edit_row_indentation(bm, bm->redraw[bm->caret_row].node, (int) pointer->i);
		break;
	case BOOKMARK_TB_EXPAND:
		bookmark_tree_node_expansion(bm, TRUE);
		break;
	case BOOKMARK_TB_CONTRACT:
		bookmark_tree_node_expansion(bm, FALSE);
		break;
	}
}


/**
 * Callback handler for Wimp Key events in the toolbar.
 *
 * \param  *key			The associated wimp event block.
 * \return			TRUE if the keypress was handled; else FALSE.
 */

static osbool bookmark_toolbar_key_handler(wimp_key *key)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(key->w);
	if (bm == NULL)
		return FALSE;

	switch (key->c) {
	case wimp_KEY_RETURN:
	case wimp_KEY_TAB:
		bookmark_place_edit_icon(bm, 0, BOOKMARK_ICON_TITLE);
		break;
	default:
		if (key->i == BOOKMARK_TB_NAME)
			bookmark_set_unsaved_state(bm, TRUE);
		break;
	}

	/* Pass on combinations of F12 to the rest of the Wimp.  This is ugly,
	 * but doing it "right" would require working out if the key was used
	 * by the code above -- not easy.
	 */

	if ((key->c == wimp_KEY_F12) ||
			(key->c == (wimp_KEY_F12 | wimp_KEY_SHIFT)) ||
			(key->c == (wimp_KEY_F12 | wimp_KEY_CONTROL)) ||
			(key->c == (wimp_KEY_F12 | wimp_KEY_SHIFT | wimp_KEY_CONTROL)))
		return FALSE;

	return TRUE;
}


/* ****************************************************************************
 * Bookmark Window Menu Handling
 * ****************************************************************************/

/**
 * Prepare the bookmark window menu for (re)-opening.
 *
 * \param  w			The handle of the menu's parent window.
 * \param  *menu		Pointer to the menu being opened.
 * \param  *pointer		Pointer to the Wimp Pointer event block.
 */

static void bookmark_menu_prepare(wimp_w w, wimp_menu *menu, wimp_pointer *pointer)
{
	int			row = -1, expand, contract;
	bookmark_block		*bm;
	bookmark_node		*node, *parent;
	wimp_window_state	state;
	os_error		*error;

	bm = (bookmark_block *) event_get_window_user_data(w);
	if (bm == NULL || menu != bookmark_menu)
		return;

	if (bm->menu_row != -1) {
		row = bm->menu_row;
	} else if (pointer != NULL) {
		state.w = pointer->w;
		error = xwimp_get_window_state(&state);
		if (error == NULL)
			row = bookmark_calculate_window_click_row(bm, &(pointer->pos), &state);
		else
			row = -1;
	}

	/* Set up the row highlight in the bookmark window and find the node and
	 * parent node blocks.
	 */

	if (row != -1) {
		node = bm->redraw[row].node;
		for (parent = bm->root; parent != NULL && parent->next != node; parent = parent->next);

		bm->menu_row = row;
		bookmark_force_window_redraw(bm, bm->menu_row, bm->menu_row);
	} else {
		node = NULL;
		parent = NULL;
	}

	/* Set up the menu itself. */

	saveas_initialise_dialogue(bookmark_saveas_file, (strlen(bm->filename) > 0) ? bm->filename : NULL, "BMFileName", NULL, FALSE, FALSE, bm);

	bookmark_toolbar_set_expansion_icons(bm, &expand, &contract);

	menus_shade_entry(bookmark_menu, BOOKMARK_MENU_LEVEL, row == -1);
	menus_shade_entry(bookmark_menu, BOOKMARK_MENU_INSERT, row == -1);
	menus_shade_entry(bookmark_menu, BOOKMARK_MENU_DELETE,
			row == -1 || (bm->root == node && node->next == NULL));

	menus_shade_entry(bookmark_menu_view, BOOKMARK_MENU_VIEW_EXPAND, !expand);
	menus_shade_entry(bookmark_menu_view, BOOKMARK_MENU_VIEW_CONTRACT, !contract);

	menus_shade_entry(bookmark_menu_level, BOOKMARK_MENU_LEVEL_PROMOTE,
			row == -1 || node == NULL || parent == NULL || node->level > parent->level);
	menus_shade_entry(bookmark_menu_level, BOOKMARK_MENU_LEVEL_PROMOTEG,
			row == -1 || node == NULL || parent == NULL || node->level > parent->level);
	menus_shade_entry(bookmark_menu_level, BOOKMARK_MENU_LEVEL_DEMOTE,
			row == -1 || node == NULL || parent == NULL || node->level <= 1);
	menus_shade_entry(bookmark_menu_level, BOOKMARK_MENU_LEVEL_DEMOTEG,
			row == -1 || node == NULL || parent == NULL || node->level <= 1);

	menus_shade_entry(bookmark_menu_insert, BOOKMARK_MENU_INSERT_ABOVE, row == -1);
	menus_shade_entry(bookmark_menu_insert, BOOKMARK_MENU_INSERT_BELOW, row == -1);
}


/**
 * Handle submenu warnings for the bookmark window menu.
 *
 * \param  w			The window to which the menu belongs.
 * \param  *menu		Pointer to the menu itself.
 * \param  *warning		Pointer to the Wimp menu warning block.
 */

static void bookmark_menu_warning(wimp_w w, wimp_menu *menu, wimp_message_menu_warning *warning)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(w);
	if (bm == NULL)
		return;

	switch (warning->selection.items[0]) {
	case BOOKMARK_MENU_FILE:
		switch (warning->selection.items[1]) {
			case BOOKMARK_MENU_FILE_INFO:
				bookmark_prepare_file_info_window(bm);
				break;
			case BOOKMARK_MENU_FILE_SAVE:
				saveas_prepare_dialogue(bookmark_saveas_file);
				break;
		}
		break;
	}

	wimp_create_sub_menu(warning->sub_menu, warning->pos.x, warning->pos.y);
}


/**
 * Handle selections from the bookmark window menu.
 *
 * \param  w			The window to which the menu belongs.
 * \param  *menu		Pointer to the menu itself.
 * \param  *selection		Pointer to the Wimp menu selction block.
 */

static void bookmark_menu_selection(wimp_w w, wimp_menu *menu, wimp_selection *selection)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(w);
	if (bm == NULL)
		return;

	switch (selection->items[0]) {
	case BOOKMARK_MENU_FILE:
		switch (selection->items[1]) {
		case BOOKMARK_MENU_FILE_SAVE:
			bookmark_start_direct_menu_save(bm);
			break;
		}
		break;
	case BOOKMARK_MENU_VIEW:
		switch (selection->items[1]) {
		case BOOKMARK_MENU_VIEW_EXPAND:
			bookmark_tree_node_expansion(bm, TRUE);
			break;
		case BOOKMARK_MENU_VIEW_CONTRACT:
			bookmark_tree_node_expansion(bm, FALSE);
			break;
		}
		break;
	case BOOKMARK_MENU_LEVEL:
		switch (selection->items[1]) {
		case BOOKMARK_MENU_LEVEL_PROMOTE:
			bookmark_change_edit_row_indentation(bm, bm->redraw[bm->menu_row].node, BOOKMARK_TB_PROMOTE);
			break;
		case BOOKMARK_MENU_LEVEL_PROMOTEG:
			bookmark_change_edit_row_indentation(bm, bm->redraw[bm->menu_row].node, BOOKMARK_TB_PROMOTEG);
			break;
		case BOOKMARK_MENU_LEVEL_DEMOTE:
			bookmark_change_edit_row_indentation(bm, bm->redraw[bm->menu_row].node, BOOKMARK_TB_DEMOTE);
			break;
		case BOOKMARK_MENU_LEVEL_DEMOTEG:
			bookmark_change_edit_row_indentation(bm, bm->redraw[bm->menu_row].node, BOOKMARK_TB_DEMOTEG);
			break;
		}
		break;
	case BOOKMARK_MENU_INSERT:
		switch (selection->items[1]) {
		case BOOKMARK_MENU_INSERT_ABOVE:
			if (!bookmark_insert_edit_row(bm, bm->redraw[bm->menu_row].node, BOOKMARK_ABOVE))
				bm->menu_row++;
			break;
		case BOOKMARK_MENU_INSERT_BELOW:
			bookmark_insert_edit_row(bm, bm->redraw[bm->menu_row].node, BOOKMARK_BELOW);
			break;
		}
		break;
	case BOOKMARK_MENU_DELETE:
		bookmark_delete_edit_row(bm, bm->redraw[bm->menu_row].node);
	}
}


/**
 * Handle the bookmark window menu closing.
 */

static void bookmark_menu_close(wimp_w w, wimp_menu *menu)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(w);
	if (bm == NULL)
		return;

	if (bm->menu_row != -1) {
		bookmark_force_window_redraw(bm, bm->menu_row, bm->menu_row);
		bm->menu_row = -1;
	}
}


/* ****************************************************************************
 * File Info Dialogue Handling
 * ****************************************************************************/

/**
 * Prepare the contents of the File Info dialogue.
 *
 * \param  *bm			The bookmark file to be used.
 */

static void bookmark_prepare_file_info_window(bookmark_block *bm)
{
	if (bm == NULL)
		return;

	icons_strncpy(bookmark_window_fileinfo, FILEINFO_ICON_NAME, bm->name);

	if (strlen(bm->filename) > 0) {
		icons_strncpy(bookmark_window_fileinfo, FILEINFO_ICON_LOCATION, bm->filename);
		territory_convert_standard_date_and_time (territory_CURRENT, (os_date_and_time const *) bm->datestamp,
				icons_get_indirected_text_addr(bookmark_window_fileinfo, FILEINFO_ICON_DATE),
				icons_get_indirected_text_length(bookmark_window_fileinfo, FILEINFO_ICON_DATE));
	} else {
		icons_msgs_lookup(bookmark_window_fileinfo, FILEINFO_ICON_LOCATION, "Unsaved");
		icons_msgs_lookup(bookmark_window_fileinfo, FILEINFO_ICON_DATE, "Unsaved");
	}

	icons_msgs_lookup(bookmark_window_fileinfo, FILEINFO_ICON_MODIFIED, (bm->unsaved) ? "Yes" : "No");
}


/* ****************************************************************************
 * SaveAs Dialogue Handling
 * ****************************************************************************/

/**
 * Prepare the contents of the SaveAs window for the bookmark window.
 *
 * \param  *bm			The bookmark file to which the window applies.
 */

static void bookmark_prepare_save_window(bookmark_block *bm, wimp_pointer *pointer)
{
	if (bm == NULL || pointer == NULL)
		return;

	saveas_initialise_dialogue(bookmark_saveas_file, (strlen(bm->filename) > 0) ? bm->filename : NULL, "BMFileName", NULL, FALSE, FALSE, bm);
	saveas_prepare_dialogue(bookmark_saveas_file);
	saveas_open_dialogue(bookmark_saveas_file, pointer);
}


/**
 * Process a click File->Save menu selection, or Adjust-click on the
 * Save toolbar icon.
 *
 * \param  *bm		The bookmark block to be saved.
 */

static void bookmark_start_direct_menu_save(bookmark_block *bm)
{
	wimp_pointer	pointer;

	if (bm == NULL)
		return;

	if (strlen(bm->filename) > 0) {
		bookmarks_save_file(bm->filename, FALSE, bm);
	} else {
		wimp_get_pointer_info(&pointer);

		saveas_initialise_dialogue(bookmark_saveas_file, (strlen(bm->filename) > 0) ? bm->filename : NULL, "BMFileName", NULL, FALSE, FALSE, bm);
		saveas_prepare_dialogue(bookmark_saveas_file);
		saveas_open_dialogue(bookmark_saveas_file, &pointer);
	}
}


/**
 * Save a bookmark file from memory to disc.
 *
 * \param *filename	The filename to save to.
 * \param selection	DataXFer selection flag (unused).
 * \param *data		Pointer to the bookmark block to save (void* to allow
 *			this function to be used as a data load callback for DataXFer).
 * \return		TRUE on success; FALSE on failure.
 */

static osbool bookmarks_save_file(char *filename, osbool selection, void *data)
{
	FILE			*out;
	bookmark_node		*node;
	bits			load, exec;
	bookmark_block		*bm = data;

	if (bm == NULL)
		return FALSE;

	out = fopen(filename, "w");
	if (out == NULL) {
		// \TODO -- Add an error report here.
		return FALSE;
	}

	/* Write the file header. */

	fprintf(out, "# PrintPDF File\n# Written by PrintPDF\n\n");
	fprintf(out, "Format: 1.00\n\n");

	/* Write the bookmarks section. */

	fprintf(out, "[Bookmarks]\n");
	fprintf(out, "Name: %s\n", bm->name);

	node = bm->root;

	while (node != NULL) {
		fprintf(out, "@: %s\n", node->title);
		fprintf(out, "Page: %d\n", node->page);
		if (node->yoffset >= 0)
			fprintf(out, "YOffset: %d\n", node->yoffset);
		if (node->level > 1)
			fprintf(out, "Level: %d\n", node->level);
		if (!node->expanded)
			fprintf(out, "Expanded: %s\n", config_return_opt_string(node->expanded));

		node = node->next;
	}

	fclose(out);
	osfile_set_type(filename, (bits) dataxfer_TYPE_PRINTPDF);

	osfile_read_stamped(filename, &load, &exec, NULL, NULL, NULL);
	bm->datestamp[0] = exec & 0xff;
	bm->datestamp[1] = (exec & 0xff00) >> 8;
	bm->datestamp[2] = (exec & 0xff0000) >> 16;
	bm->datestamp[3] = (exec & 0xff000000) >> 24;
	bm->datestamp[4] = load & 0xff;

	string_copy(bm->filename, filename, MAX_BOOKMARK_FILENAME);
	bm->unsaved = TRUE; /* Force the titlebar to update, even if the file was already saved. */
	bookmark_set_unsaved_state(bm, FALSE);

	return TRUE;
}


/**
 * Load a bookmark file into memory, storing the data it contains in a new
 * bookmark_block structure and opening a bookmark window.
 *
 * \param  *filename	The file to load.
 * \return		The bookmark block containing the file; else NULL.
 */

bookmark_block *bookmarks_load_file(char *filename)
{
	FILE			*in;
	bookmark_block		*block;
	bookmark_node		*current, *new;
	int			bookmarks = 0, unknown_data = 0, unknown_format = 0, version = 100;
	char			section[BOOKMARK_FILE_LINE_LEN], token[BOOKMARK_FILE_LINE_LEN], value[BOOKMARK_FILE_LINE_LEN];
	bits			load, exec;
	enum config_read_status	result;


	block = bookmark_create_block();

	if (block == NULL) {
		// \TODO -- Add an error report here.
		return NULL;
	}

	osfile_read_stamped(filename, &load, &exec, NULL, NULL, NULL);
	block->datestamp[0] = exec & 0xff;
	block->datestamp[1] = (exec & 0xff00) >> 8;
	block->datestamp[2] = (exec & 0xff0000) >> 16;
	block->datestamp[3] = (exec & 0xff000000) >> 24;
	block->datestamp[4] = load & 0xff;

	in = fopen(filename, "r");

	if (in == NULL) {
		// \TODO -- Add an error report here.
		bookmark_delete_block(block);
		return NULL;
	}

	hourglass_on();

	string_copy(block->filename, filename, MAX_BOOKMARK_FILENAME);
	bookmark_update_window_title(block);

	/* Read the nodes into a linear linked list, ignoring for the time
	 * being any levels.
	 */

	current = NULL;

	while ((result = config_read_token_pair(in, token, value, section)) != sf_CONFIG_READ_EOF) {
		if (result == sf_CONFIG_READ_NEW_SECTION)
			bookmarks = (string_nocase_strcmp(section, "Bookmarks") == 0);

		if (bookmarks) {
			if (string_nocase_strcmp(token, "Name") == 0) {
				string_copy(block->name, value, MAX_BOOKMARK_BLOCK_NAME);
			} else if (string_nocase_strcmp(token, "@") == 0) {
				new = (bookmark_node *) malloc(sizeof(bookmark_node));

				if (new != NULL) {
					string_copy(new->title, value, MAX_BOOKMARK_LEN);

					new->page = 0;
					new->yoffset = -1;
					new->expanded = TRUE;
					new->level = 1;
					new->count = 0;

					new->next = NULL;

					if (current == NULL)
						block->root = new;
					else
						current->next = new;

					current = new;
				}

			} else if (string_nocase_strcmp(token, "Page") == 0) {
				if (current != NULL)
					current->page = atoi(value);
			} else if (string_nocase_strcmp(token, "YOffset") == 0) {
				if (current != NULL && version > 100) /* Version 1.00 files probably have buggy YOffets that we can't use anyway. */
					current->yoffset = atoi(value);
			} else if (string_nocase_strcmp(token, "Level") == 0) {
				if (current != NULL)
					current->level = atoi(value);
			} else if (string_nocase_strcmp(token, "Expanded") == 0) {
				if (current != NULL)
					current->expanded = config_read_opt_string(value);
			} else {
				unknown_data = 1;
			}
		} else {
			if (string_nocase_strcmp(token, "Format") == 0) {
				version = string_convert_version_number(value);
				if (version != 100)
					unknown_format = 1;
			} else {
				unknown_data = 1;
			}
		}
	}

	hourglass_off();

	fclose (in);

	if (unknown_data)
		error_msgs_report_info ("UnknownFileData");

	if (unknown_format)
		error_msgs_report_info ("UnknownFileFormat");

	bookmark_rebuild_data(block);
	bookmark_open_window(block);

	return block;
}


/**
 * Recalculate the details of a bookmark block.
 *
 * \param *bm		Pointer to the block to recalculate.
 */

static void bookmark_rebuild_data(bookmark_block *bm)
{
	bookmark_node		*node, *n, *edit_node, *edit_parent;;
	int			count, i, edit_col;
	wimp_caret		caret;

	if (bm == NULL)
		return;

	/* If there is an edit icon in the window, find its node and position
	 * before deleting it.
	 */

	if (bm == bookmarks_edit) {
		if (xwimp_get_caret_position(&caret) == NULL) {
			edit_node = bm->redraw[bm->caret_row].node;
			edit_col = bm->caret_col;
		} else {
			edit_node = NULL;
			edit_col = -1;
		}
		bookmark_remove_edit_icon();
	} else {
		edit_node = NULL;
		edit_col = -1;
	}

	/* Find the number of entries in the block list, and reallocate the
	 * redraw array if it has changed.
	 */

	count = 0;
	for (node = bm->root; node != NULL; node = node->next)
		count++;

	if (count != bm->nodes) {
		if (bm->redraw != NULL)
			free(bm->redraw);

		bm->redraw = (bookmark_redraw *) malloc(count * sizeof(bookmark_redraw));
		bm->nodes = count;
	}

	/* Scan through the list, building up the tree blocks. */

	for (node = bm->root; node != NULL; node = node->next) {
		count = 0;

		for (n = node->next; (n != NULL) && (n->level > node->level); n = n->next)
			count++;

		node->count = count;
	}

	edit_parent = NULL;

	if (bm->redraw != NULL) {
		count = 0;

		for (node = bm->root; node != NULL; node = node->next) {
			bm->redraw[count].node = node;
			bm->redraw[count].selected = 0;

			/* Skip past any contracted lines and identify if the
			 * edited node is one of them.
			 */

			if (!node->expanded) {
				n = node;

				for (i = node->count; node != NULL && i > 0; node = node->next) {
					i--;
					if (node->next == edit_node)
						edit_parent = n;
				}
			}

			count++;
		}

		bm->lines = count;
	}

	bookmark_set_window_extent(bm);
	bookmark_toolbar_set_expansion_icons(bm, NULL, NULL);

	/* If there was an edit icon, find its new home and restore it. */

	if (edit_node != NULL) {
		if (edit_parent != NULL)
			edit_node = edit_parent;

		for (i = 0; i < bm->lines && bm->redraw[i].node != edit_node; i++);

		if (i < bm->lines) {
			bookmark_place_edit_icon(bm, i, edit_col);
			wimp_set_caret_position(bm->window, bm->edit_icon, caret.pos.x,
			(-(i+1) * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET + 4 - BOOKMARK_TOOLBAR_HEIGHT), -1, -1);
		}
	}
}


/**
 * Output PDFMark data related to the associated bookmarks parameters file.
 *
 * \param  *pdfmark_file	The file to write to.
 * \param  *params		The parameter block to use.
 */

void bookmarks_write_pdfmark_out_file(FILE *pdfmark_file, bookmark_params *params)
{
	bookmark_node		*node;
	char			buffer[MAX_BOOKMARK_LEN * 4];

	params->bookmarks = bookmark_find_block(params->bookmarks);

	if (pdfmark_file != NULL && bookmark_data_available(params))
		for (node = params->bookmarks->root; node != NULL; node = node->next) {
			if (strlen(node->title) > 0 && node->page > 0) {
				fprintf(pdfmark_file, "[");

				if (node->count > 0)
					fprintf(pdfmark_file, " /Count %d", (node->expanded) ? node->count : -node->count);

				fprintf(pdfmark_file, " /Page %d", node->page);

				if (node->yoffset >= 0)
					fprintf(pdfmark_file, " /View [/XYZ 0 %.4f null]", ((double) node->yoffset / 1000));

				fprintf(pdfmark_file, " /Title (%s) /OUT pdfmark\n",
						convert_to_pdf_doc_encoding(buffer, node->title, MAX_BOOKMARK_LEN * 4));
			}
		}
}

