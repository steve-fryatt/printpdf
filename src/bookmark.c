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

#include "menus.h"
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
void		bookmark_window_redraw_loop(bookmark_block *bm,
				wimp_draw *redraw);
void		rebuild_bookmark_data(bookmark_block *bm);

/* ==================================================================================================================
 * Global variables.
 */

/* Pointer to bookmark window data list. */

bookmark_block *bookmarks_list = NULL;

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

	new = (bookmark_block *) malloc(sizeof(bookmark_block));

	if (new != NULL) {
		new->window = NULL;
		new->toolbar = NULL;
		new->redraw = NULL;
		new->root = NULL;
		new->lines = 0;

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

	extern global_windows	windows;


	more = wimp_redraw_window(redraw);

	ox = redraw->box.x0 - redraw->xscroll;
	oy = redraw->box.y1 - redraw->yscroll;

	while (more) {
		top = (oy - redraw->clip.y1 - BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;
		if (top < 0)
			top = 0;

		bottom = ((BOOKMARK_LINE_HEIGHT * 1.5) + oy - redraw->clip.y0
				- BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;

		for (y = top; y <= bottom; y++) {
			windows.bookmark_window_def->icons[0].extent.y0 =
					(-y * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			windows.bookmark_window_def->icons[0].extent.y1 =
					(-y * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);
			windows.bookmark_window_def->icons[1].extent.y0 =
					(-y * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT);
			windows.bookmark_window_def->icons[1].extent.y1 =
					(-y * BOOKMARK_LINE_HEIGHT
					+ BOOKMARK_LINE_OFFSET
					- BOOKMARK_TOOLBAR_HEIGHT
					+ BOOKMARK_ICON_HEIGHT);

			wimp_plot_icon(&(windows.bookmark_window_def->icons[0]));
			wimp_plot_icon(&(windows.bookmark_window_def->icons[1]));
		}

		more = wimp_get_rectangle(redraw);
	}
}

/* ------------------------------------------------------------------------------------------------------------------ */

bookmark_block *find_bookmark_window(wimp_w window)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && bm->window != window)
		bm = bm->next;

	return (bm);
}

/* ------------------------------------------------------------------------------------------------------------------ */

bookmark_block *find_bookmark_toolbar(wimp_w window)
{
	bookmark_block		*bm = bookmarks_list;

	while ((bm != NULL) && bm->toolbar != window)
		bm = bm->next;

	return (bm);
}

/* ==================================================================================================================
 * Bookmark Settings Support
 */

void initialise_bookmark_settings(bookmark_params *params)
{
}

/* ------------------------------------------------------------------------------------------------------------------ */

void fill_bookmark_field (wimp_w window, wimp_i icon, bookmark_params *params)
{
	msgs_lookup ("Info", indirected_icon_text (window, icon), 20);
	wimp_set_icon_state (window, icon, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */


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
					new->expanded = 0;
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
			}
		}
	}

	hourglass_off();

	current = block->root;

	while (current != NULL) {
		debug_printf("Node: '%s'", current->title);
		debug_printf("  For page %d, at level %d", current->destination, current->level);

		current = current->next;
	}

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
	bookmark_node		*node;
	int			count;

	if (bm == NULL)
		return;

	/* Find the number of entries in the block list, and reallocate the
	 * redraw array if it has changed.
	 */

	count = 0;
	for (node = bm->root; node != NULL; node = node->next)
		count++;

	if (count != bm->lines) {
		if (bm->redraw != NULL)
			free(bm->redraw);

		bm->redraw = (bookmark_redraw *) malloc(count * sizeof(bookmark_redraw));
		bm->lines = count;
	}

	if (bm->redraw != NULL) {
		count = 0;

		for (node = bm->root; node != NULL; node = node->next) {
			bm->redraw[count].node = node;
			bm->redraw[count].selected = 0;

			count++;
		}
	}

	debug_printf("Found %d entries in file.", count);

}

