/* PrintPDF - bookmark.h
 *
 * (c) Stephen Fryatt, 2010
 */

#ifndef _PRINTPDF_BOOKMARK
#define _PRINTPDF_BOOKARK

#include <stdio.h>

/* ==================================================================================================================
 * Static constants
 */

#define MAX_BOOKMARK_LEN 64  /* The real maximum is 256, but Adobe recommend 32 max for practicality. */
#define MAX_BOOKMARK_NUM_LEN 10
#define MAX_BOOKMARK_BLOCK_NAME 64
#define MAX_BOOKMARK_FIELD_LEN 20
#define MAX_BOOKMARK_FILENAME 256
#define MAX_BOOKMARK_FILESPR 9

#define BOOKMARK_TOOLBAR_HEIGHT 82
#define BOOKMARK_LINE_HEIGHT 56
#define BOOKMARK_ICON_HEIGHT 48
#define BOOKMARK_LINE_OFFSET 4
#define BOOKMARK_TOOLBAR_OFFSET 2
#define BOOKMARK_WINDOW_MARGIN 4
#define BOOKMARK_MIN_LINES 10
#define BOOKMARK_HORIZONTAL_SCROLL 4
#define BOOKMARK_COLUMN_WIDTH 160
#define BOOKMARK_WINDOW_WIDTH 1600
#define BOOKMARK_WINDOW_STANDOFF 400
#define BOOKMARK_WINDOW_OPENSTEP 100

#define BOOKMARK_FILE_LINE_LEN 1024

#define BOOKMARK_ABOVE 1
#define BOOKMARK_BELOW 2

/* Bookmarks Window Menu Structure. */

#define BOOKMARK_MENU_FILE   0
#define BOOKMARK_MENU_VIEW   1
#define BOOKMARK_MENU_LEVEL  2
#define BOOKMARK_MENU_INSERT 3
#define BOOKMARK_MENU_DELETE 4

#define BOOKMARK_MENU_FILE_INFO 0
#define BOOKMARK_MENU_FILE_SAVE 1

#define BOOKMARK_MENU_VIEW_EXPAND   0
#define BOOKMARK_MENU_VIEW_CONTRACT 1

#define BOOKMARK_MENU_LEVEL_PROMOTE  0
#define BOOKMARK_MENU_LEVEL_DEMOTE   1
#define BOOKMARK_MENU_LEVEL_PROMOTEG 2
#define BOOKMARK_MENU_LEVEL_DEMOTEG  3

#define BOOKMARK_MENU_INSERT_ABOVE 0
#define BOOKMARK_MENU_INSERT_BELOW 1

/* Bookmark Window icons. */

#define BOOKMARK_WINDOW_COLUMNS 3

#define BOOKMARK_ICON_EXPAND  0
#define BOOKMARK_ICON_TITLE   1
#define BOOKMARK_ICON_PAGE    2
#define BOOKMARK_ICON_YEXTENT 3

/* Bookmark Toolbar icons. */

#define BOOKMARK_TB_NAME 0

#define BOOKMARK_TB_SAVE     1
#define BOOKMARK_TB_DEMOTEG  2
#define BOOKMARK_TB_DEMOTE   3
#define BOOKMARK_TB_PROMOTE  4
#define BOOKMARK_TB_PROMOTEG 5
#define BOOKMARK_TB_EXPAND   6
#define BOOKMARK_TB_CONTRACT 7

/* Save As Dialogue icons. */

#define SAVEAS_ICON_OK     0
#define SAVEAS_ICON_CANCEL 1
#define SAVEAS_ICON_NAME   2
#define SAVEAS_ICON_FILE   3

/* File Info Dialogue icons. */

#define FILEINFO_ICON_NAME     1
#define FILEINFO_ICON_LOCATION 3
#define FILEINFO_ICON_MODIFIED 5
#define FILEINFO_ICON_DATE     7


/* ****************************************************************************
 * Data structures
 * ****************************************************************************/

typedef struct bookmark_node {
	char			title[MAX_BOOKMARK_LEN];
	int			page;		/*< Destination page number.		*/
	int			yoffset;	/*< Destination Y offset (millipt from top).	*/
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
	os_date_and_time	datestamp;

	int			unsaved;

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
} bookmark_block;

typedef struct bookmark_params {
	bookmark_block		*bookmarks;
} bookmark_params;


/* ****************************************************************************
 * Function prototypes
 * ****************************************************************************/

/* Bookmarks System Initialisation and Termination */

void initialise_bookmarks(void);
void terminate_bookmarks(void);

/* PDF Creation Interface */

void initialise_bookmark_settings(bookmark_params *params);
void open_bookmark_menu(bookmark_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_bookmark_menu(bookmark_params *params, wimp_selection *selection);
int load_and_select_bookmark_file(bookmark_params *params, char *filename);
void fill_bookmark_field (wimp_w window, wimp_i icon, bookmark_params *params);
int bookmark_data_available(bookmark_params *params);
int bookmark_validate_params(bookmark_params *params);

/* Bookmark Block Management */

int bookmark_files_unsaved(void);

/* Bookmark Window Handling */

bookmark_block *create_new_bookmark_window(void);

/* SaveAs Dialogue Handling */

int drag_end_save_saveas(char *filename);

/* Bookmark Data Processing */

void save_bookmark_file(bookmark_block *bm, char *filename);
bookmark_block *load_bookmark_file(char *filename);
void write_pdfmark_out_file(FILE *pdfmark_file, bookmark_params *params);

#endif
