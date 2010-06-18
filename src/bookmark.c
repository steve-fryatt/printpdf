/* PrintPDF - bookmark.c
 *
 * (C) Stephen Fryatt, 2010
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
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/windows.h"
#include "sflib/debug.h"
#include "sflib/event.h"
#include "sflib/errors.h"

/* Application header files */

#include "bookmark.h"

#include "convert.h"
#include "dataxfer.h"
#include "menus.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "windows.h"


/* ****************************************************************************
 * Function prototypes
 * ****************************************************************************/

/* PDF Creation Interface */

wimp_menu	*build_bookmark_menu(bookmark_params *params);

/* Bookmark Block Management */

bookmark_block	*create_bookmark_block(void);
void		delete_bookmark_block(bookmark_block *bookmark);
bookmark_node	*insert_bookmark_node(bookmark_block *bm, bookmark_node *before);
void		set_bookmark_unsaved_state(bookmark_block *bm, int unsaved);

bookmark_block	*find_bookmark_window(wimp_w window);
bookmark_block	*find_bookmark_toolbar(wimp_w window);
bookmark_block	*find_bookmark_name(char *name);
bookmark_block	*find_bookmark_block(bookmark_block *block);

/* Bookmark Window Handling */

void		open_bookmark_window(bookmark_block *bm);
void		close_bookmark_window(wimp_close *close);
void		redraw_bookmark_window(wimp_draw *redraw);
void		bookmark_click_handler(wimp_pointer *pointer);
void		bookmark_key_handler(wimp_key *key);
void		bookmark_change_edit_row(bookmark_block *bm, int direction, wimp_caret *caret);
void		bookmark_insert_edit_row(bookmark_block *bm, wimp_caret *caret);
int		bookmark_place_edit_icon(bookmark_block *bm, int row, int col);
void		bookmark_remove_edit_icon(void);
void		bookmark_resync_edit_with_file(void);
void		update_bookmark_window_title(bookmark_block *bm);
void		force_bookmark_window_redraw(bookmark_block *bm, int from, int to);
void		set_bookmark_window_extent(bookmark_block *bm);
void		set_bookmark_window_columns(bookmark_block *bm);
void		calculate_bookmark_window_row_start(bookmark_block *bm, int row);

/* Bookmark Window Menu Handling */

void		bookmark_menu_prepare(wimp_pointer *pointer, wimp_menu *menu);
void		bookmark_menu_selection(wimp_w w, wimp_menu *menu, wimp_selection *selection);
void		bookmark_menu_warning(wimp_w w, wimp_menu *menu, wimp_message_menu_warning *warning);

/* File Info Dialogue Handling */

void prepare_file_info_window(bookmark_block *bm);

/* SaveAs Dialogue Handling */

void		prepare_bookmark_save_window(bookmark_block *bm);
void		bookmark_save_as_click(wimp_pointer *pointer);
void		bookmark_save_as_keypress(wimp_key *key);
int		start_direct_dialog_save(void);
void		start_direct_menu_save(bookmark_block *bm);

/* Bookmark Data Processing */

void		rebuild_bookmark_data(bookmark_block *bm);


/* ****************************************************************************
 * Global variables
 * ****************************************************************************/

/* Pointer to bookmark window data list. */

static bookmark_block	*bookmarks_list = NULL;
static int		untitled_number = 1;

static bookmark_block	*bookmarks_edit = NULL;
static char		*bookmarks_edit_buffer = NULL;

static wimp_menu	*bookmarks_menu = NULL;
static bookmark_block	**bookmarks_menu_links = NULL;
static int		bookmarks_menu_size = 0;


/* ****************************************************************************
 * Bookmarks System Initialisation and Termination
 * ****************************************************************************/

/**
 * Initialise the bookmarks system.
 */

void initialise_bookmarks(void)
{
}

/**
 * Terminate the bookmarks system, freeing up the resources used.
 */

void terminate_bookmarks(void)
{
	/* Work through the bookmarks list, deleting everything. */

	while (bookmarks_list != NULL)
		delete_bookmark_block(bookmarks_list);
}


/* ****************************************************************************
 * PDF Creation Interface
 * ****************************************************************************/

/**
 * Initialise a bookmarks settings block with default parameters.
 *
 * \param  *params		The parameter block to initialise.
 */

void initialise_bookmark_settings(bookmark_params *params)
{
	params->bookmarks = NULL;
}


/**
 * Create and open the bookmark list pop-up menu.
 *
 * \param  *params		The associated bookmark parameters.
 * \param  *pointer		The pointer details of the click.
 * \param  window		The window in which the pop-up resides.
 * \param  icon			The icon to which the pop-up is attached.
 */

void open_bookmark_menu(bookmark_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon)
{
	wimp_menu		*menu;

	extern global_menus	menus;

	params->bookmarks = find_bookmark_block(params->bookmarks);

	menu = build_bookmark_menu(params);

	if (menu != NULL)
		menus.menu_up = create_popup_menu(menu, pointer);
}


/**
 * Handle selection events from the bookmarks pop-up menu.
 *
 * \param  *params		The associated bookmarks parameters.
 * \param  *selection		The Wimp Menu selection block.
 */

void process_bookmark_menu(bookmark_params *params, wimp_selection *selection)
{
	params->bookmarks = find_bookmark_block(params->bookmarks);

	if (selection->items[0] >= 0 && selection->items[0] < bookmarks_menu_size)
		params->bookmarks = bookmarks_menu_links[selection->items[0]];

	build_bookmark_menu(params);
}


/**
 * Build the bookmarks pop-up menu used for selecting a bookmark set to use, and
 * register it with the global_menus structure.
 *
 * \param  *params		Bookamrks param block to use to set ticks.
 * \return			The menu block, or NULL.
 */

wimp_menu *build_bookmark_menu(bookmark_params *params)
{
	int			count, item, width;
	bookmark_block		*bm;

	extern global_menus	menus;


	params->bookmarks = find_bookmark_block(params->bookmarks);

	/* Count up the entries; we need a menu length one greater, to allow
	 * for the 'None' entry.
	 */

	for (bm = bookmarks_list, count = 1; bm != NULL; bm = bm->next)
		count++;

	/* (Re-)Allocate memory for the menu and block links. */

	if (count != bookmarks_menu_size) {
		if (bookmarks_menu != NULL)
			free(bookmarks_menu);
		if (bookmarks_menu_links != NULL)
			free(bookmarks_menu_links);

		bookmarks_menu = (wimp_menu *) malloc(sizeof (wimp_menu_base) + sizeof (wimp_menu_entry) * count);
		bookmarks_menu_links = (bookmark_block **) malloc(sizeof(bookmark_block) * count);
		bookmarks_menu_size = count;

		menus.bookmarks_list = bookmarks_menu;
	}

	/* If we got the memory, build the menu and links. */

	if (bookmarks_menu != NULL && bookmarks_menu_links != NULL) {
		msgs_lookup("BMListMenu", bookmarks_menu->title_data.text, 12);

		item = 0;

		bookmarks_menu->entries[item].menu_flags = (count > 1) ? wimp_MENU_SEPARATE : 0;
		bookmarks_menu->entries[item].sub_menu = (wimp_menu *) -1;
		bookmarks_menu->entries[item].icon_flags = wimp_ICON_TEXT | wimp_ICON_FILLED |
				wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT |
				wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT;
		msgs_lookup("None", bookmarks_menu->entries[item].data.text, 12);
		bookmarks_menu->entries[item].data.indirected_text.validation = NULL;
		bookmarks_menu->entries[item].data.indirected_text.size = PARAM_MENU_LEN;

		bookmarks_menu_links[item] = NULL;

		if (params->bookmarks == NULL)
			bookmarks_menu->entries[item].menu_flags |= wimp_MENU_TICKED;

		width = strlen(bookmarks_menu->entries[item].data.text);

		for (bm = bookmarks_list; bm != NULL; bm = bm->next) {
			item++;

			bookmarks_menu->entries[item].menu_flags = 0;
			bookmarks_menu->entries[item].sub_menu = (wimp_menu *) -1;
			bookmarks_menu->entries[item].icon_flags = wimp_ICON_TEXT | wimp_ICON_FILLED |
					wimp_ICON_INDIRECTED | wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT |
					wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT;
			bookmarks_menu->entries[item].data.indirected_text.text = bm->name;
			bookmarks_menu->entries[item].data.indirected_text.validation = NULL;
			bookmarks_menu->entries[item].data.indirected_text.size = MAX_BOOKMARK_BLOCK_NAME;

			bookmarks_menu_links[item] = bm;

			if (params->bookmarks == bm)
				bookmarks_menu->entries[item].menu_flags |= wimp_MENU_TICKED;

			if (strlen(bm->name) > width)
				width = strlen(bm->name);
		}

		bookmarks_menu->entries[item].menu_flags |= wimp_MENU_LAST;

		bookmarks_menu->title_fg = wimp_COLOUR_BLACK;
		bookmarks_menu->title_bg = wimp_COLOUR_LIGHT_GREY;
		bookmarks_menu->work_fg = wimp_COLOUR_BLACK;
		bookmarks_menu->work_bg = wimp_COLOUR_WHITE;

		bookmarks_menu->width = (width + 1) * 16;
		bookmarks_menu->height = 44;
		bookmarks_menu->gap = 0;
	}

	return bookmarks_menu;
}


/**
 * Fill the Bookmark info field based on the supplied parameters.
 *
 * \param  window		The window the field is in.
 * \param  icon			The icon for the field.
 * \param  *params		The parameters to use.
 */

void fill_bookmark_field(wimp_w window, wimp_i icon, bookmark_params *params)
{
	params->bookmarks = find_bookmark_block(params->bookmarks);

	if (params == NULL || params->bookmarks == NULL) {
		msgs_lookup ("None", indirected_icon_text (window, icon), MAX_BOOKMARK_FIELD_LEN);
	} else {
		strncpy(indirected_icon_text (window, icon), params->bookmarks->name, MAX_BOOKMARK_FIELD_LEN);
		if (strlen(params->bookmarks->name) >= MAX_BOOKMARK_FIELD_LEN)
			strcpy(indirected_icon_text (window, icon) + MAX_BOOKMARK_FIELD_LEN - 4, "...");
	}

	wimp_set_icon_state (window, icon, 0, 0);
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
	params->bookmarks = find_bookmark_block(params->bookmarks);

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
		params->bookmarks = find_bookmark_block(params->bookmarks);

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

bookmark_block *create_bookmark_block(void)
{
	bookmark_block		*new;
	char			name[MAX_BOOKMARK_BLOCK_NAME], number[10];

	new = (bookmark_block *) malloc(sizeof(bookmark_block));

	if (new != NULL) {
		do {
			snprintf(number, 10, "%d", untitled_number++);
			msgs_param_lookup("UntBM", name, MAX_BOOKMARK_BLOCK_NAME,
					number, NULL, NULL, NULL);
		} while (find_bookmark_name(name) != NULL);
		strncpy(new->name, name, MAX_BOOKMARK_BLOCK_NAME);
		strncpy(new->filename, "", MAX_BOOKMARK_FILENAME);
		new->unsaved = 0;
		new->window = NULL;
		new->toolbar = NULL;
		new->redraw = NULL;
		new->root = NULL;
		new->lines = 0;
		new->nodes = 0;
		new->caret_row = -1;
		new->caret_col = -1;
		new->edit_icon = wimp_ICON_WINDOW;

		update_bookmark_window_title(new);

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

void delete_bookmark_block(bookmark_block *bookmark)
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

bookmark_node *insert_bookmark_node(bookmark_block *bm, bookmark_node *before)
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

	strncpy(new->title, "", MAX_BOOKMARK_LEN);

	new->page = 0;
	new->yoffset = 0;
	new->expanded = 1;
	new->level = 1;
	new->count = 0;
	new->next = NULL;

	new->next = *node;
	*node = new;

	return new;
}


/**
 * Set the 'unsaved' status of a bookmark block.
 *
 * \param  *bm			The block to update.
 * \param  unsaved		The unsaved status (1 = unsaved; 0 = saved).
 */

void set_bookmark_unsaved_state(bookmark_block *bm, int unsaved)
{
	if (unsaved != bm->unsaved) {
		bm->unsaved = unsaved;
		update_bookmark_window_title(bm);
	}
}


/**
 * Find a bookmark block by its window handle.
 *
 * \param  window		The handle of the window.
 * \return 			The block address, or NULL if it wasn't found.
 */

bookmark_block *find_bookmark_window(wimp_w window)
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

bookmark_block *find_bookmark_toolbar(wimp_w window)
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

bookmark_block *find_bookmark_name(char *name)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && strcmp_no_case(bm->name, name) != 0)
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

bookmark_block *find_bookmark_block(bookmark_block *block)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && bm != block)
		bm = bm->next;

	return bm;
}


/* ****************************************************************************
 * Bookmark Window Handling
 * ****************************************************************************/

/**
 * Given a pointer click, create a new bookmark window.
 *
 * \param  *pointer		The details of the mouse click.
 */

void create_new_bookmark_window(wimp_pointer *pointer)
{
	bookmark_block		*new;

	new = create_bookmark_block();

	if (new != NULL)
		insert_bookmark_node(new, NULL);
		rebuild_bookmark_data(new);
		open_bookmark_window(new);
}

/**
 * Create and open a window for a bookmark block.
 *
 * \param *bm		The block to open the window for.
 */

void open_bookmark_window(bookmark_block *bm)
{
	extern global_windows	windows;
	extern global_menus	menus;

	if (bm != NULL && bm->window == NULL && bm->toolbar == NULL) {
		windows.bookmark_window_def->title_data.indirected_text.text = bm->window_title;
		windows.bookmark_window_def->title_data.indirected_text.size = MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10;

		windows.bookmark_window_def->extent.y0 = -((((bm->lines > BOOKMARK_MIN_LINES) ? bm->lines : BOOKMARK_MIN_LINES) * BOOKMARK_LINE_HEIGHT)
				+ BOOKMARK_TOOLBAR_HEIGHT + (BOOKMARK_LINE_HEIGHT-(BOOKMARK_ICON_HEIGHT+BOOKMARK_LINE_OFFSET)));

		place_window_as_toolbar(windows.bookmark_window_def,
				windows.bookmark_pane_def,
				BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_TOOLBAR_OFFSET);
		bm->window = wimp_create_window(windows.bookmark_window_def);
		bm->toolbar = wimp_create_window(windows.bookmark_pane_def);

		/* Register the window's event handlers. */

		event_add_window_close_event(bm->window, close_bookmark_window);
		event_add_window_redraw_event(bm->window, redraw_bookmark_window);
		event_add_window_mouse_event(bm->window, bookmark_click_handler);
		event_add_window_key_event(bm->window, bookmark_key_handler);
		event_add_window_user_data(bm->window, bm);
		event_add_window_menu(bm->window, menus.bookmarks,
				bookmark_menu_prepare, bookmark_menu_selection,
				NULL, bookmark_menu_warning);

		event_add_window_user_data(bm->toolbar, bm);
		event_add_window_menu(bm->toolbar, menus.bookmarks,
				bookmark_menu_prepare, bookmark_menu_selection,
				NULL, bookmark_menu_warning);

		/* Open the window and toolbar. */

		set_bookmark_window_columns(bm);
		open_window(bm->window);
		open_window_nested_as_toolbar(bm->toolbar, bm->window,
				BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_TOOLBAR_OFFSET);
	}
}


/**
 * Close the given bookmark window and delete all of the associated data.
 *
 * \param  *close		The Wimp close data block.
 */

void close_bookmark_window(wimp_close *close)
{
	bookmark_block		*bm;

	bm = find_bookmark_window(close->w);

	if (bm != NULL) {
		wimp_delete_window(bm->window);
		wimp_delete_window(bm->toolbar);
		event_delete_window(bm->window);

		if (bookmarks_edit == bm)
			bookmarks_edit = NULL;

		delete_bookmark_block(bm);
	}
}


/**
 * Callback to handle redraw events on a bookmark window.
 *
 * \param  *redraw		The Wimp redraw event block.
 */

void redraw_bookmark_window(wimp_draw *redraw)
{
	int			ox, oy, top, bottom, y;
	osbool			more;
	bookmark_node		*node;
	wimp_icon		*icon;
	char			buf[MAX_BOOKMARK_NUM_LEN];
	bookmark_block		*bm;

	extern global_windows	windows;
	extern osspriteop_area	*wimp_sprites;


	bm = (bookmark_block *) event_get_window_user_data(redraw->w);

	if (bm == NULL)
		return;


	more = wimp_redraw_window(redraw);

	ox = redraw->box.x0 - redraw->xscroll;
	oy = redraw->box.y1 - redraw->yscroll;

	icon = windows.bookmark_window_def->icons;

	while (more) {
		top = (oy - redraw->clip.y1 - BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;
		if (top < 0)
			top = 0;

		bottom = ((BOOKMARK_LINE_HEIGHT * 1.5) + oy - redraw->clip.y0
				- BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;
		if (bottom > bm->lines)
			bottom = bm->lines;

		for (y = top; y < bottom; y++) {
			calculate_bookmark_window_row_start(bm, y);
			node = bm->redraw[y].node;

			icon[BOOKMARK_ICON_EXPAND].extent.x0 = bm->column_pos[BOOKMARK_ICON_EXPAND];
			icon[BOOKMARK_ICON_EXPAND].extent.x1 = bm->column_pos[BOOKMARK_ICON_EXPAND] + bm->column_width[BOOKMARK_ICON_EXPAND];
			icon[BOOKMARK_ICON_EXPAND].extent.y0 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			icon[BOOKMARK_ICON_EXPAND].extent.y1 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);

			icon[BOOKMARK_ICON_TITLE].extent.x0 = bm->column_pos[BOOKMARK_ICON_TITLE];
			icon[BOOKMARK_ICON_TITLE].extent.x1 = bm->column_pos[BOOKMARK_ICON_TITLE] + bm->column_width[BOOKMARK_ICON_TITLE];
			icon[BOOKMARK_ICON_TITLE].extent.y0 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			icon[BOOKMARK_ICON_TITLE].extent.y1 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);

			icon[BOOKMARK_ICON_PAGE].extent.x0 = bm->column_pos[BOOKMARK_ICON_PAGE];
			icon[BOOKMARK_ICON_PAGE].extent.x1 = bm->column_pos[BOOKMARK_ICON_PAGE] + bm->column_width[BOOKMARK_ICON_PAGE];
			icon[BOOKMARK_ICON_PAGE].extent.y0 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			icon[BOOKMARK_ICON_PAGE].extent.y1 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);

			if (node->page > 0)
				snprintf(buf, MAX_BOOKMARK_NUM_LEN, "%d", node->page);
			else
				*buf = '\0';

			icon[BOOKMARK_ICON_TITLE].data.indirected_text.text = node->title;
			icon[BOOKMARK_ICON_PAGE].data.indirected_text.text = buf;
			icon[BOOKMARK_ICON_EXPAND].data.indirected_sprite.id = (osspriteop_id) ((node->expanded) ? "nodee" : "nodec");
			icon[BOOKMARK_ICON_EXPAND].data.indirected_sprite.area = wimp_sprites;
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

void bookmark_click_handler(wimp_pointer *pointer)
{
	int			i, x, y, row, col, row_x_pos, row_y_pos;
	bookmark_block		*bm;
	bookmark_node		*node;
	wimp_window_state	state;
	os_error		*error;

	bm = (bookmark_block *) event_get_window_user_data(pointer->w);
	if (bm == NULL)
		return;

	state.w = pointer->w;
	error = xwimp_get_window_state(&state);
	if (error != NULL)
		return;

	x = pointer->pos.x - state.visible.x0 + state.xscroll;
	y = state.visible.y1 - pointer->pos.y + state.yscroll;

	row = (y - BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;
	row_y_pos = ((y - BOOKMARK_TOOLBAR_HEIGHT) % BOOKMARK_LINE_HEIGHT)
			 - BOOKMARK_LINE_OFFSET;

	if (row >= bm->lines ||
			row_y_pos < (BOOKMARK_LINE_HEIGHT - (BOOKMARK_LINE_OFFSET + BOOKMARK_ICON_HEIGHT)) ||
			row_y_pos > (BOOKMARK_LINE_HEIGHT - BOOKMARK_LINE_OFFSET))
		row = -1;

	if (row != -1) {
		calculate_bookmark_window_row_start(bm, row);
		node = bm->redraw[row].node;
		row_x_pos = x - node->level * BOOKMARK_LINE_HEIGHT;
		col = -1;

		for (i=0; i<BOOKMARK_WINDOW_COLUMNS; i++)
			if (x >= bm->column_pos[i] && x <= bm->column_pos[i]+bm->column_width[i]) {
				col = i;
				break;
			}

		if (col == BOOKMARK_ICON_EXPAND && pointer->buttons == wimp_CLICK_SELECT &&
				node->count > 0) {
			/* Handle expandion arrow clicks. */

			node->expanded = !node->expanded;
			rebuild_bookmark_data(bm);
			force_bookmark_window_redraw(bm, row, -1);
			set_bookmark_unsaved_state(bm, 1);
		} else if (col >= BOOKMARK_ICON_TITLE && pointer->buttons == wimp_CLICK_SELECT) {
			if (!bookmark_place_edit_icon(bm, row, col))
				wimp_set_caret_position(bm->window, bm->edit_icon, x, y, -1, -1);
		}
	}
}


/**
 * Callback handler for Wimp Key events.
 *
 * \param  *key			The associated wimp event block.
 */

void bookmark_key_handler(wimp_key *key)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(key->w);
	if (bm == NULL)
		return;

	if (bookmarks_edit != NULL && bookmarks_edit->window == key->w && bookmarks_edit->edit_icon == key->i) {
		switch (key->c) {
		case wimp_KEY_RETURN:
			bookmark_insert_edit_row(bm, (wimp_caret *) key);
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
				bookmark_insert_edit_row(bm, (wimp_caret *) key);
				bookmark_place_edit_icon(bm, bm->caret_row, BOOKMARK_ICON_TITLE);
			}
			if (bm->edit_icon != wimp_ICON_WINDOW)
				put_caret_at_end(bm->window, bm->edit_icon);
			break;
		case wimp_KEY_TAB | wimp_KEY_SHIFT:
			if (bm->caret_col > BOOKMARK_ICON_TITLE)
				bookmark_place_edit_icon(bm, bm->caret_row, bm->caret_col - 1);
			else if (bm->caret_row > 0)
				bookmark_place_edit_icon(bm, bm->caret_row - 1, BOOKMARK_WINDOW_COLUMNS - 1);
			if (bm->edit_icon != wimp_ICON_WINDOW)
				put_caret_at_end(bm->window, bm->edit_icon);
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
		wimp_process_key(key->c);
}


/**
 * Move the caret up or down a row in a bookmark window.
 *
 * \param  *bm			The bookmark window concerned.
 * \param  direction		The direction to move (BOOKMARK_ABOVE or _BELOW).
 * \param  *caret		A block detailing the current caret position.
 */

void bookmark_change_edit_row(bookmark_block *bm, int direction, wimp_caret *caret)
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
		wimp_set_caret_position(bm->window, bm->edit_icon, caret->pos.x,
				(-(row+1) * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET + 4 - BOOKMARK_TOOLBAR_HEIGHT), -1, -1);
	}
}


/**
 * Insert a line into the edit window in the specified location and move the
 * caret accordingly.
 *
 * \param  *bm			The bookmark window concerned.
 * \param  *caret		A block detailing the current caret position.
 */

void bookmark_insert_edit_row(bookmark_block *bm, wimp_caret *caret)
{
	bookmark_node		*new;
	int			direction;

	if (bm == NULL)
		return;

	if (caret->w != bm->window || bm->caret_row == -1 || bm->caret_col == -1)
		return;

	direction = (caret->index == 0 && bm->caret_col == 1 &&
			strlen(bm->redraw[bm->caret_row].node->title) > 0) ?
			BOOKMARK_ABOVE : BOOKMARK_BELOW;

	if (direction == BOOKMARK_ABOVE) {
		new = insert_bookmark_node(bm, bm->redraw[bm->caret_row].node);
		if (new != NULL) {
			if (bm->caret_row == 0)
				new->level = 1;
			else
				new->level = bm->redraw[bm->caret_row-1].node->level;
			rebuild_bookmark_data(bm);
			force_bookmark_window_redraw(bm, bm->caret_row - 1, -1);
			set_bookmark_unsaved_state(bm, 1);
		}
	} else {
		new = insert_bookmark_node(bm, ((bm->caret_row + 1) < bm->lines) ? bm->redraw[bm->caret_row+1].node : NULL);
		if (new != NULL) {
			new->level = bm->redraw[bm->caret_row].node->level;
			rebuild_bookmark_data(bm);
			force_bookmark_window_redraw(bm, bm->caret_row, -1);
			bookmark_change_edit_row(bm, BOOKMARK_BELOW, caret);
			set_bookmark_unsaved_state(bm, 1);
		}
	}
}


/**
 * Place the edit icon into a bookmark window at the specified location.
 * Note that this does not place the caret in the icon, so care must be taken
 * to ensure that this happens before Wimp_Poll is called again to avoid a
 * LoseCaret event being received.
 *
 * \param  *bm			The bookmark window to place the icon in.
 * \param  row			The row to place the icon in.
 * \param  col			The column to place the icon in.
 * \return			0 if icon placed OK; else 1;
 */

int bookmark_place_edit_icon(bookmark_block *bm, int row, int col)
{
	size_t				buf_len;
	wimp_icon_create		icon;
	extern global_windows		windows;

	if (bm == NULL || (bm == bookmarks_edit && bm->caret_row == row && bm->caret_col == col))
		return 1;

	bookmark_remove_edit_icon();

	calculate_bookmark_window_row_start(bm, row);

	memcpy(&(icon.icon), &(windows.bookmark_window_def->icons[col]), sizeof(wimp_icon));

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
		strncpy(bookmarks_edit_buffer, bm->redraw[row].node->title, buf_len);
		break;
	case BOOKMARK_ICON_PAGE:
		if (bm->redraw[row].node->page > 0)
			snprintf(bookmarks_edit_buffer, buf_len, "%d", bm->redraw[row].node->page);
		else
			*bookmarks_edit_buffer = '\0';
		break;
	}

	icon.w = bm->window;
	icon.icon.extent.x0 = bm->column_pos[col];
	icon.icon.extent.x1 = bm->column_pos[col] + bm->column_width[col];
	icon.icon.extent.y0 = (-(row+1) * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET - BOOKMARK_TOOLBAR_HEIGHT);
	icon.icon.extent.y1 = (-(row+1) * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET - BOOKMARK_TOOLBAR_HEIGHT + BOOKMARK_ICON_HEIGHT);
	if (xwimp_create_icon(&icon, &(bm->edit_icon)) == NULL) {
		bookmarks_edit = bm;
		bm->caret_row = row;
		bm->caret_col = col;
	} else {
		bm->edit_icon = wimp_ICON_WINDOW;
	}

	return (bm->edit_icon == wimp_ICON_WINDOW) ? 1 : 0;
}


/**
 * Remove the edit icon from a bookmark window.
 */

void bookmark_remove_edit_icon(void)
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

void bookmark_resync_edit_with_file(void)
{
	int		page;

	if (bookmarks_edit == NULL ||
			bookmarks_edit->edit_icon == wimp_ICON_WINDOW)
		return;

	switch (bookmarks_edit->caret_col) {
	case BOOKMARK_ICON_TITLE:
		if (strcmp(bookmarks_edit->redraw[bookmarks_edit->caret_row].node->title, bookmarks_edit_buffer) != 0) {
			strncpy(bookmarks_edit->redraw[bookmarks_edit->caret_row].node->title, bookmarks_edit_buffer, MAX_BOOKMARK_LEN);
			set_bookmark_unsaved_state(bookmarks_edit, 1);
		}
		break;
	case BOOKMARK_ICON_PAGE:
		page = atoi(bookmarks_edit_buffer);

		if (page != bookmarks_edit->redraw[bookmarks_edit->caret_row].node->page) {
			bookmarks_edit->redraw[bookmarks_edit->caret_row].node->page = page;
			set_bookmark_unsaved_state(bookmarks_edit, 1);
		}
		break;
	}
}


/**
 * Set the title of the bookmark window.
 *
 * \param  *bm			The block to set the window title for.
 */

void update_bookmark_window_title(bookmark_block *bm)
{
	char		*asterisk;

	asterisk = (bm->unsaved) ? " *" : "";

	if (strlen(bm->filename) > 0)
		snprintf(bm->window_title, MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10,
				"%s (%s)%s", bm->filename, bm->name, asterisk);
	else
		snprintf(bm->window_title, MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10,
				"%s%s", bm->name, asterisk);

	xwimp_force_redraw_title(bm->window);
}


/**
 * Force part or all of the bookmarks window to redraw.
 *
 * \param  *bm			The window block to be redrawn.
 * \param  from			The row to start the redraw at (-1 = from start)
 * \param  to			The row to end the readraw at (-1 = to end)
 */

void force_bookmark_window_redraw(bookmark_block *bm, int from, int to)
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
		y0 = -((BOOKMARK_LINE_HEIGHT * to) + BOOKMARK_TOOLBAR_HEIGHT);

	x1 = info.extent.x1;
	if (--from < 0)
		y1 = -BOOKMARK_TOOLBAR_HEIGHT;
	else
		y1 = -((BOOKMARK_LINE_HEIGHT * from) + BOOKMARK_TOOLBAR_HEIGHT);

	wimp_force_redraw(bm->window, x0, y0, x1, y1);
}


/**
 * Set the vertical extent of the a bookmarks window to suit the contents.
 *
 * \param  *bm			The window block.
 */

void set_bookmark_window_extent(bookmark_block *bm)
{
	int			new_y, visible_y, new_y_scroll;
	wimp_window_info	info;
	os_error		*error;

	if (bm == NULL && bm->window != NULL)
		return;

	info.w = bm->window;
	error = xwimp_get_window_info_header_only(&info);
	if (error != NULL)
		return;

	new_y = -((((bm->lines > BOOKMARK_MIN_LINES) ? bm->lines : BOOKMARK_MIN_LINES) * BOOKMARK_LINE_HEIGHT)
			+ BOOKMARK_TOOLBAR_HEIGHT + (BOOKMARK_LINE_HEIGHT-(BOOKMARK_ICON_HEIGHT+BOOKMARK_LINE_OFFSET)));

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

void set_bookmark_window_columns(bookmark_block *bm)
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
			(BOOKMARK_LINE_HEIGHT-(BOOKMARK_ICON_HEIGHT+BOOKMARK_LINE_OFFSET)));

	/* Any remaining columns (if there are any) are calculated back from this. */

	for (i=BOOKMARK_WINDOW_COLUMNS-2; i>=BOOKMARK_ICON_PAGE; i--)
		bm->column_pos[i] = bm->column_pos[i+1] - (bm->column_width[i] +
			(BOOKMARK_LINE_HEIGHT-(BOOKMARK_ICON_HEIGHT+BOOKMARK_LINE_OFFSET)));

	// \TODO -- Shift any writable icon, if it is in the wrong place.
}


/**
 * Calculate the column details for the start of the given bookmark window
 * row, and update the column_pos[] and column_width[] arrays.
 *
 * \param  *bm			The window to calculate for.
 * \param  row			The row to calculate.
 */

void calculate_bookmark_window_row_start(bookmark_block *bm, int row)
{
	bookmark_node		*node;

	if (bm == NULL || row >= bm->lines)
		return;

	node = bm->redraw[row].node;

	bm->column_pos[BOOKMARK_ICON_EXPAND] = (node->level - 1) * BOOKMARK_LINE_HEIGHT;
	bm->column_pos[BOOKMARK_ICON_TITLE] = node->level * BOOKMARK_LINE_HEIGHT;
	bm->column_width[BOOKMARK_ICON_TITLE] = bm->column_pos[BOOKMARK_ICON_PAGE] - bm->column_pos[BOOKMARK_ICON_TITLE] -
			(BOOKMARK_LINE_HEIGHT-(BOOKMARK_ICON_HEIGHT+BOOKMARK_LINE_OFFSET));
}

/* ****************************************************************************
 * Bookmark Window Menu Handling
 * ****************************************************************************/

/**
 * Prepare the bookmark window menu for (re)-opening.
 *
 * \param  *pointer		Pointer to the Wimp Pointer event block.
 * \param  *menu		Pointer to the menu being opened.
 */

void bookmark_menu_prepare(wimp_pointer *pointer, wimp_menu *menu)
{
//	debug_printf("Bookmark menu prepare");
}


/**
 * Handle submenu warnings for the bookmark window menu.
 *
 * \param  w			The window to which the menu belongs.
 * \param  *menu		Pointer to the menu itself.
 * \param  *warning		Pointer to the Wimp menu warning block.
 */

void bookmark_menu_warning(wimp_w w, wimp_menu *menu, wimp_message_menu_warning *warning)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(w);
	if (bm == NULL)
		return;

	switch (warning->selection.items[0]) {
	case BOOKMARK_MENU_FILE:
		switch (warning->selection.items[1]) {
			case BOOKMARK_MENU_FILE_INFO:
				prepare_file_info_window(bm);
				break;
			case BOOKMARK_MENU_FILE_SAVE:
				prepare_bookmark_save_window(bm);
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

void bookmark_menu_selection(wimp_w w, wimp_menu *menu, wimp_selection *selection)
{
	bookmark_block		*bm;

	bm = (bookmark_block *) event_get_window_user_data(w);
	if (bm == NULL)
		return;

	switch (selection->items[0]) {
	case BOOKMARK_MENU_FILE:
		switch (selection->items[1]) {
			case BOOKMARK_MENU_FILE_SAVE:
				start_direct_menu_save(bm);
				break;
		}
		break;
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

void prepare_file_info_window(bookmark_block *bm)
{
	extern global_windows		windows;

	if (bm == NULL)
		return;

	strncpy(indirected_icon_text(windows.file_info, FILEINFO_ICON_NAME), bm->name,
			indirected_icon_length(windows.file_info, FILEINFO_ICON_NAME));

	if (strlen(bm->filename) > 0)
		strncpy(indirected_icon_text(windows.file_info, FILEINFO_ICON_LOCATION), bm->filename,
				indirected_icon_length(windows.file_info, FILEINFO_ICON_LOCATION));
	else
		msgs_lookup("Unsaved", indirected_icon_text(windows.file_info, FILEINFO_ICON_LOCATION),
				indirected_icon_length(windows.file_info, FILEINFO_ICON_LOCATION));

	msgs_lookup((bm->unsaved) ? "Yes" : "No", indirected_icon_text(windows.file_info, FILEINFO_ICON_MODIFIED),
				indirected_icon_length(windows.file_info, FILEINFO_ICON_MODIFIED));
}


/* ****************************************************************************
 * SaveAs Dialogue Handling
 * ****************************************************************************/

/**
 * Prepare the contents of the SaveAs window for the bookmark window.
 *
 * \param  *bm			The bookmark file to which the window applies.
 */

void prepare_bookmark_save_window(bookmark_block *bm)
{
	extern global_windows		windows;

	if (strlen(bm->filename) > 0)
		strncpy(indirected_icon_text(windows.save_as, SAVEAS_ICON_NAME),
				bm->filename, MAX_BOOKMARK_FILENAME);
	else
		msgs_lookup("BMFileName", indirected_icon_text(windows.save_as,
				SAVEAS_ICON_NAME), MAX_BOOKMARK_FILENAME);

	snprintf(indirected_icon_text(windows.save_as, SAVEAS_ICON_FILE),
			MAX_BOOKMARK_FILESPR, "file_%3x", PRINTPDF_FILE_TYPE);

	event_add_window_user_data(windows.save_as, bm);
	event_add_window_mouse_event(windows.save_as, bookmark_save_as_click);
	event_add_window_key_event(windows.save_as, bookmark_save_as_keypress);
}


/**
 * Callback to handle clicks in the SaveAs window.
 *
 * \param  *pointer	The relevant Wimp pointer data block.
 */

void bookmark_save_as_click(wimp_pointer *pointer)
{
	switch ((int) pointer->i)
	{
	case SAVEAS_ICON_FILE:
		if (pointer->buttons == wimp_DRAG_SELECT)
			start_save_window_drag(DRAG_SAVE_SAVEAS);
		break;
	case SAVEAS_ICON_OK:
		if (start_direct_dialog_save())
			wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;
	case SAVEAS_ICON_CANCEL:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;
	}
}


/**
 * Process keypresses in the SaveAs window.
 */

void bookmark_save_as_keypress(wimp_key *key)
{
	switch (key->c) {
	case wimp_KEY_RETURN:
		if (start_direct_dialog_save())
			wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;
	case wimp_KEY_ESCAPE:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;
	}
}

/**
 * Start a direct save from the SaveAs dialog.
 *
 * \return		1 if the save completed; else 0.
 */

int start_direct_dialog_save(void)
{
	bookmark_block		*bm;
	char			*filename;
	extern global_windows	windows;

	bm = event_get_window_user_data(windows.save_as);
	if (bm == NULL)
		return 0;

	filename = indirected_icon_text(windows.save_as, SAVEAS_ICON_NAME);

	if (strchr(filename, '.') == NULL)
		wimp_msgtrans_info_report("DragSave");
	else {
		save_bookmark_file(bm, filename);
		return 1;
	}

	return 0;
}

/**
 * Process a click File->Save menu selection, or Adjust-click on the
 * Save toolbar icon.
 *
 * \param  *bm		The bookmark block to be saved.
 */

void start_direct_menu_save(bookmark_block *bm)
{
	wimp_pointer		pointer;

	extern global_windows	windows;

	if (strlen(bm->filename) > 0) {
		save_bookmark_file(bm, bm->filename);
	} else {
		wimp_get_pointer_info(&pointer);
		prepare_bookmark_save_window(bm);
		create_standard_menu((wimp_menu *) windows.save_as, &pointer);
	}
}


/**
 * Callback to terminate a dragging filesave.
 *
 * \param  *filename	The filename to save under.
 * \return		0 if the save completes OK; else non-0.
 */

int drag_end_save_saveas(char *filename)
{
	extern global_windows		windows;
	bookmark_block			*bm;

	bm = event_get_window_user_data(windows.save_as);

	if (bm == NULL)
		return 0;

	save_bookmark_file(bm, filename);
	wimp_create_menu((wimp_menu *) -1, 0, 0);

	return 0;
}


/* ****************************************************************************
 * Bookmark Data Processing
 * ****************************************************************************/

/**
 * Save a bookmark file from memory to disc.
 *
 * \param  *bm		The bookmark block to save.
 * \param  *filename	The filename to save to.
 */

void save_bookmark_file(bookmark_block *bm, char *filename)
{
	FILE			*out;
	bookmark_node		*node;

	if (bm == NULL)
		return;

	out = fopen(filename, "w");
	if (out == NULL) {
		// \TODO -- Add an error report here.
		return;
	}

	/* Write the file header. */

	fprintf(out, "# PrintPDF File\n# Written by PrintPDF\n\n");
	fprintf(out, "Format: 1.00\n\n");

	/* Write the bookmarks section. */

	fprintf(out, "[Bookmarks]\n");

	node = bm->root;

	while (node != NULL) {
		fprintf(out, "@: %s\n", node->title);
		fprintf(out, "Page: %d\n", node->page);
		if (node->yoffset > 0)
			fprintf(out, "YOffset: %d\n", node->yoffset);
		if (node->level > 1)
			fprintf(out, "Level: %d\n", node->level);
		if (!node->expanded)
			fprintf(out, "Expanded: %s\n", return_opt_string(node->expanded));

		node = node->next;
	}

	fclose(out);
	osfile_set_type (filename, (bits) PRINTPDF_FILE_TYPE);

	set_bookmark_unsaved_state(bm, 0);
}


/**
 * Load a bookmark file into memory, storing the data it contains in a new
 * bookmark_block structure.
 *
 * \param  *filename	The file to load.
 */

void load_bookmark_file(char *filename)
{
	FILE			*in;
	bookmark_block		*block;
	bookmark_node		*current, *new;
	int			result, bookmarks = 0, unknown_data = 0;
	char			section[BOOKMARK_FILE_LINE_LEN], token[BOOKMARK_FILE_LINE_LEN], value[BOOKMARK_FILE_LINE_LEN];


	block = create_bookmark_block();

	if (block == NULL) {
		// \TODO -- Add an error report here.
		return;
	}

	in = fopen(filename, "r");

	if (in == NULL) {
		// \TODO -- Add an error report here.
		delete_bookmark_block(block);
		return;
	}

	hourglass_on();

	strncpy(block->filename, filename, MAX_BOOKMARK_FILENAME);
	update_bookmark_window_title(block);

	/* Read the nodes into a linear linked list, ignoring for the time
	 * being any levels.
	 */

	current = NULL;

	while ((result = read_config_token_pair (in, token, value, section)) != sf_READ_CONFIG_EOF) {
		if (result == sf_READ_CONFIG_NEW_SECTION)
			bookmarks = (strcmp_no_case(section, "Bookmarks") == 0);

		if (bookmarks) {
			if (strcmp_no_case(token, "@") == 0) {
				new = (bookmark_node *) malloc(sizeof(bookmark_node));

				if (new != NULL) {
					strncpy(new->title, value, MAX_BOOKMARK_LEN);
					new->title[MAX_BOOKMARK_LEN - 1] = '\0';

					new->page = 0;
					new->yoffset = 0;
					new->expanded = 1;
					new->level = 1;
					new->count = 0;

					new->next = NULL;

					if (current == NULL)
						block->root = new;
					else
						current->next = new;

					current = new;
				}

			} else if (strcmp_no_case(token, "Page") == 0) {
				if (current != NULL)
					current->page = atoi(value);
			} else if (strcmp_no_case(token, "YOffset") == 0) {
				if (current != NULL)
					current->yoffset = atoi(value);
			} else if (strcmp_no_case(token, "Level") == 0) {
				if (current != NULL)
					current->level = atoi(value);
			} else if (strcmp_no_case(token, "Expanded") == 0) {
				if (current != NULL)
					current->expanded = read_opt_string(value);
			} else {
				unknown_data = 1;
			}
		}
	}

	hourglass_off();

	fclose (in);

	if (unknown_data)
		wimp_msgtrans_info_report ("UnknownFileData");

	rebuild_bookmark_data(block);
	open_bookmark_window(block);
}


/**
 * Recalculate the details of a bookmark block.
 *
 * \param *bm		Pointer to the block to recalculate.
 */

void rebuild_bookmark_data(bookmark_block *bm)
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

	set_bookmark_window_extent(bm);

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

void write_pdfmark_out_file(FILE *pdfmark_file, bookmark_params *params)
{
	bookmark_node		*node;
	char			buffer[MAX_BOOKMARK_LEN * 4];

	params->bookmarks = find_bookmark_block(params->bookmarks);

	if (pdfmark_file != NULL && bookmark_data_available(params))
		for (node = params->bookmarks->root; node != NULL; node = node->next) {
			if (strlen(node->title) > 0 && node->page > 0) {
				fprintf(pdfmark_file, "[");

				if (node->count > 0)
					fprintf(pdfmark_file, " /Count %d", (node->expanded) ? node->count : -node->count);

				fprintf(pdfmark_file, " /Page %d /View [/XYZ 0 %.4f null] /Title (%s) /OUT pdfmark\n",
						node->page, ((double) node->yoffset / 1000),
						convert_to_pdf_doc_encoding(buffer, node->title, MAX_BOOKMARK_LEN * 4));
			}
		}
}

