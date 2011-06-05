/* PrintPDF - optimize.h
 *
 * (c) Stephen Fryatt, 2007-2011
 */

#ifndef PRINTPDF_OPTIMIZE
#define PRINTPDF_OPTIMIZE

typedef struct optimize_params {
	int		standard_preset;

	int		downsample_mono_images;
	int		downsample_mono_type;
	int		downsample_mono_resolution;
	int		downsample_mono_threshold;
	int		downsample_mono_depth;

	int		downsample_grey_images;
	int		downsample_grey_type;
	int		downsample_grey_resolution;
	int		downsample_grey_threshold;
	int		downsample_grey_depth;

	int		downsample_colour_images;
	int		downsample_colour_type;
	int		downsample_colour_resolution;
	int		downsample_colour_threshold;
	int		downsample_colour_depth;

	int		encode_mono_images;
	int		encode_mono_type;

	int		encode_grey_images;
	int		encode_grey_type;

	int		encode_colour_images;
	int		encode_colour_type;

	int		auto_page_rotation;

	int		compress_pages;
} optimize_params;


/**
 * Initialise the optimization dialogue.
 */

void optimize_initialise(void);


/**
 * Initialise the values in an optimization settings structure.
 *
 * \param *params		The optimisation params struct to be initialised.
 */

void optimize_initialise_settings(optimize_params *params);


/**
 * Save the settings from an optimization settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The optimisation params struct to be saved.
 */

void optimise_save_settings(optimize_params *params);


/**
 * Build and open the optimization values menu.
 *
 * \param *params		The optimization parameter block to use for the menu.
 * \param *menu			The version menu block.
 */

void optimize_set_menu(optimize_params *params, wimp_menu *menu);


/**
 * Handle selections from the optimize menu.
 *
 * \param *params		The optimization parameter block for the menu.
 * \param *menu			The version menu block.
 * \param *selection		The menu selection details.
 */

void optimize_process_menu(optimize_params *params, wimp_menu *menu, wimp_selection *selection);


/**
 * Set a callback handler to be called when the OK button of the
 * optimize dialogue is clicked.
 *
 * \param callback		The callback function to use, or NULL.
 */

void optimize_set_dialogue_callback(void (*callback)(void));


/**
 * Store the settings from the currently open Optimization dialogue box in
 * an optimization parameter block.
 *
 * \param *params		The optimization parameter block to be used.
 */

void optimize_process_dialogue(optimize_params *params);


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given optimization parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The optimization parameter block to use.
 */

void optimize_fill_field(wimp_w window, wimp_i icon, optimize_params *params);


/**
 * Build up a text string in the supplied buffer containing the GS
 * parameters that reflect the contents of the given optimization
 * parameter block.
 *
 * \param *buffer		Buffer to hold the result.
 * \param len			The size of the buffer.
 * \param *params		The optimization parameter block to translate.
 */

void optimize_build_params(char *buffer, size_t len, optimize_params *params);

#endif

