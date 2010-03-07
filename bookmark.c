/* PrintPDF - bookmark.c
 *
 * (C) Stephen Fryatt, 2010
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>

/* Acorn C header files */

/* OSLib header files */

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

void delete_bookmark_block(bookmark_block *bookmark);
bookmark_block *find_bookmark_window(wimp_w window);
bookmark_block *find_bookmark_toolbar(wimp_w window);
void bookmark_window_redraw_loop(bookmark_block *bm, wimp_draw *redraw);

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
  bookmark_block *bm, *next_bm;

  /* Work through the bookmarks list, deleting everything. */

  bm = bookmarks_list;

  while (bm != NULL)
  {
    wimp_delete_window(bm->window);
    wimp_delete_window(bm->toolbar);

    next_bm = bm->next;
    free(bm);
    bm = next_bm;
  }

  bookmarks_list = NULL;
}

/* ==================================================================================================================
 * Window and Editing Support
 */

void create_new_bookmark_window(wimp_pointer *pointer)
{
  bookmark_block        *new;
  extern global_windows windows;

  new = (bookmark_block *) malloc(sizeof(bookmark_block));

  if (new != NULL)
  {
    place_window_as_toolbar(windows.bookmark_window_def, windows.bookmark_pane_def, BOOKMARK_TOOLBAR_HEIGHT
                                                                                     - BOOKMARK_TOOLBAR_OFFSET);
    new->window = wimp_create_window(windows.bookmark_window_def);
    new->toolbar = wimp_create_window(windows.bookmark_pane_def);

    /* Link in to chain. */

    new->next = bookmarks_list;
    bookmarks_list = new;

    /* Open the window and toolbar. */

    open_window(new->window);
    open_window_nested_as_toolbar(new->toolbar, new->window, BOOKMARK_TOOLBAR_HEIGHT - BOOKMARK_TOOLBAR_OFFSET);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void delete_bookmark_block(bookmark_block *bookmark)
{
  bookmark_block **bm, *f;

  bm = &bookmarks_list;

  while ((*bm != NULL) && (*bm != bookmark))
    bm = &((*bm)->next);

  if (*bm != NULL)
  {
    f = *bm;
    *bm = (*bm)->next;
    free(f);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

int close_bookmark_window(wimp_w window)
{
  bookmark_block *bm;

  bm = find_bookmark_window(window);

  if (bm != NULL)
  {
    wimp_delete_window(bm->window);
    wimp_delete_window(bm->toolbar);
    delete_bookmark_block(bm);

    return 1;
  }
  else
  {
    return 0;
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

int redraw_bookmark_window(wimp_draw *redraw)
{
  bookmark_block *bm;

  bm = find_bookmark_window(redraw->w);

  if (bm != NULL)
  {
    bookmark_window_redraw_loop(bm, redraw);

    return 1;
  }
  else
  {
    return 0;
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void bookmark_window_redraw_loop(bookmark_block *bm, wimp_draw *redraw)
{
  int     ox, oy, top, bottom, y;
  osbool  more;

  extern global_windows windows;

  more = wimp_redraw_window(redraw);

  ox = redraw->box.x0 - redraw->xscroll;
  oy = redraw->box.y1 - redraw->yscroll;

  while (more)
  {
    top = (oy - redraw->clip.y1 - BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;
    if (top < 0)
      top = 0;

    bottom = ((BOOKMARK_LINE_HEIGHT * 1.5) + oy - redraw->clip.y0 - BOOKMARK_TOOLBAR_HEIGHT) / BOOKMARK_LINE_HEIGHT;

    for (y = top; y <= bottom; y++)
    {
      windows.bookmark_window_def->icons[0].extent.y0 = (-y * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET
                                                            - BOOKMARK_TOOLBAR_HEIGHT);
      windows.bookmark_window_def->icons[0].extent.y1 = (-y * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET
                                                            - BOOKMARK_TOOLBAR_HEIGHT + BOOKMARK_ICON_HEIGHT);
      windows.bookmark_window_def->icons[1].extent.y0 = (-y * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET
                                                            - BOOKMARK_TOOLBAR_HEIGHT);
      windows.bookmark_window_def->icons[1].extent.y1 = (-y * BOOKMARK_LINE_HEIGHT + BOOKMARK_LINE_OFFSET
                                                            - BOOKMARK_TOOLBAR_HEIGHT + BOOKMARK_ICON_HEIGHT);

      wimp_plot_icon(&(windows.bookmark_window_def->icons[0]));
      wimp_plot_icon(&(windows.bookmark_window_def->icons[1]));
    }

    more = wimp_get_rectangle(redraw);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

bookmark_block *find_bookmark_window(wimp_w window)
{
  bookmark_block *bm = bookmarks_list;

  while ((bm != NULL) && bm->window != window)
  {
    bm = bm->next;
  }

  return (bm);
}

/* ------------------------------------------------------------------------------------------------------------------ */

bookmark_block *find_bookmark_toolbar(wimp_w window)
{
  bookmark_block *bm = bookmarks_list;

  while ((bm != NULL) && bm->toolbar != window)
  {
    bm = bm->next;
  }

  return (bm);
}

/* ==================================================================================================================
 * Bookmark Settings Support
 */

void initialise_bookmark_settings (bookmark_params *params)
{
}

/* ------------------------------------------------------------------------------------------------------------------ */

void fill_bookmark_field (wimp_w window, wimp_i icon, bookmark_params *params)
{
  msgs_lookup ("Info", indirected_icon_text (window, icon), 20);
  wimp_set_icon_state (window, icon, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */


/* ==================================================================================================================
 * Bookmark File Handling
 */

void load_bookmark_file (bookmark_block *mb, char *filename)
{

}