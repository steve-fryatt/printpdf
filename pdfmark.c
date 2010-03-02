/* PrintPDF - pdfmark.c
 *
 * (C) Stephen Fryatt, 2010
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/windows.h"
#include "sflib/debug.h"

/* Application header files */

#include "pdfmark.h"

#include "menus.h"
#include "pmenu.h"
#include "windows.h"

/* ==================================================================================================================
 * Global variables.
 */


/* ==================================================================================================================
 *
 */

void initialise_pdfmark_settings (pdfmark_params *params)
{
  strncpy(params->title, read_config_str("PDFMarkTitle"), MAX_INFO_FIELD);
  strncpy(params->author, read_config_str("PDFMarkAuthor"), MAX_INFO_FIELD);
  strncpy(params->subject, read_config_str("PDFMarkSubject"), MAX_INFO_FIELD);
  strncpy(params->keywords, read_config_str("PDFMarkKeyWords"), MAX_INFO_FIELD);

  strncpy(params->userfile, read_config_str("PDFMarkUserFile"), MAX_PDFMARK_FILENAME);

}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_pdfmark_dialogue (pdfmark_params *params, wimp_pointer *pointer)
{
  extern global_windows windows;

  /* Set the dialogue icons. */

  strcpy (indirected_icon_text (windows.pdfmark, PDFMARK_ICON_TITLE), params->title);
  strcpy (indirected_icon_text (windows.pdfmark, PDFMARK_ICON_AUTHOR), params->author);
  strcpy (indirected_icon_text (windows.pdfmark, PDFMARK_ICON_SUBJECT), params->subject);
  strcpy (indirected_icon_text (windows.pdfmark, PDFMARK_ICON_KEYWORDS), params->keywords);
  strcpy (indirected_icon_text (windows.pdfmark, PDFMARK_ICON_USERFILE), params->userfile);

  shade_pdfmark_dialogue ();

  open_transient_window_centred_at_pointer (windows.pdfmark, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_pdfmark_dialogue (pdfmark_params *params)
{
  extern global_windows windows;

  strncpy(params->title, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_TITLE), MAX_INFO_FIELD);
  strncpy(params->author, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_AUTHOR), MAX_INFO_FIELD);
  strncpy(params->subject, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_SUBJECT), MAX_INFO_FIELD);
  strncpy(params->keywords, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_KEYWORDS), MAX_INFO_FIELD);

  strncpy(params->userfile, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_USERFILE), MAX_PDFMARK_FILENAME);

  wimp_create_menu ((wimp_menu *) -1, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void shade_pdfmark_dialogue (void)
{
  extern global_windows windows;

  replace_caret_in_window (windows.pdfmark);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void fill_pdfmark_field (wimp_w window, wimp_i icon, pdfmark_params *params)
{
  msgs_lookup ("Custom", indirected_icon_text (window, icon), 20);

  wimp_set_icon_state (window, icon, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void write_pdfmark_file (char *filename, pdfmark_params *params)
{
  FILE *pdfmark_file;

  if (*(params->title) != '\0' || *(params->author) != '\0' || *(params->subject) != '\0' || *(params->keywords))
  {
    pdfmark_file = fopen (filename, "w");
    if (pdfmark_file != NULL)
    {
      if (*(params->title) != '\0' || *(params->author) != '\0' || *(params->subject) != '\0' || *(params->keywords))
      {
        fprintf (pdfmark_file, "[");

        if (*(params->title) != '\0')
        {
          fprintf (pdfmark_file, " /Title (%s)", params->title);
        }

        if (*(params->author) != '\0')
        {
          fprintf (pdfmark_file, " /Author (%s)", params->author);
        }

        if (*(params->subject) != '\0')
        {
          fprintf (pdfmark_file, " /Subject (%s)", params->subject);
        }

        if (*(params->keywords) != '\0')
        {
          fprintf (pdfmark_file, " /Keywords (%s)", params->keywords);
        }

        fprintf (pdfmark_file, " /DOCINFO pdfmark\n");
      }
      fclose (pdfmark_file);
    }
  }
}
