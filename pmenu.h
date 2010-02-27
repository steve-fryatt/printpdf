/* PrintPDF - pmenu.h
 *
 * (c) Stephen Fryatt, 2007
 */

#ifndef _PRINTPDF_PMENU
#define _PRINTPDF_PMENU

/* ==================================================================================================================
 * Static constants
 */

/* Parameter menu */

#define PARAM_MENU_SIZE 10
#define PARAM_MENU_LEN  32

/* Menu types */

#define PARAM_MENU_DEFAULT 0
#define PARAM_MENU_CONVERT_OPTIMIZE 1
#define PARAM_MENU_CONVERT_VERSION 2
#define PARAM_MENU_CHOICES_OPTIMIZE 3
#define PARAM_MENU_CHOICES_VERSION 4

/* ==================================================================================================================
 * Function prototypes.
 */

wimp_menu *build_param_menu (char *param_list, int ident, int current);

int param_menu_ident (void);

char *param_menu_entry (char *buffer, char* param_list, int entry);

int param_menu_len (char *param_list);

#endif
