/* PrintPDF - optimize.c
 *
 * (C) Stephen Fryatt, 2005-2011
 */

/* ANSI C header files */

#include <string.h>
#include <stdlib.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/event.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/windows.h"

/* Application header files */

#include "optimize.h"

#include "ihelp.h"
#include "pmenu.h"
#include "templates.h"


#define OPTIMIZE_MENU_LENGTH 6

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


static wimp_w	optimize_window = NULL;

static int	downsample_mono_depth;
static int	downsample_grey_depth;
static int	downsample_colour_depth;

static void	(*optimize_dialogue_close_callback)(void) = NULL;

/* Function Prototypes. */

static void	optimize_click_handler(wimp_pointer *pointer);
static osbool	optimize_keypress_handler(wimp_key *key);

static int	optimize_tick_menu(optimize_params *params);
static void	optimize_open_dialogue(optimize_params *params, wimp_pointer *pointer);
static bool	optimize_shade_dialogue(wimp_pointer *pointer);

static char	*optimize_true_false(osbool value);
static char	*optimize_depth_text(char *buffer, int value);

static void	optimize_update_resolution_icon(wimp_i i, int dir);
static void	optimize_update_threshold_icon(wimp_i i, int dir);
static void	optimize_update_depth_icon(wimp_i i, int dir);

/**
 * Initialise the optimization dialogue.
 */

void optimize_initialise(void)
{
	optimize_window = templates_create_window("Optimize");
	ihelp_add_window(optimize_window, "Optimize", NULL);

	event_add_window_mouse_event(optimize_window, optimize_click_handler);
	event_add_window_key_event(optimize_window, optimize_keypress_handler);

	event_add_window_icon_click(optimize_window, OPTIMIZE_ICON_COLOUR_DOWNSAMPLE, optimize_shade_dialogue);
	event_add_window_icon_click(optimize_window, OPTIMIZE_ICON_GREY_DOWNSAMPLE, optimize_shade_dialogue);
	event_add_window_icon_click(optimize_window, OPTIMIZE_ICON_MONO_DOWNSAMPLE, optimize_shade_dialogue);
	event_add_window_icon_click(optimize_window, OPTIMIZE_ICON_COLOUR_ENCODE, optimize_shade_dialogue);
	event_add_window_icon_click(optimize_window, OPTIMIZE_ICON_GREY_ENCODE, optimize_shade_dialogue);
	event_add_window_icon_click(optimize_window, OPTIMIZE_ICON_MONO_ENCODE, optimize_shade_dialogue);

	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_COLOUR_SUBSAMPLE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_GREY_SUBSAMPLE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_MONO_SUBSAMPLE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_COLOUR_AVERAGE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_GREY_AVERAGE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_MONO_AVERAGE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_COLOUR_DCT);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_GREY_DCT);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_MONO_CCITT);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_COLOUR_FLATE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_GREY_FLATE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_MONO_FLATE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_MONO_RUNLENGTH);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_ROTATE_NONE);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_ROTATE_ALL);
	event_add_window_icon_radio(optimize_window, OPTIMIZE_ICON_ROTATE_PAGE);
}


/**
 * Initialise the values in an optimization settings structure.
 *
 * \param *params		The optimisation params struct to be initialised.
 */

void optimize_initialise_settings(optimize_params *params)
{
	params->standard_preset = config_int_read("Optimization");

	params->downsample_mono_images = config_opt_read("DownsampleMono");
	params->downsample_mono_type = config_int_read("DownsampleMonoType");
	params->downsample_mono_resolution = config_int_read("DownsampleMonoResolution");
	params->downsample_mono_threshold = config_int_read("DownsampleMonoThreshold");
	params->downsample_mono_depth = config_int_read("DownsampleMonoDepth");

	params->downsample_grey_images = config_opt_read("DownsampleGrey");
	params->downsample_grey_type = config_int_read("DownsampleGreyType");
	params->downsample_grey_resolution = config_int_read("DownsampleGreyResolution");
	params->downsample_grey_threshold = config_int_read("DownsampleGreyThreshold");
	params->downsample_grey_depth = config_int_read("DownsampleGreyDepth");

	params->downsample_colour_images = config_opt_read("DownsampleColour");
	params->downsample_colour_type = config_int_read("DownsampleColourType");
	params->downsample_colour_resolution = config_int_read("DownsampleColourResolution");
	params->downsample_colour_threshold = config_int_read("DownsampleColourThreshold");
	params->downsample_colour_depth = config_int_read("DownsampleColourDepth");

	params->encode_mono_images = config_opt_read("EncodeMono");
	params->encode_mono_type = config_int_read("EncodeMonoType");

	params->encode_grey_images = config_opt_read("EncodeGrey");
	params->encode_grey_type = config_int_read("EncodeGreyType");

	params->encode_colour_images = config_opt_read("EncodeColour");
	params->encode_colour_type = config_int_read("EncodeColourType");

	params->auto_page_rotation = config_int_read("AutoPageRotation");

	params->compress_pages = config_opt_read("CompressPages");
}


/**
 * Save the settings from an optimization settings structure back into the
 * corresponding config settings.
 *
 * \param *param		The optimisation params struct to be saved.
 */

void optimise_save_settings(optimize_params *params)
{
	config_int_set("Optimization", params->standard_preset);

	config_opt_set("DownsampleMono", params->downsample_mono_images);
	config_int_set("DownsampleMonoType", params->downsample_mono_type);
	config_int_set("DownsampleMonoResolution", params->downsample_mono_resolution);
	config_int_set("DownsampleMonoThreshold", params->downsample_mono_threshold);
	config_int_set("DownsampleMonoDepth", params->downsample_mono_depth);

	config_opt_set("DownsampleGrey", params->downsample_grey_images);
	config_int_set("DownsampleGreyType", params->downsample_grey_type);
	config_int_set("DownsampleGreyResolution", params->downsample_grey_resolution);
	config_int_set("DownsampleGreyThreshold", params->downsample_grey_threshold);
	config_int_set("DownsampleGreyDepth", params->downsample_grey_depth);

	config_opt_set("DownsampleColour", params->downsample_colour_images);
	config_int_set("DownsampleColourType", params->downsample_colour_type);
	config_int_set("DownsampleColourResolution", params->downsample_colour_resolution);
	config_int_set("DownsampleColourThreshold", params->downsample_colour_threshold);
	config_int_set("DownsampleColourDepth", params->downsample_colour_depth);

	config_opt_set("EncodeMono", params->encode_mono_images);
	config_int_set("EncodeMonoType", params->encode_mono_type);

	config_opt_set("EncodeGrey", params->encode_grey_images);
	config_int_set("EncodeGreyType", params->encode_grey_type);

	config_opt_set("EncodeColour", params->encode_colour_images);
	config_int_set("EncodeColourType", params->encode_colour_type);

	config_int_set("AutoPageRotation", params->auto_page_rotation);

	config_opt_set("CompressPages", params->compress_pages);
}


/**
 * Build and open the optimization values menu.
 *
 * \param *params		The optimization parameter block to use for the menu.
 * \param *menu			The version menu block.
 */

void optimize_set_menu(optimize_params *params, wimp_menu *menu)
{
	int		i;

	for (i = 0; i < OPTIMIZE_MENU_LENGTH; i++)
		menus_tick_entry(menu, i, i == optimize_tick_menu(params));
}


/**
 * Handle selections from the optimize menu.
 *
 * \param *params		The optimization parameter block for the menu.
 * \param *menu			The version menu block.
 * \param *selection		The menu selection details.
 */

void optimize_process_menu(optimize_params *params, wimp_menu *menu, wimp_selection *selection)
{
	wimp_pointer		pointer;

	if (selection->items[0] == OPTIMIZE_MENU_LENGTH - 1) {
		wimp_get_pointer_info(&pointer);
		optimize_open_dialogue(params, &pointer);
	} else {
		params->standard_preset = selection->items[0];
	}
}


/**
 * Return the number of the menu item which should be ticked based on the
 * supplied parameter block.
 *
 * \param *params		The optimization parameter block to be used.
 * \return			The menu entry to be ticked.
 */

static int optimize_tick_menu(optimize_params *params)
{
	int		item;

	if (params->standard_preset == -1)
		item = OPTIMIZE_MENU_LENGTH - 1;
	else
		item = params->standard_preset;

	return item;
}


/**
 * Set a callback handler to be called when the OK button of the
 * optimize dialogue is clicked.
 *
 * \param callback		The callback function to use, or NULL.
 */

void optimize_set_dialogue_callback(void (*callback)(void))
{
	optimize_dialogue_close_callback = callback;
}


/**
 * Process mouse clicks in the Optimization dialogue.
 *
 * \param *pointer		The mouse event block to handle.
 */

static void optimize_click_handler(wimp_pointer *pointer)
{
	if (pointer == NULL)
		return;

	switch ((int) pointer->i) {
	case OPTIMIZE_ICON_CANCEL:
		wimp_create_menu((wimp_menu *) -1, 0, 0);
		break;

	case OPTIMIZE_ICON_OK:
		if (optimize_dialogue_close_callback != NULL)
			optimize_dialogue_close_callback();
		break;

	case OPTIMIZE_ICON_COLOUR_RESOLUTION_UP:
		optimize_update_resolution_icon(OPTIMIZE_ICON_COLOUR_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_COLOUR_RESOLUTION_DOWN:
		optimize_update_resolution_icon(OPTIMIZE_ICON_COLOUR_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_GREY_RESOLUTION_UP:
		optimize_update_resolution_icon(OPTIMIZE_ICON_GREY_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_GREY_RESOLUTION_DOWN:
		optimize_update_resolution_icon(OPTIMIZE_ICON_GREY_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_MONO_RESOLUTION_UP:
		optimize_update_resolution_icon(OPTIMIZE_ICON_MONO_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_MONO_RESOLUTION_DOWN:
		optimize_update_resolution_icon(OPTIMIZE_ICON_MONO_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_COLOUR_THRESHOLD_UP:
		optimize_update_threshold_icon(OPTIMIZE_ICON_COLOUR_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_COLOUR_THRESHOLD_DOWN:
		optimize_update_threshold_icon(OPTIMIZE_ICON_COLOUR_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_GREY_THRESHOLD_UP:
		optimize_update_threshold_icon(OPTIMIZE_ICON_GREY_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_GREY_THRESHOLD_DOWN:
		optimize_update_threshold_icon(OPTIMIZE_ICON_GREY_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_MONO_THRESHOLD_UP:
		optimize_update_threshold_icon(OPTIMIZE_ICON_MONO_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_MONO_THRESHOLD_DOWN:
		optimize_update_threshold_icon(OPTIMIZE_ICON_MONO_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_COLOUR_DEPTH_UP:
		optimize_update_depth_icon(OPTIMIZE_ICON_COLOUR_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_COLOUR_DEPTH_DOWN:
		optimize_update_depth_icon(OPTIMIZE_ICON_COLOUR_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_GREY_DEPTH_UP:
		optimize_update_depth_icon(OPTIMIZE_ICON_GREY_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_GREY_DEPTH_DOWN:
		optimize_update_depth_icon(OPTIMIZE_ICON_GREY_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;

	case OPTIMIZE_ICON_MONO_DEPTH_UP:
		optimize_update_depth_icon(OPTIMIZE_ICON_MONO_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
		break;

	case OPTIMIZE_ICON_MONO_DEPTH_DOWN:
		optimize_update_depth_icon(OPTIMIZE_ICON_MONO_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
		break;
	}
}


/**
 * Process keypresses in the Optimization window.
 *
 * \param *key		The keypress event block to handle.
 * \return		TRUE if the event was handled; else FALSE.
 */

static osbool optimize_keypress_handler(wimp_key *key)
{
	if (key == NULL)
		return FALSE;

	switch (key->c) {
	case wimp_KEY_RETURN:
		if (optimize_dialogue_close_callback != NULL)
			optimize_dialogue_close_callback();
		break;

	case wimp_KEY_ESCAPE:
		wimp_create_menu ((wimp_menu *) -1, 0, 0);
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}


/**
 * Open the Optimize dialogue for the given parameter block.
 *
 * \param *params		The optimization parameter block to be used.
 * \param *pointer		The current pointer state.
 */

static void optimize_open_dialogue(optimize_params *params, wimp_pointer *pointer)
{
	icons_set_selected(optimize_window, OPTIMIZE_ICON_COLOUR_DOWNSAMPLE, params->downsample_colour_images);
	icons_set_selected(optimize_window, OPTIMIZE_ICON_GREY_DOWNSAMPLE, params->downsample_grey_images);
	icons_set_selected(optimize_window, OPTIMIZE_ICON_MONO_DOWNSAMPLE, params->downsample_mono_images);

	icons_set_radio_group_selected(optimize_window, params->downsample_colour_type, 2,
			OPTIMIZE_ICON_COLOUR_SUBSAMPLE, OPTIMIZE_ICON_COLOUR_AVERAGE);
	icons_set_radio_group_selected(optimize_window, params->downsample_grey_type, 2,
			OPTIMIZE_ICON_GREY_SUBSAMPLE, OPTIMIZE_ICON_GREY_AVERAGE);
	icons_set_radio_group_selected(optimize_window, params->downsample_mono_type, 2,
			OPTIMIZE_ICON_MONO_SUBSAMPLE, OPTIMIZE_ICON_MONO_AVERAGE);

	icons_set_selected(optimize_window, OPTIMIZE_ICON_COLOUR_ENCODE, params->encode_colour_images);
	icons_set_selected(optimize_window, OPTIMIZE_ICON_GREY_ENCODE, params->encode_grey_images);
	icons_set_selected(optimize_window, OPTIMIZE_ICON_MONO_ENCODE, params->encode_mono_images);

	icons_set_radio_group_selected(optimize_window, params->encode_colour_type, 2,
			OPTIMIZE_ICON_COLOUR_DCT, OPTIMIZE_ICON_COLOUR_FLATE);
	icons_set_radio_group_selected(optimize_window, params->encode_grey_type, 2,
			OPTIMIZE_ICON_GREY_DCT, OPTIMIZE_ICON_GREY_FLATE);
	icons_set_radio_group_selected(optimize_window, params->encode_mono_type, 3,
			OPTIMIZE_ICON_MONO_CCITT, OPTIMIZE_ICON_MONO_FLATE, OPTIMIZE_ICON_MONO_RUNLENGTH);

	icons_set_radio_group_selected(optimize_window, params->auto_page_rotation, 3,
			OPTIMIZE_ICON_ROTATE_NONE, OPTIMIZE_ICON_ROTATE_ALL, OPTIMIZE_ICON_ROTATE_PAGE);

	icons_set_selected(optimize_window, OPTIMIZE_ICON_COMPRESS, params->compress_pages);

	sprintf(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_COLOUR_RESOLUTION),
			"%d", params->downsample_colour_resolution);

	sprintf(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_GREY_RESOLUTION),
			"%d", params->downsample_grey_resolution);

	sprintf(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_MONO_RESOLUTION),
			"%d", params->downsample_mono_resolution);

	sprintf(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_COLOUR_THRESHOLD),
			"%.1f", (double) params->downsample_colour_threshold / 10.0);

	sprintf(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_GREY_THRESHOLD),
			"%.1f", (double) params->downsample_grey_threshold / 10.0);

	sprintf(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_MONO_THRESHOLD),
			"%.1f", (double) params->downsample_mono_threshold / 10.0);

	downsample_mono_depth = params->downsample_mono_depth;
	downsample_grey_depth = params->downsample_grey_depth;
	downsample_colour_depth = params->downsample_colour_depth;

	optimize_depth_text(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_MONO_DEPTH), downsample_mono_depth);
	optimize_depth_text(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_GREY_DEPTH), downsample_grey_depth);
	optimize_depth_text(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_COLOUR_DEPTH), downsample_colour_depth);

	optimize_shade_dialogue(NULL);

	windows_open_transient_centred_at_pointer(optimize_window, pointer);
}


/**
 * Store the settings from the currently open Optimization dialogue box in
 * an optimization parameter block.
 *
 * \param *params		The optimization parameter block to be used.
 */

void optimize_process_dialogue(optimize_params *params)
{
	params->standard_preset = -1;

	params->downsample_colour_images = icons_get_selected(optimize_window,
			OPTIMIZE_ICON_COLOUR_DOWNSAMPLE);
	params->downsample_grey_images = icons_get_selected(optimize_window,
			OPTIMIZE_ICON_GREY_DOWNSAMPLE);
	params->downsample_mono_images = icons_get_selected(optimize_window,
			OPTIMIZE_ICON_MONO_DOWNSAMPLE);

	params->downsample_colour_type = icons_get_radio_group_selected(optimize_window, 2,
			OPTIMIZE_ICON_COLOUR_SUBSAMPLE, OPTIMIZE_ICON_COLOUR_AVERAGE);
	params->downsample_grey_type = icons_get_radio_group_selected(optimize_window, 2,
			OPTIMIZE_ICON_GREY_SUBSAMPLE, OPTIMIZE_ICON_GREY_AVERAGE);
	params->downsample_mono_type = icons_get_radio_group_selected(optimize_window, 2,
			OPTIMIZE_ICON_MONO_SUBSAMPLE, OPTIMIZE_ICON_MONO_AVERAGE);

	params->encode_colour_images = icons_get_selected(optimize_window, OPTIMIZE_ICON_COLOUR_ENCODE);
	params->encode_grey_images = icons_get_selected(optimize_window, OPTIMIZE_ICON_GREY_ENCODE);
	params->encode_mono_images = icons_get_selected(optimize_window, OPTIMIZE_ICON_MONO_ENCODE);

	params->encode_colour_type = icons_get_radio_group_selected(optimize_window, 2,
			OPTIMIZE_ICON_COLOUR_DCT, OPTIMIZE_ICON_COLOUR_FLATE);
	params->encode_grey_type = icons_get_radio_group_selected(optimize_window, 2,
			OPTIMIZE_ICON_GREY_DCT, OPTIMIZE_ICON_GREY_FLATE);
	params->encode_mono_type = icons_get_radio_group_selected(optimize_window, 3,
			OPTIMIZE_ICON_MONO_CCITT, OPTIMIZE_ICON_MONO_FLATE, OPTIMIZE_ICON_MONO_RUNLENGTH);

	params->auto_page_rotation = icons_get_radio_group_selected(optimize_window, 3,
			OPTIMIZE_ICON_ROTATE_NONE, OPTIMIZE_ICON_ROTATE_ALL, OPTIMIZE_ICON_ROTATE_PAGE);

	params->compress_pages = icons_get_selected (optimize_window, OPTIMIZE_ICON_COMPRESS);

	string_ctrl_zero_terminate(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_COLOUR_RESOLUTION));
	string_ctrl_zero_terminate(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_GREY_RESOLUTION));
	string_ctrl_zero_terminate(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_MONO_RESOLUTION));

	params->downsample_colour_resolution =
			atoi(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_COLOUR_RESOLUTION));
	params->downsample_grey_resolution =
			atoi(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_GREY_RESOLUTION));
	params->downsample_mono_resolution =
			atoi(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_MONO_RESOLUTION));

	string_ctrl_zero_terminate(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_COLOUR_THRESHOLD));
	string_ctrl_zero_terminate(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_GREY_THRESHOLD));
	string_ctrl_zero_terminate(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_MONO_THRESHOLD));

	params->downsample_colour_threshold = (int) 10.0 *
			atof(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_COLOUR_THRESHOLD));
	params->downsample_grey_threshold = (int) 10.0 *
			atof(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_GREY_THRESHOLD));
	params->downsample_mono_threshold = (int) 10.0 *
			atof(icons_get_indirected_text_addr(optimize_window, OPTIMIZE_ICON_MONO_THRESHOLD));

	params->downsample_mono_depth = downsample_mono_depth;
	params->downsample_grey_depth = downsample_grey_depth;
	params->downsample_colour_depth = downsample_colour_depth;

	wimp_create_menu((wimp_menu *) -1, 0, 0);
}


/**
 * Update the shading in the optimization dialogue, based on the current icon
 * selections
 *
 * \param: *pointer		The wimp pointer event data for the click.
 * \return			TRUE to show the event was handled; else FALSE.
 */

static bool optimize_shade_dialogue(wimp_pointer *pointer)
{
	if (pointer == NULL || pointer->i == OPTIMIZE_ICON_COLOUR_DOWNSAMPLE)
		icons_set_group_shaded_when_off(optimize_window, OPTIMIZE_ICON_COLOUR_DOWNSAMPLE, 11,
				OPTIMIZE_ICON_COLOUR_SUBSAMPLE, OPTIMIZE_ICON_COLOUR_AVERAGE,
				OPTIMIZE_ICON_COLOUR_RESOLUTION, OPTIMIZE_ICON_COLOUR_RESOLUTION_UP,
				OPTIMIZE_ICON_COLOUR_RESOLUTION_DOWN, OPTIMIZE_ICON_COLOUR_THRESHOLD,
				OPTIMIZE_ICON_COLOUR_THRESHOLD_UP, OPTIMIZE_ICON_COLOUR_THRESHOLD_DOWN,
				OPTIMIZE_ICON_COLOUR_DEPTH, OPTIMIZE_ICON_COLOUR_DEPTH_UP,
				OPTIMIZE_ICON_COLOUR_DEPTH_DOWN);

	if (pointer == NULL || pointer->i == OPTIMIZE_ICON_GREY_DOWNSAMPLE)
		icons_set_group_shaded_when_off(optimize_window, OPTIMIZE_ICON_GREY_DOWNSAMPLE, 11,
				OPTIMIZE_ICON_GREY_SUBSAMPLE, OPTIMIZE_ICON_GREY_AVERAGE,
				OPTIMIZE_ICON_GREY_RESOLUTION, OPTIMIZE_ICON_GREY_RESOLUTION_UP,
				OPTIMIZE_ICON_GREY_RESOLUTION_DOWN, OPTIMIZE_ICON_GREY_THRESHOLD,
				OPTIMIZE_ICON_GREY_THRESHOLD_UP, OPTIMIZE_ICON_GREY_THRESHOLD_DOWN,
				OPTIMIZE_ICON_GREY_DEPTH, OPTIMIZE_ICON_GREY_DEPTH_UP,
				OPTIMIZE_ICON_GREY_DEPTH_DOWN);

	if (pointer == NULL || pointer->i == OPTIMIZE_ICON_MONO_DOWNSAMPLE)
		icons_set_group_shaded_when_off(optimize_window, OPTIMIZE_ICON_MONO_DOWNSAMPLE, 11,
				OPTIMIZE_ICON_MONO_SUBSAMPLE, OPTIMIZE_ICON_MONO_AVERAGE,
				OPTIMIZE_ICON_MONO_RESOLUTION, OPTIMIZE_ICON_MONO_RESOLUTION_UP,
				OPTIMIZE_ICON_MONO_RESOLUTION_DOWN, OPTIMIZE_ICON_MONO_THRESHOLD,
				OPTIMIZE_ICON_MONO_THRESHOLD_UP, OPTIMIZE_ICON_MONO_THRESHOLD_DOWN,
				OPTIMIZE_ICON_MONO_DEPTH, OPTIMIZE_ICON_MONO_DEPTH_UP,
				OPTIMIZE_ICON_MONO_DEPTH_DOWN);

	if (pointer == NULL || pointer->i == OPTIMIZE_ICON_COLOUR_ENCODE)
		icons_set_group_shaded_when_off(optimize_window, OPTIMIZE_ICON_COLOUR_ENCODE, 2,
				OPTIMIZE_ICON_COLOUR_DCT, OPTIMIZE_ICON_COLOUR_FLATE);

	if (pointer == NULL || pointer->i == OPTIMIZE_ICON_GREY_ENCODE)
		icons_set_group_shaded_when_off(optimize_window, OPTIMIZE_ICON_GREY_ENCODE, 2,
				OPTIMIZE_ICON_GREY_DCT, OPTIMIZE_ICON_GREY_FLATE);

	if (pointer == NULL || pointer->i == OPTIMIZE_ICON_MONO_ENCODE)
		icons_set_group_shaded_when_off(optimize_window, OPTIMIZE_ICON_MONO_ENCODE, 3,
				OPTIMIZE_ICON_MONO_CCITT, OPTIMIZE_ICON_MONO_FLATE, OPTIMIZE_ICON_MONO_RUNLENGTH);

	icons_replace_caret_in_window(optimize_window);

	return TRUE;
}


/**
 * Update the given text field icon with a status reflecting the settings
 * in the given optimization parameter block.
 *
 * \param window		The window containing the icon.
 * \param icon			The icon to update.
 * \param *params		The optimization parameter block to use.
 */

void optimize_fill_field (wimp_w window, wimp_i icon, optimize_params *params)
{
	char		token[20];

	if (params->standard_preset == -1)
		snprintf(token, sizeof(token), "Custom");
	else
		snprintf(token, sizeof(token), "Optimization%d", params->standard_preset);

	msgs_lookup(token, icons_get_indirected_text_addr(window, icon), 20);
	wimp_set_icon_state(window, icon, 0, 0);
}


/**
 * Build up a text string in the supplied buffer containing the GS
 * parameters that reflect the contents of the given optimization
 * parameter block.
 *
 * \param *buffer		Buffer to hold the result.
 * \param len			The size of the buffer.
 * \param *params		The optimization parameter block to translate.
 */

void optimize_build_params(char *buffer, size_t len, optimize_params *params)
{
	char		settings[1024], *pointers[10], *extras, *end;

	if (buffer == NULL || params == NULL)
		return;

	*buffer = '\0';
	extras = "";

	if (params->standard_preset != -1) {
		pmenu_list_entry(settings, "OptimizationList", params->standard_preset);

		switch (params->standard_preset) {
		case 2:
			extras = "-dUseCIEColor=true ";
			break;
		}

		snprintf(buffer, len, "-dPDFSETTINGS=%s %s", settings, extras);
	} else {
		end = settings;

		end = pmenu_list_entry(pointers[0] = end, "DownsampleList", params->downsample_colour_type);
		end = pmenu_list_entry(pointers[1] = end, "DownsampleList", params->downsample_grey_type);
		end = pmenu_list_entry(pointers[2] = end, "DownsampleList", params->downsample_mono_type);
		end = pmenu_list_entry(pointers[3] = end, "EncodeList1", params->encode_colour_type);
		end = pmenu_list_entry(pointers[4] = end, "EncodeList1", params->encode_grey_type);
		end = pmenu_list_entry(pointers[5] = end, "EncodeList2", params->encode_mono_type);
		end = pmenu_list_entry(pointers[6] = end, "AutoPageRotateList", params->auto_page_rotation);

	snprintf(buffer, len, "-dDownsampleColorImages=%s -dDownsampleGrayImages=%s -dDownsampleMonoImages=%s "
			"-dColorImageDownsampleType=%s -dGrayImageDownsampleType=%s -dMonoImageDownsampleType=%s "
			"-dColorImageResolution=%d -dGrayImageResolution=%d -dMonoImageResolution=%d "
			"-dColorImageDownsampleThreshold=%.1f -dGrayImageDownsampleThreshold=%.1f "
			"-dMonoImageDownsampleThreshold=%.1f "
			"-dColorImageDepth=%d -dGrayImageDepth=%d -dMonoImageDepth=%d "
			"-dEncodeColorImages=%s -dEncodeGrayImages=%s -dEncodeMonoImages=%s "
			"-dAutoFilterColorImages=false -dAutofilterGrayImages=false -dAutoFilterMonoImages=false "
			"-dColorImageFilter=%s -dGreyImageFilter=%s -dMonoImageFilter=%s "
			"-dAutoRotatePages=%s -dCompressPages=%s ",
			optimize_true_false(params->downsample_colour_images), optimize_true_false(params->downsample_grey_images),
			optimize_true_false(params->downsample_mono_images), pointers[0], pointers[1], pointers[2],
			params->downsample_colour_resolution, params->downsample_grey_resolution,
			params->downsample_mono_resolution, (double) params->downsample_colour_threshold / 10.0,
			(double) params->downsample_grey_threshold / 10.0,  (double) params->downsample_mono_threshold / 10.0,
			params->downsample_colour_depth, params->downsample_grey_depth, params->downsample_mono_depth,
			optimize_true_false(params->encode_colour_images), optimize_true_false(params->encode_grey_images),
			optimize_true_false(params->encode_mono_images), pointers[3], pointers[4], pointers[5], pointers[6],
			optimize_true_false(params->compress_pages));
	}
}


/**
 * Return "true" or "false" depending on the logical value of the
 * parameter.
 *
 * \param value		The logical value to test.
 * \return		Pointer to "true" or "false".
 */

static char *optimize_true_false(osbool value)
{
	if (value)
		return "true";
	else
		return "false";
}


/**
 * Write a textual version of the given depth value into thje supplied buffer.
 *
 * \TODO -- This needs to take the buffer size as a parameter.
 *
 * \param *buffer		The buffer to take the textual value.
 * \param value			The value to convert.
 * \return			Pointer to the start of the buffer.
 */

static char *optimize_depth_text(char *buffer, int value)
{
	if (value > 0)
		snprintf(buffer, sizeof(buffer), "%d", value);
	else
		msgs_lookup("DepthOff", buffer, sizeof(buffer));

	return buffer;
}


/**
 * Update a resolution icon in the specified direction.
 *
 * \param i		The icon to update.
 * \param dir		The direction to move.
 */

static void optimize_update_resolution_icon(wimp_i i, int dir)
{
	int			value = -1;

	value = atoi(icons_get_indirected_text_addr(optimize_window, i));

	if (dir > 0 && value < 999)
		value += 1;
	else if (dir < 0 && value > 1)
		value -= 1;

	sprintf(icons_get_indirected_text_addr(optimize_window, i), "%d", value);
	wimp_set_icon_state(optimize_window, i, 0, 0);
}


/**
 * Update a threshold icon in the specified direction.
 *
 * \param i		The icon to update.
 * \param dir		The direction to move.
 */

static void optimize_update_threshold_icon(wimp_i i, int dir)
{
	int			value = -1;

	value = (int) 10.0 * atof(icons_get_indirected_text_addr(optimize_window, i));

	if (dir > 0 && value < 999)
		value += 1;
	else if (dir < 0 && value > 1)
		value -= 1;

	sprintf(icons_get_indirected_text_addr(optimize_window, i), "%.1f", (double) value / 10.0);
	wimp_set_icon_state(optimize_window, i, 0, 0);
}


/**
 * Update a depth icon in the specified direction.
 *
 * \param i		The icon to update.
 * \param dir		The direction to move.
 */

static void optimize_update_depth_icon(wimp_i i, int dir)
{
	int			value = -1;

	switch (i) {
	case OPTIMIZE_ICON_MONO_DEPTH:
		value = downsample_mono_depth;
		break;

	case OPTIMIZE_ICON_GREY_DEPTH:
		value = downsample_grey_depth;
		break;

	case OPTIMIZE_ICON_COLOUR_DEPTH:
		value = downsample_colour_depth;
		break;
	}

	if (dir > 0) {
		if (value >=1 && value <= 4)
			value *= 2;
		else if (value == -1)
			value = 1;
	} else if (dir < 0) {
		if (value >=2 && value <= 8)
			value /= 2;
		else if (value == 1)
			value = -1;
	}

	if (value != -1 && value != 1 && value != 2 && value != 4 && value != 8)
		value = -1;

	switch (i) {
	case OPTIMIZE_ICON_MONO_DEPTH:
		downsample_mono_depth = value;
		break;

	case OPTIMIZE_ICON_GREY_DEPTH:
		downsample_grey_depth = value;
		break;

	case OPTIMIZE_ICON_COLOUR_DEPTH:
		downsample_colour_depth = value;
		break;
	}

	optimize_depth_text(icons_get_indirected_text_addr(optimize_window, i), value);
	wimp_set_icon_state(optimize_window, i, 0, 0);
}

