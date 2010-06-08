/* PrintPDF - bookmark.c
 *
 * (C) Stephen Fryatt, 2010
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/hourglass.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/windows.h"
#include "sflib/debug.h"

/* Application header files */

#include "bookmark.h"

#include "convert.h"
#include "menus.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "windows.h"

/* ==================================================================================================================
 * Data Structures.
 */


/* ==================================================================================================================
 * Function prototypes.
 */

void		open_bookmark_window(bookmark_block *bm);
bookmark_block	*create_bookmark_block(void);
void		delete_bookmark_block(bookmark_block *bookmark);
bookmark_block	*find_bookmark_window(wimp_w window);
bookmark_block	*find_bookmark_toolbar(wimp_w window);
bookmark_block	*find_bookmark_name(char *name);
bookmark_block	*find_bookmark_block(bookmark_block *block);
wimp_menu	*build_bookmark_menu(bookmark_params *params);
void		bookmark_window_redraw_loop(bookmark_block *bm,
				wimp_draw *redraw);
void		rebuild_bookmark_data(bookmark_block *bm);

/* ==================================================================================================================
 * Global variables.
 */

/* Pointer to bookmark window data list. */

static bookmark_block *bookmarks_list = NULL;
static int		untitled_number = 1;

static wimp_menu	*bookmarks_menu = NULL;
static bookmark_block	**bookmarks_menu_links = NULL;
static int		bookmarks_menu_size = 0;


/* ==================================================================================================================
 * General System Initialisation
 */

void initialise_bookmarks(void)
{
}

/* ------------------------------------------------------------------------------------------------------------------ */

void terminate_bookmarks(void)
{
	/* Work through the bookmarks list, deleting everything. */

	while (bookmarks_list != NULL)
		delete_bookmark_block(bookmarks_list);
}

/* ==================================================================================================================
 * Window and Editing Support
 */



/**
 * Create a new bookmark block and initialise it with some standard
 * values.
 *
 * Return:		Pointer to block; or NULL if failed.
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
		new->window = NULL;
		new->toolbar = NULL;
		new->redraw = NULL;
		new->root = NULL;
		new->lines = 0;
		new->nodes = 0;

		new->next = bookmarks_list;
		bookmarks_list = new;
	}

	return new;
}

/**
 * Delete a bookmark block and its associated data.
 *
 * Param:  *bookmark		The block to delete.
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
 * Given a pointer click, create a new bookmark window.
 *
 * Param:  *pointer		The details of the mouse click.
 */

void create_new_bookmark_window(wimp_pointer *pointer)
{
	bookmark_block		*new;

	new = create_bookmark_block();

	if (new != NULL)
		rebuild_bookmark_data(new);
		open_bookmark_window(new);
}

/**
 * Create and open a window for a bookmark block.
 *
 * Param: *bm		The block to open the window for.
 */

void open_bookmark_window(bookmark_block *bm)
{
	extern global_windows	windows;

	if (bm != NULL && bm->window == NULL && bm->toolbar == NULL) {
		windows.bookmark_window_def->title_data.indirected_text.text = bm->name;
		windows.bookmark_window_def->title_data.indirected_text.size = MAX_BOOKMARK_BLOCK_NAME;

		place_window_as_toolbar(windows.bookmark_window_def,
				windows.bookmark_pane_def,
				BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_TOOLBAR_OFFSET);
		bm->window = wimp_create_window(windows.bookmark_window_def);
		bm->toolbar = wimp_create_window(windows.bookmark_pane_def);

		/* Open the window and toolbar. */

		open_window(bm->window);
		open_window_nested_as_toolbar(bm->toolbar, bm->window,
				BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_TOOLBAR_OFFSET);
	}
}

/**
 * Close the given bookmark window and delete all of the associated data.
 *
 * Param:  window		The window handle of the window to be closed.
 * Return:			0 if the close was successful; 1 if the window
 *				handle wasn't recognised.
 */

int close_bookmark_window(wimp_w window)
{
	bookmark_block		*bm;

	bm = find_bookmark_window(window);

	if (bm != NULL) {
		wimp_delete_window(bm->window);
		wimp_delete_window(bm->toolbar);
		delete_bookmark_block(bm);

		return 0;
	} else {
		return 1;
	}
}

/* ------------------------------------------------------------------------------------------------------------------ */

int redraw_bookmark_window(wimp_draw *redraw)
{
	bookmark_block		*bm;

	bm = find_bookmark_window(redraw->w);

	if (bm != NULL) {
		bookmark_window_redraw_loop(bm, redraw);

		return 1;
	} else {
		return 0;
	}
}

/* ------------------------------------------------------------------------------------------------------------------ */

void bookmark_window_redraw_loop(bookmark_block *bm, wimp_draw *redraw)
{
	int			ox, oy, top, bottom, y;
	osbool			more;
	bookmark_node		*node;
	wimp_icon		*icon;
	char			buf[64];

	extern global_windows	windows;
	extern osspriteop_area	*wimp_sprites;


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
			node = bm->redraw[y].node;

			icon[0].extent.x0 = node->level * BOOKMARK_LINE_HEIGHT;

			icon[0].extent.y0 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			icon[0].extent.y1 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);
			icon[1].extent.y0 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			icon[1].extent.y1 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);
			icon[2].extent.x0 = (node->level - 1) * BOOKMARK_LINE_HEIGHT;
			icon[2].extent.x1 = icon[2].extent.x0 + BOOKMARK_ICON_HEIGHT;
			icon[2].extent.y0 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			icon[2].extent.y1 = (-(y+1) * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);

			sprintf(buf, "Line %d", y);

			icon[0].data.indirected_text.text = node->title;
			icon[1].data.indirected_text.text = buf;
			icon[2].data.indirected_sprite.id = (osspriteop_id) ((node->expanded) ? "nodee" : "nodec");
			icon[2].data.indirected_sprite.area = wimp_sprites;
			icon[2].data.indirected_sprite.size = 6;

			wimp_plot_icon(&(icon[0]));
			wimp_plot_icon(&(icon[1]));

			/* Plot the expansion arrow for node heads, which show up
			 * as entries whose count is non-zero.
			 */

			if (node->count > 0)
				wimp_plot_icon(&(icon[2]));
		}

		more = wimp_get_rectangle(redraw);
	}
}


/**
 * Find a bookmark block by its window handle.
 *
 * Param:  window		The handle of the window.
 * Return: 			The block address, or NULL if it wasn't found.
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
 * Param:  window		The handle of the toolbar.
 * Return: 			The block address, or NULL if it wasn't found.
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
 * Param:  *name		The block name to find.
 * Return: 			The block address, or NULL if it wasn't found.
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
 * Param:  *block		The block to validate.
 * Return: 			The block address, or NULL if it failed.
 */

bookmark_block *find_bookmark_block(bookmark_block *block)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && bm != block)
		bm = bm->next;

	return bm;
}

/* ==================================================================================================================
 * Bookmark Settings Support
 */

/**
 * Initialise a bookmarks settings block with default parameters.
 *
 * Param:  *params		The parameter block to initialise.
 */

void initialise_bookmark_settings(bookmark_params *params)
{
	params->bookmarks = NULL;
}


/**
 * Create and open the bookmark list pop-up menu.
 *
 * Param:  *params		The associated bookmark parameters.
 * Param:  *pointer		The pointer details of the click.
 * Param:  window		The window in which the pop-up resides.
 * Param:  icon			The icon to which the pop-up is attached.
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
 * Param:  *params		The associated bookmarks parameters.
 * Param:  *selection		The Wimp Menu selection block.
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
 * Param:  *params		Bookamrks param block to use to set ticks.
 * Return:			The menu block, or NULL.
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
 * Param:  window		The window the field is in.
 * Param:  icon			The icon for the field.
 * Param:  *params		The parameters to use.
 */

void fill_bookmark_field (wimp_w window, wimp_i icon, bookmark_params *params)
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
 * Param:  *params		The parameter block to check.
 * Return:			1 if data is available; else 0.
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
 * Param:  *params		The parameter block to check.
 * Return:			1 if parameters were chnaged; else 0.
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

/**
 * Output PDFMark data related to the associated bookmarks parameters file.
 *
 * Param:  *pdfmark_file	The file to write to.
 * Param:  *params		The parameter block to use.
 */

void write_pdfmark_out_file(FILE *pdfmark_file, bookmark_params *params)
{
	bookmark_node		*node;
	char			buffer[MAX_BOOKMARK_LEN * 4];

	params->bookmarks = find_bookmark_block(params->bookmarks);

	if (pdfmark_file != NULL && bookmark_data_available(params))
		for (node = params->bookmarks->root; node != NULL; node = node->next) {
			fprintf(pdfmark_file, "[");

			if (node->count > 0)
				fprintf(pdfmark_file, " /Count %d", (node->expanded) ? node->count : -node->count);

			fprintf(pdfmark_file, " /Page %d /Title (%s) /OUT pdfmark\n", node->destination,
					convert_to_pdf_doc_encoding(buffer, node->title, MAX_BOOKMARK_LEN * 4));
		}
}


/**
 * Load a bookmark file into memory, storing the data it contains in a new
 * bookmark_block structure.
 *
 * Param:  *filename		The file to load.
 */

void load_bookmark_file(char *filename)
{
	FILE			*in;
	bookmark_block		*block;
	bookmark_node		*current, *new;
	int			result, bookmarks = 0;
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

					new->destination = 0;
					new->expanded = 1;
					new->level = 0;
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
					current->destination = atoi(value);
			} else if (strcmp_no_case(token, "Level") == 0) {
				if (current != NULL)
					current->level = atoi(value);
			} else if (strcmp_no_case(token, "Expanded") == 0) {
				if (current != NULL)
					current->expanded = read_opt_string(value);
			}
		}
	}

	hourglass_off();

	fclose (in);

	rebuild_bookmark_data(block);
	open_bookmark_window(block);
}


/**
 * Recalculate the details of a bookmark block.
 *
 * Param: *bm		Pointer to the block to recalculate.
 */

void rebuild_bookmark_data(bookmark_block *bm)
{
	bookmark_node		*node, *n;
	int			count, i;

	if (bm == NULL)
		return;

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

	if (bm->redraw != NULL) {
		count = 0;

		for (node = bm->root; node != NULL; node = node->next) {
			bm->redraw[count].node = node;
			bm->redraw[count].selected = 0;

			/* Skip past any contracted lines. */

			if (!node->expanded)
				for (i = node->count; node != NULL && i > 0; node = node->next)
					i--;

			count++;
		}

		bm->lines = count;
	}
}

