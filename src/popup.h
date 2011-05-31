/* PrintPDF - popup.h
 *
 * (c) Stephen Fryatt, 2005-2011
 */

#ifndef PRINTPDF_POPUP
#define PRINTPDF_POPUP

/**
 * Initialise the popup window module.
 */

void popup_initialise(void);


/**
 * Test the time given against the time to close the popup, and if that time has passed, close the window.
 *
 * \param current		The current OS Monotonic Time.
 */

void popup_test_and_close(os_t current);


/**
 * Open the popup for a minimum of the given time (this may be longer, as checks are only made at the externally
 * selected poll interval).
 *
 * \param open_time	The time to open the window for.
 */

void popup_open(int open_time);

#endif

