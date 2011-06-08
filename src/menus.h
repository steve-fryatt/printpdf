/* PrintPDF - menus.h
 *
 * (c) Stephen Fryatt, 2003-2011
 */

#ifndef PRINTPDF_MENUS
#define PRINTPDF_MENUS

/* ==================================================================================================================
 * Type definitions
 */

typedef struct {
	wimp_menu	*menu_up;

	wimp_menu	*icon_bar;
	wimp_menu	*bookmarks;
	wimp_menu	*bookmarks_sub_view;
	wimp_menu	*bookmarks_sub_level;
	wimp_menu	*bookmarks_sub_insert;
	wimp_menu	*bookmarks_list;
	wimp_menu	*version_popup;
	wimp_menu	*optimize_popup;
} global_menus;

/* ------------------------------------------------------------------------------------------------------------------
 * Function prototypes.
 */

void load_menu_definitions (char *menu_file);
char *get_current_menu_name (char *buffer);

#endif

