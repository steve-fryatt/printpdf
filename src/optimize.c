/* PrintPDF - optimize.c
 *
 * (C) Stephen Fryatt, 2005
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

/* Application header files */

#include "optimize.h"

#include "menus.h"
#include "pmenu.h"
#include "windows.h"

/* ==================================================================================================================
 * Global variables.
 */

static int downsample_mono_depth;
static int downsample_grey_depth;
static int downsample_colour_depth;

/* Function Prototypes. */

int optimize_menu_tick (optimize_params *params);

/* ==================================================================================================================
 *
 */

void initialise_optimization_settings (optimize_params *params)
{
  params->standard_preset = config_int_read ("Optimization");

  params->downsample_mono_images = config_opt_read ("DownsampleMono");
  params->downsample_mono_type = config_int_read ("DownsampleMonoType");
  params->downsample_mono_resolution = config_int_read ("DownsampleMonoResolution");
  params->downsample_mono_threshold = config_int_read ("DownsampleMonoThreshold");
  params->downsample_mono_depth = config_int_read ("DownsampleMonoDepth");

  params->downsample_grey_images = config_opt_read ("DownsampleGrey");
  params->downsample_grey_type = config_int_read ("DownsampleGreyType");
  params->downsample_grey_resolution = config_int_read ("DownsampleGreyResolution");
  params->downsample_grey_threshold = config_int_read ("DownsampleGreyThreshold");
  params->downsample_grey_depth = config_int_read ("DownsampleGreyDepth");

  params->downsample_colour_images = config_opt_read ("DownsampleColour");
  params->downsample_colour_type = config_int_read ("DownsampleColourType");
  params->downsample_colour_resolution = config_int_read ("DownsampleColourResolution");
  params->downsample_colour_threshold = config_int_read ("DownsampleColourThreshold");
  params->downsample_colour_depth = config_int_read ("DownsampleColourDepth");

  params->encode_mono_images = config_opt_read ("EncodeMono");
  params->encode_mono_type = config_int_read ("EncodeMonoType");

  params->encode_grey_images = config_opt_read ("EncodeGrey");
  params->encode_grey_type = config_int_read ("EncodeGreyType");

  params->encode_colour_images = config_opt_read ("EncodeColour");
  params->encode_colour_type = config_int_read ("EncodeColourType");

  params->auto_page_rotation = config_int_read ("AutoPageRotation");

  params->compress_pages = config_opt_read ("CompressPages");
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_optimize_menu (optimize_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon, int ident)
{
  if (build_param_menu ("OptimizationMenu", ident, optimize_menu_tick (params)) != NULL)
  {
    open_param_menu (pointer, window, icon);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_optimize_menu (optimize_params *params, wimp_selection *selection)
{
  wimp_pointer pointer;

  if (selection->items[0] == param_menu_len ("OptimizationMenu") - 1)
  {
    wimp_get_pointer_info (&pointer);
    open_optimize_dialogue (params, &pointer);
  }
  else
  {
    params->standard_preset = selection->items[0];
  }

  build_param_menu ("OptimizationMenu", param_menu_ident (), optimize_menu_tick (params));
}

/* ------------------------------------------------------------------------------------------------------------------ */

int optimize_menu_tick (optimize_params *params)
{
  int          item;

  if (params->standard_preset == -1)
  {
    item = param_menu_len ("OptimizationMenu") - 1;
  }
  else
  {
    item = params->standard_preset;
  }
  return (item);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_optimize_dialogue (optimize_params *params, wimp_pointer *pointer)
{
  extern global_windows windows;

  /* Set the dialogue icons. */

  set_icon_selected (windows.optimization, OPTIMIZE_ICON_COLOUR_DOWNSAMPLE, params->downsample_colour_images);
  set_icon_selected (windows.optimization, OPTIMIZE_ICON_GREY_DOWNSAMPLE, params->downsample_grey_images);
  set_icon_selected (windows.optimization, OPTIMIZE_ICON_MONO_DOWNSAMPLE, params->downsample_mono_images);

  set_radio_icon_group_selected (windows.optimization, params->downsample_colour_type, 2,
                                 OPTIMIZE_ICON_COLOUR_SUBSAMPLE, OPTIMIZE_ICON_COLOUR_AVERAGE);
  set_radio_icon_group_selected (windows.optimization, params->downsample_grey_type, 2,
                                 OPTIMIZE_ICON_GREY_SUBSAMPLE, OPTIMIZE_ICON_GREY_AVERAGE);
  set_radio_icon_group_selected (windows.optimization, params->downsample_mono_type, 2,
                                 OPTIMIZE_ICON_MONO_SUBSAMPLE, OPTIMIZE_ICON_MONO_AVERAGE);

  set_icon_selected (windows.optimization, OPTIMIZE_ICON_COLOUR_ENCODE, params->encode_colour_images);
  set_icon_selected (windows.optimization, OPTIMIZE_ICON_GREY_ENCODE, params->encode_grey_images);
  set_icon_selected (windows.optimization, OPTIMIZE_ICON_MONO_ENCODE, params->encode_mono_images);

  set_radio_icon_group_selected (windows.optimization, params->encode_colour_type, 2,
                                 OPTIMIZE_ICON_COLOUR_DCT, OPTIMIZE_ICON_COLOUR_FLATE);
  set_radio_icon_group_selected (windows.optimization, params->encode_grey_type, 2,
                                 OPTIMIZE_ICON_GREY_DCT, OPTIMIZE_ICON_GREY_FLATE);
  set_radio_icon_group_selected (windows.optimization, params->encode_mono_type, 3,
                                 OPTIMIZE_ICON_MONO_CCITT, OPTIMIZE_ICON_MONO_FLATE, OPTIMIZE_ICON_MONO_RUNLENGTH);

  set_radio_icon_group_selected (windows.optimization, params->auto_page_rotation, 3,
                                 OPTIMIZE_ICON_ROTATE_NONE, OPTIMIZE_ICON_ROTATE_ALL, OPTIMIZE_ICON_ROTATE_PAGE);

  set_icon_selected (windows.optimization, OPTIMIZE_ICON_COMPRESS, params->compress_pages);

  sprintf (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_COLOUR_RESOLUTION),
           "%d", params->downsample_colour_resolution);

  sprintf (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_GREY_RESOLUTION),
           "%d", params->downsample_grey_resolution);

  sprintf (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_MONO_RESOLUTION),
           "%d", params->downsample_mono_resolution);

  sprintf (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_COLOUR_THRESHOLD),
           "%.1f", (double) params->downsample_colour_threshold / 10.0);

  sprintf (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_GREY_THRESHOLD),
           "%.1f", (double) params->downsample_grey_threshold / 10.0);

  sprintf (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_MONO_THRESHOLD),
           "%.1f", (double) params->downsample_mono_threshold / 10.0);

  downsample_mono_depth = params->downsample_mono_depth;
  downsample_grey_depth = params->downsample_grey_depth;
  downsample_colour_depth = params->downsample_colour_depth;

  depth_text (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_MONO_DEPTH), downsample_mono_depth);
  depth_text (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_GREY_DEPTH), downsample_grey_depth);
  depth_text (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_COLOUR_DEPTH), downsample_colour_depth);

  shade_optimize_dialogue ();

  open_transient_window_centred_at_pointer (windows.optimization, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_optimize_dialogue (optimize_params *params)
{
  extern global_windows windows;

  params->standard_preset = -1;

  params->downsample_colour_images = read_icon_selected (windows.optimization, OPTIMIZE_ICON_COLOUR_DOWNSAMPLE);
  params->downsample_grey_images = read_icon_selected (windows.optimization, OPTIMIZE_ICON_GREY_DOWNSAMPLE);
  params->downsample_mono_images = read_icon_selected (windows.optimization, OPTIMIZE_ICON_MONO_DOWNSAMPLE);

  params->downsample_colour_type = read_radio_icon_group_selected (windows.optimization, 2,
                                   OPTIMIZE_ICON_COLOUR_SUBSAMPLE, OPTIMIZE_ICON_COLOUR_AVERAGE);
  params->downsample_grey_type = read_radio_icon_group_selected (windows.optimization, 2,
                                 OPTIMIZE_ICON_GREY_SUBSAMPLE, OPTIMIZE_ICON_GREY_AVERAGE);
  params->downsample_mono_type = read_radio_icon_group_selected (windows.optimization, 2,
                                 OPTIMIZE_ICON_MONO_SUBSAMPLE, OPTIMIZE_ICON_MONO_AVERAGE);

  params->encode_colour_images = read_icon_selected (windows.optimization, OPTIMIZE_ICON_COLOUR_ENCODE);
  params->encode_grey_images = read_icon_selected (windows.optimization, OPTIMIZE_ICON_GREY_ENCODE);
  params->encode_mono_images = read_icon_selected (windows.optimization, OPTIMIZE_ICON_MONO_ENCODE);

  params->encode_colour_type = read_radio_icon_group_selected (windows.optimization, 2,
                               OPTIMIZE_ICON_COLOUR_DCT, OPTIMIZE_ICON_COLOUR_FLATE);
  params->encode_grey_type = read_radio_icon_group_selected (windows.optimization, 2,
                             OPTIMIZE_ICON_GREY_DCT, OPTIMIZE_ICON_GREY_FLATE);
  params->encode_mono_type = read_radio_icon_group_selected (windows.optimization, 3,
                             OPTIMIZE_ICON_MONO_CCITT, OPTIMIZE_ICON_MONO_FLATE, OPTIMIZE_ICON_MONO_RUNLENGTH);

  params->auto_page_rotation = read_radio_icon_group_selected (windows.optimization, 3,
                               OPTIMIZE_ICON_ROTATE_NONE, OPTIMIZE_ICON_ROTATE_ALL, OPTIMIZE_ICON_ROTATE_PAGE);

  params->compress_pages = read_icon_selected (windows.optimization, OPTIMIZE_ICON_COMPRESS);

  terminate_ctrl_str (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_COLOUR_RESOLUTION));
  terminate_ctrl_str (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_GREY_RESOLUTION));
  terminate_ctrl_str (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_MONO_RESOLUTION));

  params->downsample_colour_resolution = atoi (indirected_icon_text (windows.optimization,
                                                                     OPTIMIZE_ICON_COLOUR_RESOLUTION));
  params->downsample_grey_resolution = atoi (indirected_icon_text (windows.optimization,
                                                                   OPTIMIZE_ICON_GREY_RESOLUTION));
  params->downsample_mono_resolution = atoi (indirected_icon_text (windows.optimization,
                                                                   OPTIMIZE_ICON_MONO_RESOLUTION));

  terminate_ctrl_str (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_COLOUR_THRESHOLD));
  terminate_ctrl_str (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_GREY_THRESHOLD));
  terminate_ctrl_str (indirected_icon_text (windows.optimization, OPTIMIZE_ICON_MONO_THRESHOLD));

  params->downsample_colour_threshold = (int) 10.0 * atof (indirected_icon_text (windows.optimization,
                                                                                 OPTIMIZE_ICON_COLOUR_THRESHOLD));
  params->downsample_grey_threshold = (int) 10.0 * atof (indirected_icon_text (windows.optimization,
                                                                               OPTIMIZE_ICON_GREY_THRESHOLD));
  params->downsample_mono_threshold = (int) 10.0 * atof (indirected_icon_text (windows.optimization,
                                                                               OPTIMIZE_ICON_MONO_THRESHOLD));

  params->downsample_mono_depth = downsample_mono_depth;
  params->downsample_grey_depth = downsample_grey_depth;
  params->downsample_colour_depth = downsample_colour_depth;

  wimp_create_menu ((wimp_menu *) -1, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void shade_optimize_dialogue (void)
{
  extern global_windows windows;

  set_icons_shaded_when_radio_off (windows.optimization, OPTIMIZE_ICON_COLOUR_DOWNSAMPLE, 11,
                                   OPTIMIZE_ICON_COLOUR_SUBSAMPLE, OPTIMIZE_ICON_COLOUR_AVERAGE,
                                   OPTIMIZE_ICON_COLOUR_RESOLUTION, OPTIMIZE_ICON_COLOUR_RESOLUTION_UP,
                                   OPTIMIZE_ICON_COLOUR_RESOLUTION_DOWN, OPTIMIZE_ICON_COLOUR_THRESHOLD,
                                   OPTIMIZE_ICON_COLOUR_THRESHOLD_UP, OPTIMIZE_ICON_COLOUR_THRESHOLD_DOWN,
                                   OPTIMIZE_ICON_COLOUR_DEPTH, OPTIMIZE_ICON_COLOUR_DEPTH_UP,
                                   OPTIMIZE_ICON_COLOUR_DEPTH_DOWN);

  set_icons_shaded_when_radio_off (windows.optimization, OPTIMIZE_ICON_GREY_DOWNSAMPLE, 11,
                                   OPTIMIZE_ICON_GREY_SUBSAMPLE, OPTIMIZE_ICON_GREY_AVERAGE,
                                   OPTIMIZE_ICON_GREY_RESOLUTION, OPTIMIZE_ICON_GREY_RESOLUTION_UP,
                                   OPTIMIZE_ICON_GREY_RESOLUTION_DOWN, OPTIMIZE_ICON_GREY_THRESHOLD,
                                   OPTIMIZE_ICON_GREY_THRESHOLD_UP, OPTIMIZE_ICON_GREY_THRESHOLD_DOWN,
                                   OPTIMIZE_ICON_GREY_DEPTH, OPTIMIZE_ICON_GREY_DEPTH_UP,
                                   OPTIMIZE_ICON_GREY_DEPTH_DOWN);

  set_icons_shaded_when_radio_off (windows.optimization, OPTIMIZE_ICON_MONO_DOWNSAMPLE, 11,
                                   OPTIMIZE_ICON_MONO_SUBSAMPLE, OPTIMIZE_ICON_MONO_AVERAGE,
                                   OPTIMIZE_ICON_MONO_RESOLUTION, OPTIMIZE_ICON_MONO_RESOLUTION_UP,
                                   OPTIMIZE_ICON_MONO_RESOLUTION_DOWN, OPTIMIZE_ICON_MONO_THRESHOLD,
                                   OPTIMIZE_ICON_MONO_THRESHOLD_UP, OPTIMIZE_ICON_MONO_THRESHOLD_DOWN,
                                   OPTIMIZE_ICON_MONO_DEPTH, OPTIMIZE_ICON_MONO_DEPTH_UP,
                                   OPTIMIZE_ICON_MONO_DEPTH_DOWN);

  set_icons_shaded_when_radio_off (windows.optimization, OPTIMIZE_ICON_COLOUR_ENCODE, 2,
                                   OPTIMIZE_ICON_COLOUR_DCT, OPTIMIZE_ICON_COLOUR_FLATE);

  set_icons_shaded_when_radio_off (windows.optimization, OPTIMIZE_ICON_GREY_ENCODE, 2,
                                   OPTIMIZE_ICON_GREY_DCT, OPTIMIZE_ICON_GREY_FLATE);

  set_icons_shaded_when_radio_off (windows.optimization, OPTIMIZE_ICON_MONO_ENCODE, 3,
                                   OPTIMIZE_ICON_MONO_CCITT, OPTIMIZE_ICON_MONO_FLATE, OPTIMIZE_ICON_MONO_RUNLENGTH);

  replace_caret_in_window (windows.optimization);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void fill_optimization_field (wimp_w window, wimp_i icon, optimize_params *params)
{
  if (params->standard_preset == -1)
  {
    msgs_lookup ("Custom", indirected_icon_text (window, icon), 20);
  }
  else
  {
    param_menu_entry (indirected_icon_text (window, icon), "OptimizationMenu", params->standard_preset + 1);
  }

  wimp_set_icon_state (window, icon, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void build_optimization_params (char *buffer, optimize_params *params)
{
  char settings[1024], *pointers[10], *extras, *end;

  *buffer = '\0';
  extras = "";

  if (params->standard_preset != -1)
  {
    param_menu_entry (settings, "OptimizationList", params->standard_preset);

    switch (params->standard_preset)
    {
      case 2:
        extras = "-dUseCIEColor=true ";
        break;
    }

    sprintf (buffer, "-dPDFSETTINGS=%s %s", settings, extras);
  }
  else
  {
    end = settings;

    end = param_menu_entry (pointers[0] = end, "DownsampleList", params->downsample_colour_type);
    end = param_menu_entry (pointers[1] = end, "DownsampleList", params->downsample_grey_type);
    end = param_menu_entry (pointers[2] = end, "DownsampleList", params->downsample_mono_type);
    end = param_menu_entry (pointers[3] = end, "EncodeList1", params->encode_colour_type);
    end = param_menu_entry (pointers[4] = end, "EncodeList1", params->encode_grey_type);
    end = param_menu_entry (pointers[5] = end, "EncodeList2", params->encode_mono_type);
    end = param_menu_entry (pointers[6] = end, "AutoPageRotateList", params->auto_page_rotation);

    sprintf (buffer, "-dDownsampleColorImages=%s -dDownsampleGrayImages=%s -dDownsampleMonoImages=%s "
             "-dColorImageDownsampleType=%s -dGrayImageDownsampleType=%s -dMonoImageDownsampleType=%s "
             "-dColorImageResolution=%d -dGrayImageResolution=%d -dMonoImageResolution=%d "
             "-dColorImageDownsampleThreshold=%.1f -dGrayImageDownsampleThreshold=%.1f "
             "-dMonoImageDownsampleThreshold=%.1f "
             "-dColorImageDepth=%d -dGrayImageDepth=%d -dMonoImageDepth=%d "
             "-dEncodeColorImages=%s -dEncodeGrayImages=%s -dEncodeMonoImages=%s "
             "-dAutoFilterColorImages=false -dAutofilterGrayImages=false -dAutoFilterMonoImages=false "
             "-dColorImageFilter=%s -dGreyImageFilter=%s -dMonoImageFilter=%s "
             "-dAutoRotatePages=%s -dCompressPages=%s ",
             true_false (params->downsample_colour_images), true_false (params->downsample_grey_images),
             true_false (params->downsample_mono_images), pointers[0], pointers[1], pointers[2],
             params->downsample_colour_resolution, params->downsample_grey_resolution,
             params->downsample_mono_resolution, (double) params->downsample_colour_threshold / 10.0,
             (double) params->downsample_grey_threshold / 10.0,  (double) params->downsample_mono_threshold / 10.0,
             params->downsample_colour_depth, params->downsample_grey_depth, params->downsample_mono_depth,
             true_false (params->encode_colour_images), true_false (params->encode_grey_images),
             true_false (params->encode_mono_images), pointers[3], pointers[4], pointers[5], pointers[6],
             true_false (params->compress_pages));
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *true_false (int value)
{
  if (value)
  {
    return ("true");
  }
  else
  {
    return ("false");
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *depth_text (char *buffer, int value)
{
  if (value > 0)
  {
    sprintf (buffer, "%d", value);
  }
  else
  {
    msgs_lookup ("DepthOff", buffer, sizeof (buffer));
  }

  return (buffer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void update_resolution_icon (wimp_i i, int dir)
{
  int value = -1;

  extern global_windows windows;


  value = atoi (indirected_icon_text (windows.optimization, i));

  if (dir > 0 && value < 999)
  {
    value += 1;
  }
  else if (dir < 0 && value > 1)
  {
    value -= 1;
  }

  sprintf (indirected_icon_text (windows.optimization, i), "%d", value);
  wimp_set_icon_state (windows.optimization, i, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void update_threshold_icon (wimp_i i, int dir)
{
  int value = -1;

  extern global_windows windows;


  value = (int) 10.0 * atof (indirected_icon_text (windows.optimization, i));

  if (dir > 0 && value < 999)
  {
    value += 1;
  }
  else if (dir < 0 && value > 1)
  {
    value -= 1;
  }

  sprintf (indirected_icon_text (windows.optimization, i), "%.1f", (double) value / 10.0);
  wimp_set_icon_state (windows.optimization, i, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void update_depth_icon (wimp_i i, int dir)
{
  int value = -1;

  extern global_windows windows;


  switch (i)
  {
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

  if (dir > 0)
  {
    if (value >=1 && value <= 4)
    {
      value *= 2;
    }
    else if (value == -1)
    {
      value = 1;
    }
  }
  else if (dir < 0)
  {
    if (value >=2 && value <= 8)
    {
      value /= 2;
    }
    else if (value == 1)
    {
      value = -1;
    }
  }

  if (value != -1 && value != 1 && value != 2 && value != 4 && value != 8)
  {
    value = -1;
  }

  switch (i)
  {
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

  depth_text (indirected_icon_text (windows.optimization, i), value);
  wimp_set_icon_state (windows.optimization, i, 0, 0);
}
