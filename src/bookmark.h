/* PrintPDF - bookmark.h
 *
 * (c) Stephen Fryatt, 2010
 */

#ifndef _PRINTPDF_BOOKMARK
#define _PRINTPDF_BOOKARK

/* ==================================================================================================================
 * Static constants
 */

#define MAX_BOOKMARK_LEN 64  /* The real maximum is 256, but Adobe recommend 32 max for practicality. */
#define MAX_BOOKMARK_BLOCK_NAME 64
#define MAX_BOOKMARK_FIELD_LEN 20
#define BOOKMARK_TOOLBAR_HEIGHT 82
#define BOOKMARK_LINE_HEIGHT 60
#define BOOKMARK_ICON_HEIGHT 52
#define BOOKMARK_LINE_OFFSET 0
#define BOOKMARK_TOOLBAR_OFFSET 2

#define BOOKMARK_FILE_LINE_LEN 1024

/* ==================================================================================================================
 * Data structures
 */

typedef struct bookmark_node {
	char			title[MAX_BOOKMARK_LEN];
	int			destination;
	int			level;
	int			count;

	int			expanded;

	struct bookmark_node	*next;
} bookmark_node;

typedef struct bookmark_redraw {
	bookmark_node		*node;
	int			selected;
} bookmark_redraw;

typedef struct bookmark_block {
	char			name[MAX_BOOKMARK_BLOCK_NAME];

	wimp_w			window;
	wimp_w			toolbar;

	bookmark_redraw		*redraw;
	int			lines;

	bookmark_node		*root;
	int			nodes;

	struct bookmark_block	*next;
} bookmark_block;

typedef struct bookmark_params {
	bookmark_block		*bookmarks;
} bookmark_params;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Handle the PDFMark window and menu. */

void initialise_bookmarks(void);
void terminate_bookmarks(void);

void create_new_bookmark_window(wimp_pointer *pointer);
int close_bookmark_window(wimp_w window);
int redraw_bookmark_window(wimp_draw *redraw);

void initialise_bookmark_settings(bookmark_params *params);
void open_bookmark_menu(bookmark_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_bookmark_menu(bookmark_params *params, wimp_selection *selection);

void fill_bookmark_field (wimp_w window, wimp_i icon, bookmark_params *params);
int bookmark_data_available(bookmark_params *params);
void write_pdfmark_out_file(FILE *pdfmark_file, bookmark_params *params);

void load_bookmark_file(char *filename);

#endif
