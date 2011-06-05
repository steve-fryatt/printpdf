/* PrintPDF - optimize.h
 *
 * (c) Stephen Fryatt, 2007-2011
 */

#ifndef PRINTPDF_OPTIMIZE
#define PRINTPDF_OPTIMIZE

/* ==================================================================================================================
 * Static constants
 */

/* Optimization Window icons. */

#define OPTIMIZE_ICON_CANCEL 0
#define OPTIMIZE_ICON_OK 1

#define OPTIMIZE_ICON_COLOUR_DOWNSAMPLE 4
#define OPTIMIZE_ICON_GREY_DOWNSAMPLE 16
#define OPTIMIZE_ICON_MONO_DOWNSAMPLE 28
#define OPTIMIZE_ICON_COLOUR_SUBSAMPLE 5
#define OPTIMIZE_ICON_GREY_SUBSAMPLE 17
#define OPTIMIZE_ICON_MONO_SUBSAMPLE 29
#define OPTIMIZE_ICON_COLOUR_AVERAGE 6
#define OPTIMIZE_ICON_GREY_AVERAGE 18
#define OPTIMIZE_ICON_MONO_AVERAGE 30
#define OPTIMIZE_ICON_COLOUR_RESOLUTION 7
#define OPTIMIZE_ICON_GREY_RESOLUTION 19
#define OPTIMIZE_ICON_MONO_RESOLUTION 31
#define OPTIMIZE_ICON_COLOUR_RESOLUTION_UP 9
#define OPTIMIZE_ICON_GREY_RESOLUTION_UP 21
#define OPTIMIZE_ICON_MONO_RESOLUTION_UP 33
#define OPTIMIZE_ICON_COLOUR_RESOLUTION_DOWN 8
#define OPTIMIZE_ICON_GREY_RESOLUTION_DOWN 20
#define OPTIMIZE_ICON_MONO_RESOLUTION_DOWN 32
#define OPTIMIZE_ICON_COLOUR_THRESHOLD 10
#define OPTIMIZE_ICON_GREY_THRESHOLD 22
#define OPTIMIZE_ICON_MONO_THRESHOLD 34
#define OPTIMIZE_ICON_COLOUR_THRESHOLD_UP 12
#define OPTIMIZE_ICON_GREY_THRESHOLD_UP 24
#define OPTIMIZE_ICON_MONO_THRESHOLD_UP 36
#define OPTIMIZE_ICON_COLOUR_THRESHOLD_DOWN 11
#define OPTIMIZE_ICON_GREY_THRESHOLD_DOWN 23
#define OPTIMIZE_ICON_MONO_THRESHOLD_DOWN 35
#define OPTIMIZE_ICON_COLOUR_DEPTH 13
#define OPTIMIZE_ICON_GREY_DEPTH 25
#define OPTIMIZE_ICON_MONO_DEPTH 37
#define OPTIMIZE_ICON_COLOUR_DEPTH_UP 15
#define OPTIMIZE_ICON_GREY_DEPTH_UP 27
#define OPTIMIZE_ICON_MONO_DEPTH_UP 39
#define OPTIMIZE_ICON_COLOUR_DEPTH_DOWN 14
#define OPTIMIZE_ICON_GREY_DEPTH_DOWN 26
#define OPTIMIZE_ICON_MONO_DEPTH_DOWN 38

#define OPTIMIZE_ICON_COLOUR_ENCODE 46
#define OPTIMIZE_ICON_GREY_ENCODE 49
#define OPTIMIZE_ICON_MONO_ENCODE 52
#define OPTIMIZE_ICON_COLOUR_DCT 47
#define OPTIMIZE_ICON_GREY_DCT 50
#define OPTIMIZE_ICON_MONO_CCITT 53
#define OPTIMIZE_ICON_COLOUR_FLATE 48
#define OPTIMIZE_ICON_GREY_FLATE 51
#define OPTIMIZE_ICON_MONO_FLATE 54
#define OPTIMIZE_ICON_MONO_RUNLENGTH 61

#define OPTIMIZE_ICON_ROTATE_NONE 57
#define OPTIMIZE_ICON_ROTATE_ALL 58
#define OPTIMIZE_ICON_ROTATE_PAGE 59

#define OPTIMIZE_ICON_COMPRESS 60


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
 * Build and open the optimization values menu.
 *
 * \param *params		The optimization parameter block to use for the menu.
 * \param *pointer		The Wimp pointer details.
 * \param window		The window to open the menu over.
 * \param icon			The icon to open the menu over.
 * \param ident			The param menu ident.
 */

void optimize_open_menu(optimize_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon, int ident);


/**
 * Handle selections from the optimize menu.
 *
 * \param *params		The optimization parameter block for the menu.
 * \param *selection		The menu selection details.
 */

void optimize_process_menu(optimize_params *params, wimp_selection *selection);


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

