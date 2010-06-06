/* PrintPDF - pdfmark.c
 *
 * (C) Stephen Fryatt, 2010
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/osbyte.h"
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

/* Lookup table to convert Acorn Latin1 into PDFDocEncoding. */

static char latin1_to_pdfdocencoding[] = {
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040,
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040,
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040,
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040,
  0040, 0041, 0042, 0043, 0044, 0045, 0046, 0047,
  0050, 0051, 0052, 0053, 0054, 0055, 0056, 0057,
  0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,
  0070, 0071, 0072, 0073, 0074, 0075, 0076, 0077,
  0100, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
  0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
  0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127,
  0130, 0131, 0132, 0133, 0134, 0135, 0136, 0137,
  0140, 0141, 0142, 0143, 0144, 0145, 0146, 0147,
  0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157,
  0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167,
  0170, 0171, 0172, 0173, 0174, 0175, 0176, 0177,
  0240, 0127, 0167, 0040, 0040, 0131, 0171, 0040,
  0040, 0040, 0040, 0040, 0203, 0222, 0213, 0200,
  0217, 0220, 0210, 0211, 0215, 0216, 0214, 0205,
  0204, 0212, 0226, 0234, 0201, 0202, 0223, 0224,
  0040, 0241, 0242, 0243, 0244, 0245, 0246, 0247,
  0250, 0251, 0252, 0253, 0254, 0055, 0256, 0257,
  0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,
  0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,
  0300, 0301, 0302, 0303, 0304, 0305, 0306, 0307,
  0310, 0311, 0312, 0313, 0314, 0315, 0316, 0317,
  0320, 0321, 0322, 0323, 0324, 0325, 0326, 0327,
  0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,
  0340, 0341, 0342, 0343, 0344, 0345, 0346, 0347,
  0350, 0351, 0352, 0353, 0354, 0355, 0356, 0357,
  0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,
  0370, 0371, 0372, 0373, 0374, 0375, 0376, 0377
};

/* ==================================================================================================================
 * Function prototypes.
 */


/* ==================================================================================================================
 *
 */

void initialise_pdfmark_settings (pdfmark_params *params)
{
  strcpy(params->title, read_config_str("PDFMarkTitle"));
  strcpy(params->author, read_config_str("PDFMarkAuthor"));
  strcpy(params->subject, read_config_str("PDFMarkSubject"));
  strcpy(params->keywords, read_config_str("PDFMarkKeyWords"));
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

  shade_pdfmark_dialogue ();

  open_transient_window_centred_at_pointer (windows.pdfmark, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_pdfmark_dialogue (pdfmark_params *params)
{
  extern global_windows windows;

  strcpy(params->title, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_TITLE));
  strcpy(params->author, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_AUTHOR));
  strcpy(params->subject, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_SUBJECT));
  strcpy(params->keywords, indirected_icon_text (windows.pdfmark, PDFMARK_ICON_KEYWORDS));

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
  if (pdfmark_data_available(params))
  {
    /* Here we would look for Info and/or Bookmarks and use Info, BMark or InfoBM accordingly. */
    msgs_lookup ("Info", indirected_icon_text (window, icon), 20);
  }
  else
  {
    msgs_lookup ("None", indirected_icon_text (window, icon), 20);
  }

  wimp_set_icon_state (window, icon, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void write_pdfmark_docinfo_file (FILE *pdfmark_file, pdfmark_params *params)
{
  char buffer[MAX_INFO_FIELD * 4];

  if (pdfmark_file != NULL && pdfmark_data_available(params))
  {
    fprintf (pdfmark_file, "[");

    if (*(params->title) != '\0')
    {
      fprintf (pdfmark_file, " /Title (%s)", convert_to_pdf_doc_encoding(buffer, params->title, MAX_INFO_FIELD * 4));
    }

    if (*(params->author) != '\0')
    {
      fprintf (pdfmark_file, " /Author (%s)", convert_to_pdf_doc_encoding(buffer, params->author, MAX_INFO_FIELD * 4));
    }

    if (*(params->subject) != '\0')
    {
      fprintf (pdfmark_file, " /Subject (%s)", convert_to_pdf_doc_encoding(buffer, params->subject, MAX_INFO_FIELD * 4));
    }

    if (*(params->keywords) != '\0')
    {
      fprintf (pdfmark_file, " /Keywords (%s)", convert_to_pdf_doc_encoding(buffer, params->keywords, MAX_INFO_FIELD * 4));
    }

    fprintf (pdfmark_file, " /DOCINFO pdfmark\n");
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

int pdfmark_data_available (pdfmark_params *params)
{
  return (*(params->title) != '\0' || *(params->author) != '\0' || *(params->subject) != '\0' || *(params->keywords));
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *convert_to_pdf_doc_encoding(char* out, char *in, int len)
{
  int  alphabet;
  char c, *ci, *co;

  alphabet = osbyte1(osbyte_ALPHABET_NUMBER, 127, 0);

  ci = in;
  co = out;

  while ((*ci != '\0') && ((co - out) < len))
  {
    switch (alphabet)
    {
    case 101: /* Latin 1, or catch-all defualt. */
    default:
      c = latin1_to_pdfdocencoding[(*ci++) % 256];
      break;
    }

    /* 'Standard' characters in range 32 to 126 go through as a single byte;
     * anything else is escaped in octal.
     */

    if (c >= 32 && c < 127 && c != '(' && c != ')')
    {
      *co++ = c;
    }
    else
    {
      co += snprintf(co, ( - (co - out + 1)), "\\%03o", (unsigned int) c);
    }
  }

  *co = '\0';

  return (out);
}
