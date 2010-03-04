/* PrintPDF - pdfmark.h
 *
 * (c) Stephen Fryatt, 2010
 */

#ifndef _PRINTPDF_PDFMARK
#define _PRINTPDF_PDFMARK

/* ==================================================================================================================
 * Static constants
 */

/* Optimization Window icons. */

#define PDFMARK_ICON_CANCEL 0
#define PDFMARK_ICON_OK 1

#define PDFMARK_ICON_TITLE 5
#define PDFMARK_ICON_AUTHOR 7
#define PDFMARK_ICON_SUBJECT 9
#define PDFMARK_ICON_KEYWORDS 11

#define MAX_INFO_FIELD 255
#define MAX_PDFMARK_FILENAME 256

/* ==================================================================================================================
 * Data structures
 */

typedef struct pdfmark_params
{
  char title[MAX_INFO_FIELD];
  char author[MAX_INFO_FIELD];
  char subject[MAX_INFO_FIELD];
  char keywords[MAX_INFO_FIELD];

  char userfile[MAX_PDFMARK_FILENAME];
}
pdfmark_params;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Handle the PDFMark window and menu. */


void initialise_pdfmark_settings (pdfmark_params *params);
void open_pdfmark_dialogue (pdfmark_params *params, wimp_pointer *pointer);
void process_pdfmark_dialogue (pdfmark_params *params);
void shade_pdfmark_dialogue (void);
void fill_pdfmark_field (wimp_w window, wimp_i icon, pdfmark_params *params);
void write_pdfmark_file (char *filename, pdfmark_params *params);
int pdfmark_data_available (pdfmark_params *params);

#endif
