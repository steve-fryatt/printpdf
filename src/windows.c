/* PrintPDF - windows.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

#include <stdlib.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/errors.h"
#include "sflib/event.h"
#include "sflib/windows.h"
#include "sflib/msgs.h"
#include "sflib/debug.h"
#include "sflib/url.h"
#
/* Application header files */

#include "windows.h"

#include "ihelp.h"
#include "convert.h"

/* ==================================================================================================================
 * Global variables.
 */

global_windows windows;

static osbool	windows_proginfo_web_click(wimp_pointer *pointer);

/* ==================================================================================================================
 * Load window template definitions.
 */

void load_window_templates (char *template_file, osspriteop_area *sprites)
{
  wimp_window  *window_def;
  os_error     *error;


  error = xwimp_open_template (template_file);
  if (error != NULL)
  {
    wimp_program_report (error);
  }

  /* Program Info Window.
   *
   * Created now.
   */

  window_def = load_window_template ("ProgInfo");
  if (window_def != NULL)
  {
    windows.prog_info = wimp_create_window (window_def);
    ihelp_add_window (windows.prog_info, "ProgInfo", NULL);
    msgs_param_lookup ("Version",
                       window_def->icons[ICON_PROGINFO_VERSION].data.indirected_text.text,
                       window_def->icons[ICON_PROGINFO_VERSION].data.indirected_text.size,
                       BUILD_VERSION, BUILD_DATE, NULL, NULL);
    event_add_window_icon_click(windows.prog_info, ICON_PROGINFO_WEBSITE, windows_proginfo_web_click);

    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* File Info Window.
   *
   * Created now.
   */

  window_def = load_window_template ("FileInfo");
  if (window_def != NULL)
  {
    windows.file_info = wimp_create_window (window_def);
    ihelp_add_window (windows.file_info, "FileInfo", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Save PDF Window.
   *
   * Created now.
   */

  window_def = load_window_template ("SavePDF");
  if (window_def != NULL)
  {
    windows.save_pdf = wimp_create_window (window_def);
    ihelp_add_window (windows.save_pdf, "SavePDF", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Save As Window.
   *
   * Created now.
   */

  window_def = load_window_template ("SaveAs");
  if (window_def != NULL)
  {
    windows.save_as = wimp_create_window (window_def);
    ihelp_add_window (windows.save_as, "SaveAs", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Security 2 Window.
   *
   * Created now.
   */

  window_def = load_window_template ("Security2");
  if (window_def != NULL)
  {
    windows.security2 = wimp_create_window (window_def);
    ihelp_add_window (windows.security2, "Security2", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Security 3 Window.
   *
   * Created now.
   */

  window_def = load_window_template ("Security3");
  if (window_def != NULL)
  {
    windows.security3 = wimp_create_window (window_def);
    ihelp_add_window (windows.security3, "Security3", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Optimization Window.
   *
   * Created now.
   */

  window_def = load_window_template ("Optimize");
  if (window_def != NULL)
  {
    windows.optimization = wimp_create_window (window_def);
    ihelp_add_window (windows.optimization, "Optimize", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* PDFMark Window.
   *
   * Created now.
   */

  window_def = load_window_template ("PDFMark");
  if (window_def != NULL)
  {
    windows.pdfmark = wimp_create_window (window_def);
    ihelp_add_window (windows.pdfmark, "PDFMark", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Queue Window.
   *
   * Created now.
   */

  window_def = load_window_template ("Queue");
  if (window_def != NULL)
  {
    windows.queue = wimp_create_window (window_def);
    ihelp_add_window (windows.queue, "Queue", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Queue Pane.
   *
   * Created now.
   */

  window_def = load_window_template ("QueuePane");
  if (window_def != NULL)
  {
    window_def->icon_count = 0;
    windows.queue_pane = wimp_create_window (window_def);
    /* Interactive help is added in the convert module initialisation. */

    windows.queue_pane_def = window_def;
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Choices Window.
   *
   * Created now.
   */

  window_def = load_window_template ("Choices");
  if (window_def != NULL)
  {
    windows.choices = wimp_create_window (window_def);
    ihelp_add_window (windows.choices, "Choices", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Pop Up Window.
   *
   * Created now.
   */

  window_def = load_window_template ("PopUp");
  if (window_def != NULL)
  {
    windows.popup = wimp_create_window (window_def);
    ihelp_add_window (windows.popup, "PopUp", NULL);
    free (window_def);
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Bookmark Window.
   *
   * Definition Saved for later.
   */

  window_def = load_window_template ("BMark");
  if (window_def != NULL)
  {
    window_def->icon_count = 0;
    windows.bookmark_window_def = window_def;
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  /* Bookmark Pane.
   *
   * Definition Saved for later.
   */

  window_def = load_window_template ("BMarkPane");
  if (window_def != NULL)
  {
    windows.bookmark_pane_def = window_def;
  }
  else
  {
    wimp_msgtrans_fatal_report ("BadTemplate");
  }

  wimp_close_template ();
}




/**
 * Handle clicks on the Website action button in the program info window.
 *
 * \param *pointer	The Wimp Event message block for the click.
 * \return		TRUE if we handle the click; else FALSE.
 */

static osbool windows_proginfo_web_click(wimp_pointer *pointer)
{
	char		temp_buf[256];

	msgs_lookup("SupportURL:http://www.stevefryatt.org.uk/software/", temp_buf, sizeof(temp_buf));
	launch_url(temp_buf);

	if (pointer->buttons == wimp_CLICK_SELECT)
		wimp_create_menu((wimp_menu *) -1, 0, 0);

	return TRUE;
}

