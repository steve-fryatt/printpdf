/* PrintPDF - menus.h
 *
 * (c) Stephen Fryatt, 2003
 */

#ifndef _PRINTPDF_MENUS
#define _PRINTPDF_MENUS

/* ==================================================================================================================
 * Type definitions
 */

typedef struct {
	wimp_menu	*menu_up;

	wimp_menu	*icon_bar;
	wimp_menu	*params;
	wimp_menu	*bookmarks;
	wimp_menu	*bookmarks_sub_view;
	wimp_menu	*bookmarks_sub_level;
	wimp_menu	*bookmarks_sub_insert;
	wimp_menu	*bookmarks_list;
} global_menus;

/* ------------------------------------------------------------------------------------------------------------------
 * Function prototypes.
 */

void load_menu_definitions (char *menu_file);
char *get_current_menu_name (char *buffer);

/* Popup parameter menus */

void open_param_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon);

void decode_param_menu (wimp_selection *selection, wimp_pointer *pointer);

#endif
