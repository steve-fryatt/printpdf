/* PrintPDF - menus.h
 *
 * (c) Stephen Fryatt, 2003
 */

#ifndef _PRINTPDF_MENUS
#define _PRINTPDF_MENUS

/* ------------------------------------------------------------------------------------------------------------------
 * Static constants
 */

/* Iconbar menu */

#define ICONBAR_MENU_INFO 0
#define ICONBAR_MENU_HELP 1
#define ICONBAR_MENU_QUEUE 2
#define ICONBAR_MENU_CHOICES 3
#define ICONBAR_MENU_QUIT 4

/* ==================================================================================================================
 * Type definitions
 */

typedef struct {
	wimp_menu	*menu_up;

	wimp_menu	*icon_bar;
	wimp_menu	*params;
	wimp_menu	*bookmarks;
	wimp_menu	*bookmarks_sub_line;
	wimp_menu	*bookmarks_list;
} global_menus;

/* ------------------------------------------------------------------------------------------------------------------
 * Function prototypes.
 */

void load_menu_definitions (char *menu_file);
char *get_current_menu_name (char *buffer);

/* Iconbar menu */

void set_iconbar_menu (void);
void open_iconbar_menu (wimp_pointer *pointer);

void decode_iconbar_menu (wimp_selection *selection, wimp_pointer *pointer);

/* Popup parameter menus */

void open_param_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon);

void decode_param_menu (wimp_selection *selection, wimp_pointer *pointer);

#endif
