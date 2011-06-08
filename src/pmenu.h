/* PrintPDF - pmenu.h
 *
 * (c) Stephen Fryatt, 2007-2011
 */

#ifndef PRINTPDF_PMENU
#define PRINTPDF_PMENU

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

char *param_menu_entry (char *buffer, char* param_list, int entry);

#endif

