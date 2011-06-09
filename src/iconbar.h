/* PrintPDF - iconbar.h
 *
 * (c) Stephen Fryatt, 2005-2011
 */

#ifndef PRINTPDF_ICONBAR
#define PRINTPDF_ICONBAR


/**
 * Initialise the iconbar icon and its associated menus and dialogues.
 */

void iconbar_initialise(void);


/**
 * Create or delete the icon on the iconbar, as appropriate.
 *
 * \param new_state		TRUE to create an icon; FALSE to remove it.
 */

void iconbar_set_icon(int new_state);

#endif

