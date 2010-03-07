/* PrintPDF - bookmark.h
 *
 * (c) Stephen Fryatt, 2010
 */

#ifndef _PRINTPDF_BOOKMARK
#define _PRINTPDF_BOOKARK

/* ==================================================================================================================
 * Static constants
 */

/* Optimization Window icons. */


/* ==================================================================================================================
 * Data structures
 */

typedef struct bookmark_block
{
  wimp_w                window;
  wimp_w                toolbar;

  struct bookmark_block *next;
} bookmark_block;

typedef struct bookmark_params
{
  bookmark_block *bookmarks;
}
bookmark_params;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Handle the PDFMark window and menu. */

void initialise_bookmarks (void);
void terminate_bookmarks(void);

void create_new_bookmark_window(wimp_pointer *pointer);
int close_bookmark_window(wimp_w window);

void initialise_bookmark_settings (bookmark_params *params);
void fill_bookmark_field (wimp_w window, wimp_i icon, bookmark_params *params);

#endif
