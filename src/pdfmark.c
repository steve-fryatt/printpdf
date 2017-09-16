/* Copyright 2010-2017, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.1 only (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 *
 * You may obtain a copy of the Licence at:
 *
 *   http://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

/**
 * \file: pdfmark.c
 *
 * PDFMark character set support.
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
#include "sflib/debug.h"
#include "sflib/event.h"
#include "sflib/icons.h"
#include "sflib/ihelp.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "pdfmark.h"

#include "pmenu.h"


/* PDFMark Window icons. */

#define PDFMARK_ICON_CANCEL 0
#define PDFMARK_ICON_OK 1

#define PDFMARK_ICON_TITLE 5
#define PDFMARK_ICON_AUTHOR 7
#define PDFMARK_ICON_SUBJECT 9
#define PDFMARK_ICON_KEYWORDS 11


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

static wimp_w	pdfmark_window = NULL;

static void	(*pdfmark_dialogue_close_callback)(void) = NULL;


static void		pdfmark_click_handler(wimp_pointer *pointer);
static osbool		pdfmark_keypress_handler(wimp_key *key);

static void		pdfmark_shade_dialogue(void);


/**
 * Initialise the PDFMark dialogue.
 */

void pdfmark_initialise(void)
{
	pdfmark_window = templates_create_window("PDFMark");
	ihelp_add_window(pdfmark_window, "PDFMark", NULL);

	event_add_window_mouse_event(pdfmark_window, pdfmark_click_handler);
	event_add_window_key_event(pdfmark_window, pdfmark_keypress_handler);
}


/**
 * Initialise the values in a PDFMark settings structure.
 *
 * \param *params		The encryption params struct to be initialised.
 */

void pdfmark_initialise_settings(pdfmark_params *params)
{
	string_copy(params->title, config_str_read("PDFMarkTitle"), MAX_INFO_FIELD);
	string_copy(params->author, config_str_read("PDFMarkAuthor"), MAX_INFO_FIELD);
	string_copy(params->subject, config_str_read("PDFMarkSubject"), MAX_INFO_FIELD);
	string_copy(params->keywords, config_str_read("PDFMarkKeyWords"), MAX_INFO_FIELD);
}


/**
 * Save the settings from a PDFMark settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The PDFMark params struct to be saved.
 */

void pdfmark_save_settings(pdfmark_params *params)
{
	config_str_set("PDFMarkTitle", params->title);
	config_str_set("PDFMarkAuthor", params->author);
	config_str_set("PDFMarkSubject", params->subject);
	config_str_set("PDFMarkKeywords", params->keywords);
}


/**
 * Set a callback handler to be called when the OK button of the
 * PDFMark dialogue is clicked.
 *
 * \param callback		The callback function to use, or NULL.
 */

void pdfmark_set_dialogue_callback(void (*callback)(void))
{
	pdfmark_dialogue_close_callback = callback;
}


/**
 * Process mouse clicks in the PDFMark dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void pdfmark_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case PDFMARK_ICON_CANCEL:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;

	case PDFMARK_ICON_OK:
		if (pdfmark_dialogue_close_callback != NULL)
			pdfmark_dialogue_close_callback();
		break;
	}
}


/**
 * Process keypresses in the PDFMark window.
 *
 * \param *key		The keypress event block to handle.
 * \return		TRUE if the event was handled; else FALSE.
 */

static osbool pdfmark_keypress_handler(wimp_key *key)
{
	if (key == NULL)
		return FALSE;

	switch (key->c) {
	case wimp_KEY_RETURN:
		if (pdfmark_dialogue_close_callback != NULL)
			pdfmark_dialogue_close_callback();
		break;

	case wimp_KEY_ESCAPE:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}


/**
 * Open the PDFMark dialogue for the given parameter block.
 *
 * \param *params		The PDFMark parameter block to be used.
 * \param *pointer		The current pointer state.
 */

void pdfmark_open_dialogue(pdfmark_params *params, wimp_pointer *pointer)
{
	icons_strncpy(pdfmark_window, PDFMARK_ICON_TITLE, params->title);
	icons_strncpy(pdfmark_window, PDFMARK_ICON_AUTHOR, params->author);
	icons_strncpy(pdfmark_window, PDFMARK_ICON_SUBJECT, params->subject);
	icons_strncpy(pdfmark_window, PDFMARK_ICON_KEYWORDS, params->keywords);

	pdfmark_shade_dialogue();

	windows_open_transient_centred_at_pointer(pdfmark_window, pointer);
}


/**
 * Store the settings from the currently open PDFMark dialogue box in
 * an PDFMark parameter block.
 *
 * \param *params		The PDFMark parameter block to be used.
 */

void pdfmark_process_dialogue(pdfmark_params *params)
{
	icons_copy_text(pdfmark_window, PDFMARK_ICON_TITLE, params->title, MAX_INFO_FIELD);
	icons_copy_text(pdfmark_window, PDFMARK_ICON_AUTHOR, params->author, MAX_INFO_FIELD);
	icons_copy_text(pdfmark_window, PDFMARK_ICON_SUBJECT, params->subject, MAX_INFO_FIELD);
	icons_copy_text(pdfmark_window, PDFMARK_ICON_KEYWORDS, params->keywords, MAX_INFO_FIELD);

	wimp_create_menu((wimp_menu *) -1, 0, 0);
}


/**
 * Update the shading in the PDFMark dialogue, based on the current icon
 * selections
 */

static void pdfmark_shade_dialogue(void)
{
	icons_replace_caret_in_window(pdfmark_window);
}


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given PDFMark parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The PDFMark parameter block to use.
 */

void pdfmark_fill_field(wimp_w window, wimp_i icon, pdfmark_params *params)
{
	if (pdfmark_data_available(params))
		/* \TODO -- Here we should look for Info and/or Bookmarks and use Info, BMark or InfoBM accordingly. */
		icons_msgs_lookup(window, icon, "Info");
	else
		icons_msgs_lookup(window, icon, "None");

	wimp_set_icon_state(window, icon, 0, 0);
}


/**
 * Write document info to a PSDMark file, reflecting the data in the supplied
 * PDFMark parameter block.
 *
 * \param *file			The file handle to write to.
 * \param *params		The PRDMark parameter block to translate.
 */

void pdfmark_write_docinfo_file(FILE *pdfmark_file, pdfmark_params *params)
{
	char buffer[MAX_INFO_FIELD * 4];

	if (pdfmark_file == NULL || params == NULL || !pdfmark_data_available(params))
		return;


	fprintf(pdfmark_file, "[");

	if (*(params->title) != '\0')
		fprintf(pdfmark_file, " /Title (%s)", convert_to_pdf_doc_encoding(buffer, params->title, MAX_INFO_FIELD * 4));

	if (*(params->author) != '\0')
		fprintf(pdfmark_file, " /Author (%s)", convert_to_pdf_doc_encoding(buffer, params->author, MAX_INFO_FIELD * 4));

	if (*(params->subject) != '\0')
		fprintf(pdfmark_file, " /Subject (%s)", convert_to_pdf_doc_encoding(buffer, params->subject, MAX_INFO_FIELD * 4));

	if (*(params->keywords) != '\0')
		fprintf(pdfmark_file, " /Keywords (%s)", convert_to_pdf_doc_encoding(buffer, params->keywords, MAX_INFO_FIELD * 4));

	fprintf(pdfmark_file, " /DOCINFO pdfmark\n");
}


/**
 * Test if a PDFMark parameter block would result in any PDFMark data being generaated.
 *
 * \param *params		The PDFMark parameter block to test.
 * \return			TRUE if data would be generated; else FALSE.
 */

osbool pdfmark_data_available(pdfmark_params *params)
{
	return (*(params->title) != '\0' || *(params->author) != '\0' ||
			*(params->subject) != '\0' || *(params->keywords)) ? TRUE : FALSE;
}


/**
 * Convert a string from the current RISC OS system alphabet into PDFDocEncoding.
 *
 * \param *out			A buffer to accept the converted string.
 * \param *in			A buffer containing the string to convert.
 * \param len			The size of the output buffer.
 * \return			A pointer to the output buffer.
 */

char *convert_to_pdf_doc_encoding(char *out, char *in, size_t len)
{
	int		alphabet;
	char		c, *ci, *co;

	alphabet = osbyte1(osbyte_ALPHABET_NUMBER, 127, 0);

	ci = in;
	co = out;

	while ((*ci != '\0') && ((co - out) < len)) {
		switch (alphabet) {
		case 101: /* Latin 1, or catch-all defualt. */
		default:
			c = latin1_to_pdfdocencoding[(*ci++) % 256];
			break;
		}

		/* 'Standard' characters in range 32 to 126 go through as a single byte;
		 * anything else is escaped in octal.
		 */

		if (c >= 32 && c < 127 && c != '(' && c != ')')
			*co++ = c;
		else
			co += snprintf(co, (len - (co - out + 1)), "\\%03o", (unsigned int) c);
	}

	*co = '\0';

	return out;
}

