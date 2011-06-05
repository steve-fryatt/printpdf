/* PrintPDF - version.h
 *
 * (c) Stephen Fryatt, 2007-2011
 */

#ifndef PRINTPDF_VERSION
#define PRINTPDF_VERSION


typedef struct version_params {
	int		standard_version;
} version_params;


/**
 * Initialise the values in a version settings structure.
 *
 * \param *params		The version params struct to be initialised.
 */

void version_initialise_settings(version_params *params);


/**
 * Save the settings from a version settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The version params struct to be saved.
 */

void version_save_settings(version_params *params);


/**
 * Build and open the PDF version menu.
 *
 * \param *params		The version parameter block to use for the menu.
 * \param *pointer		The Wimp pointer details.
 * \param window		The window to open the menu over.
 * \param icon			The icon to open the menu over.
 * \param ident			The param menu ident.
 */

void version_open_menu(version_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon, int ident);


/**
 * Handle selections from the PDF version menu.
 *
 * \param *params		The version parameter block for the menu.
 * \param *selection		The menu selection details.
 */

void version_process_menu(version_params *params, wimp_selection *selection);


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given PDF version parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The version parameter block to use.
 */

void version_fill_field (wimp_w window, wimp_i icon, version_params *params);


/**
 * Build up a text string in the supplied buffer containing the GS
 * parameters that reflect the contents of the given optimization
 * parameter block.
 *
 * \param *buffer		Buffer to hold the result.
 * \param len			The size of the buffer.
 * \param *params		The optimization parameter block to translate.
 */

void version_build_params(char *buffer, size_t len, version_params *params);

#endif

