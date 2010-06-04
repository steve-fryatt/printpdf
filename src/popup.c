/* PrintPDF - popup.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

/* Acorn C header files */

/* OSLib header files */

#include "oslib/os.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/windows.h"

/* Application header files */

#include "popup.h"

#include "windows.h"

/* ==================================================================================================================
 * Global variables.
 */

static int  popup_open = FALSE;
static os_t popup_close_time = 0;

/* ==================================================================================================================
 * Close the popup window.
 */

/* Test the time given against the time to close the popup, and if that time has passed, close the window.
 */

void test_and_close_popup (os_t current)
{
  extern global_windows windows;


  if (popup_open == TRUE)
  {
    if (current >= popup_close_time)
    {
      wimp_close_window (windows.popup);

      popup_close_time = 0;
      popup_open = FALSE;
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void close_popup (void)
{
  extern global_windows windows;


  if (popup_open == TRUE)
  {
    wimp_close_window (windows.popup);

    popup_close_time = 0;
    popup_open = FALSE;
  }
}

/* ==================================================================================================================
 * Open the popup window.
 */

/* Open the popup for a minimum of the given time (this may be longer, as checks are only made at the externally
 * selected poll interval).
 */

void open_popup (int open_time)
{
  extern global_windows windows;


  popup_open = TRUE;
  popup_close_time = os_read_monotonic_time () + open_time;

  open_window_centred_on_screen (windows.popup);
}
