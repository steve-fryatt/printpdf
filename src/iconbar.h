/* PrintPDF - iconbar.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef _PRINTPDF_
#define _PRINTPDF_

/* ==================================================================================================================
 * Static constants
 */

/* Iconbar menu */

#define ICONBAR_MENU_INFO 0
#define ICONBAR_MENU_HELP 1
#define ICONBAR_MENU_QUEUE 2
#define ICONBAR_MENU_CHOICES 3
#define ICONBAR_MENU_QUIT 4

/* ==================================================================================================================
 * Data structures
 */

/* ==================================================================================================================
 * Function prototypes.
 */

void initialise_iconbar(void);

/* Handle the iconbar icon. */

void set_iconbar_icon(int new_state);

#endif
