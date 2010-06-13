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
#define MAX_BOOKMARK_FILENAME 256

#define BOOKMARK_TOOLBAR_HEIGHT 82
#define BOOKMARK_LINE_HEIGHT 56
#define BOOKMARK_ICON_HEIGHT 48
#define BOOKMARK_LINE_OFFSET 0
#define BOOKMARK_TOOLBAR_OFFSET 2
#define BOOKMARK_MIN_LINES 10

#define BOOKMARK_FILE_LINE_LEN 1024

/* Bookmarks Window Menu Structure. */

#define BOOKMARK_MENU_FILE 0

#define BOOKMARK_MENU_FILE_INFO 0
#define BOOKMARK_MENU_FILE_SAVE 1

/* Save As Dialogue icons. */

#define SAVEAS_ICON_OK     0
#define SAVEAS_ICON_CANCEL 1
#define SAVEAS_ICON_NAME   2
#define SAVEAS_ICON_FILE   3


/* ==================================================================================================================
 * Data structures
 */

typedef struct bookmark_node {
	char			title[MAX_BOOKMARK_LEN];
	int			page;		/*< Destination page number.		*/
	int			yoffset;	/*< Destination Y offset (pt from top).	*/
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
	char			filename[MAX_BOOKMARK_FILENAME];
	char			window_title[MAX_BOOKMARK_FILENAME+MAX_BOOKMARK_BLOCK_NAME+10];

	int			unsaved;

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

void initialise_bookmark_settings(bookmark_params *params);
void open_bookmark_menu(bookmark_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_bookmark_menu(bookmark_params *params, wimp_selection *selection);

void fill_bookmark_field (wimp_w window, wimp_i icon, bookmark_params *params);
int bookmark_data_available(bookmark_params *params);
int bookmark_validate_params(bookmark_params *params);

/* Bookmark Window Handling */

void create_new_bookmark_window(wimp_pointer *pointer);

/* SaveAs Dialogue Handling */

int drag_end_save_saveas(char *filename);

/* Bookmark Data Processing */

void save_bookmark_file(bookmark_block *bm, char *filename);
void load_bookmark_file(char *filename);
void write_pdfmark_out_file(FILE *pdfmark_file, bookmark_params *params);

#endif
