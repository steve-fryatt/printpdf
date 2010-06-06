/* PrintPDF - optimize.h
 *
 * (c) Stephen Fryatt, 2007
 */

#ifndef _PRINTPDF_OPTIMIZE
#define _PRINTPDF_OPTIMIZE

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

/* ==================================================================================================================
 * Data structures
 */

typedef struct optimize_params
{
  int  standard_preset;

  int  downsample_mono_images;
  int  downsample_mono_type;
  int  downsample_mono_resolution;
  int  downsample_mono_threshold;
  int  downsample_mono_depth;

  int  downsample_grey_images;
  int  downsample_grey_type;
  int  downsample_grey_resolution;
  int  downsample_grey_threshold;
  int  downsample_grey_depth;

  int  downsample_colour_images;
  int  downsample_colour_type;
  int  downsample_colour_resolution;
  int  downsample_colour_threshold;
  int  downsample_colour_depth;

  int  encode_mono_images;
  int  encode_mono_type;

  int  encode_grey_images;
  int  encode_grey_type;

  int  encode_colour_images;
  int  encode_colour_type;

  int  auto_page_rotation;

  int  compress_pages;
}
optimize_params;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Handle the optimization window and menu. */

void initialise_optimization_settings (optimize_params *params);
void open_optimize_menu (optimize_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon, int ident);
void process_optimize_menu (optimize_params *params, wimp_selection *selection);
void open_optimize_dialogue (optimize_params *params, wimp_pointer *pointer);
void process_optimize_dialogue (optimize_params *params);
void shade_optimize_dialogue (void);
void fill_optimization_field (wimp_w window, wimp_i icon, optimize_params *params);
void build_optimization_params (char *buffer, optimize_params *params);
char *true_false (int value);
char *depth_text (char *buffer, int value);
void update_resolution_icon (wimp_i i, int dir);
void update_threshold_icon (wimp_i i, int dir);
void update_depth_icon (wimp_i i, int dir);

#endif
