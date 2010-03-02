/* PrintPDF - windows.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef _PRINTPDF_WINDOWS
#define _PRINTPDF_WINDOWS

/* ==================================================================================================================
 * Static constants
 */

#define IND_DATA_SIZE 8000

/* ==================================================================================================================
 * Data structures
 */

typedef struct
{
  wimp_w      prog_info;
  wimp_w      save_pdf;
  wimp_w      security2;
  wimp_w      security3;
  wimp_w      optimization;
  wimp_w      pdfmark;
  wimp_w      queue;
  wimp_w      queue_pane;
  wimp_w      choices;
  wimp_w      popup;

  wimp_window *queue_pane_def;
}
global_windows;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Load window template definitions. */

void load_window_templates (char *template_file, osspriteop_area *sprites);

/* Handle the iconbar icon. */

void set_iconbar_icon (int new_state);

#endif
