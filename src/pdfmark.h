/* PrintPDF - pdfmark.h
 *
 * (c) Stephen Fryatt, 2010-2011
 */

#ifndef PRINTPDF_PDFMARK
#define PRINTPDF_PDFMARK


#define MAX_INFO_FIELD 255
#define MAX_PDFMARK_FILENAME 256


typedef struct pdfmark_params {
	char		title[MAX_INFO_FIELD];
	char		author[MAX_INFO_FIELD];
	char		subject[MAX_INFO_FIELD];
	char		keywords[MAX_INFO_FIELD];
} pdfmark_params;


/**
 * Initialise the PDFMark dialogue.
 */

void pdfmark_initialise(void);


/**
 * Initialise the values in a PDFMark settings structure.
 *
 * \param *params		The encryption params struct to be initialised.
 */

void pdfmark_initialise_settings(pdfmark_params *params);


/**
 * Set a callback handler to be called when the OK button of the
 * PDFMark dialogue is clicked.
 *
 * \param callback		The callback function to use, or NULL.
 */

void pdfmark_set_dialogue_callback(void (*callback)(void));


/**
 * Open the PDFMark dialogue for the given parameter block.
 *
 * \param *params		The encryption parameter block to be used.
 * \param *pointer		The current pointer state.
 */

void pdfmark_open_dialogue(pdfmark_params *params, wimp_pointer *pointer);


/**
 * Store the settings from the currently open PDFMark dialogue box in
 * an PDFMark parameter block.
 *
 * \param *params		The PDFMark parameter block to be used.
 */

void pdfmark_process_dialogue(pdfmark_params *params);


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given PDFMark parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The PDFMark parameter block to use.
 */

void pdfmark_fill_field(wimp_w window, wimp_i icon, pdfmark_params *params);


/**
 * Write document info to a PSDMark file, reflecting the data in the supplied
 * PDFMark parameter block.
 *
 * \param *file			The file handle to write to.
 * \param *params		The PRDMark parameter block to translate.
 */

void pdfmark_write_docinfo_file(FILE *pdfmark_file, pdfmark_params *params);


/**
 * Test if a PDFMark parameter block would result in any PDFMark data being generaated.
 *
 * \param *params		The PDFMark parameter block to test.
 * \return			TRUE if data would be generated; else FALSE.
 */

osbool pdfmark_data_available (pdfmark_params *params);


/**
 * Convert a string from the current RISC OS system alphabet into PDFDocEncoding.
 *
 * \param *out			A buffer to accept the converted string.
 * \param *in			A buffer containing the string to convert.
 * \param len			The size of the output buffer.
 * \return			A pointer to the output buffer.
 */

char *convert_to_pdf_doc_encoding(char *out, char *in, size_t len);

#endif

