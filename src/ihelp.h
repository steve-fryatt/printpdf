/* PrintPDF - ihelp.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef PRINTPDF_IHELP
#define PRINTPDF_IHELP

/* ==================================================================================================================
 * Static constants
 */

#define IHELP_LENGTH 236
#define IHELP_INAME_LEN 64

/* ==================================================================================================================
 * Function prototypes.
 */


/**
 * Initialise the interactive help system.
 */

void ihelp_initialise(void);


/**
 * Add a new interactive help window definition.
 *
 * \param window		The window handle to attach help to.
 * \param *name			The token name to associate with the window.
 * \param *decode		A function to use to help decode clicks in the window.
 */

void ihelp_add_window(wimp_w window, char* name, void (*decode) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state));

/**
 * Remove an interactive help definition from the window list.
 *
 * \param window		The window handle to remove from the list.
 */

void ihelp_remove_window(wimp_w window);

#endif

