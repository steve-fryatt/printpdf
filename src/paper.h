/* Copyright 2007-2013, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/risc-os/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
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
 * \file: paper.h
 *
 * Paper dialogue implementation.
 */

#ifndef PRINTPDF_PAPER
#define PRINTPDF_PAPER

enum paper_units {
	PAPER_UNITS_MM = 0,
	PAPER_UNITS_INCH = 1,
	PAPER_UNITS_POINT = 2
};

typedef struct paper_params {
	osbool			override_document;	/**< TRUE to override the document's page size.		*/
	int			preset_size;		/**< Index into the list of Ghostscript paper sizes.	*/
	int			width;			/**< Custom page width.					*/
	int			height;			/**< Custom page height.				*/
	enum paper_units	units;			/**< Units for custom page dimensions.			*/
} paper_params;


/**
 * Initialise the paper dialogue.
 */

void paper_initialise(void);


/**
 * Initialise the values in a paper settings structure.
 *
 * \param *params		The version params struct to be initialised.
 */

void paper_initialise_settings(paper_params *params);


/**
 * Save the settings from a paper settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The paper params struct to be saved.
 */

void paper_save_settings(paper_params *params);


/**
 * Build and open the paper values menu.
 *
 * \param *params		The paper parameter block to use for the menu.
 * \param *menu			The paper menu block.
 */

void paper_set_menu(paper_params *params, wimp_menu *menu);


/**
 * Handle selections from the paper menu.
 *
 * \param *params		The paper parameter block for the menu.
 * \param *menu			The paper menu block.
 * \param *selection		The menu selection details.
 */

void paper_process_menu(paper_params *params, wimp_menu *menu, wimp_selection *selection);


/**
 * Set a callback handler to be called when the OK button of the
 * paper dialogue is clicked.
 *
 * \param callback		The callback function to use, or NULL.
 */

void paper_set_dialogue_callback(void (*callback)(void));


/**
 * Open the paper dialogue for the given parameter block.
 *
 * \param *params		The paper parameter block to be used.
 * \param *pointer		The current pointer state.
 */

void paper_open_dialogue(paper_params *params, wimp_pointer *pointer);


/**
 * Store the settings from the currently open paper dialogue box in
 * a paper parameter block.
 *
 * \param *params		The paper parameter block to be used.
 */

void paper_process_dialogue(paper_params *params);


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given paper parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The paper parameter block to use.
 */

void paper_fill_field(wimp_w window, wimp_i icon, paper_params *params);


/**
 * Build up a text string in the supplied buffer containing the GS
 * parameters that reflect the contents of the given paper
 * parameter block.
 *
 * \param *buffer		Buffer to hold the result.
 * \param len			The size of the buffer.
 * \param *params		The paper parameter block to translate.
 */

void paper_build_params(char *buffer, size_t len, paper_params *params);

#endif

