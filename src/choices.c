/* PrintPDF - choices.c
 * (c) Stephen Fryatt, 2007
 */

/* ANSI C Header files. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Acorn C Header files. */


/* OSLib Header files. */

#include "oslib/wimp.h"

/* SF-Lib Header files. */

#include "sflib/config.h"
#include "sflib/icons.h"
#include "sflib/windows.h"
#include "sflib/debug.h"
#include "sflib/string.h"

/* Application header files. */

#include "choices.h"

#include "encrypt.h"
#include "iconbar.h"
#include "optimize.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "version.h"
#include "windows.h"

/* ==================================================================================================================
 * Global variables
 */

static encrypt_params  encryption;
static optimize_params optimization;
static version_params  version;
static pdfmark_params  pdfmark;

/* ==================================================================================================================
 * Open and close the window
 */

void open_choices_window (wimp_pointer *pointer)
{
  extern global_windows windows;

  set_choices_window ();

  open_window_centred_at_pointer (windows.choices, pointer);

  put_caret_at_end (windows.choices, CHOICE_ICON_DEFFILE);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void close_choices_window (void)
{
  extern global_windows windows;

  wimp_close_window (windows.choices);
}

/* ==================================================================================================================
 * Set choices window contents
 */

/* Set the contents of the Choices window to reflect the current settings. */

void set_choices_window (void)
{
  extern global_windows windows;

  /* Set the main window up. */

  sprintf (indirected_icon_text (windows.choices, CHOICE_ICON_DEFFILE), "%s", config_str_read ("FileName"));

  initialise_encryption_settings (&encryption);
  initialise_optimization_settings (&optimization);
  initialise_version_settings (&version);
  initialise_pdfmark_settings (&pdfmark);

  set_icon_selected (windows.choices, CHOICE_ICON_RESETEVERY, config_opt_read ("ResetParams"));
  set_icon_selected (windows.choices, CHOICE_ICON_IBAR, config_opt_read ("IconBarIcon"));
  set_icon_selected (windows.choices, CHOICE_ICON_POPUP, config_opt_read ("PopUpAfter"));
  set_icon_selected (windows.choices, CHOICE_ICON_PREPROCESS, config_opt_read ("PreProcess"));

  sprintf (indirected_icon_text (windows.choices, CHOICE_ICON_MEMORY), "%d", config_int_read ("TaskMemory"));

  fill_version_field (windows.choices, CHOICE_ICON_VERSION, &version);
  fill_optimization_field (windows.choices, CHOICE_ICON_OPTIMIZE, &optimization);
  fill_encryption_field (windows.choices, CHOICE_ICON_ENCRYPT, &encryption);
  fill_pdfmark_field (windows.choices, CHOICE_ICON_INFO, &pdfmark);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Read the contents of the Choices window into the settings. */

void read_choices_window (void)
{
  extern global_windows windows;

  /* Read the main window. */

  config_str_set ("FileName", indirected_icon_text (windows.choices, CHOICE_ICON_DEFFILE));

  config_int_set ("PDFVersion", version.standard_version);

  config_int_set ("Optimization", optimization.standard_preset);

  config_opt_set ("ResetParams", read_icon_selected (windows.choices, CHOICE_ICON_RESETEVERY));
  config_opt_set ("IconBarIcon", read_icon_selected (windows.choices, CHOICE_ICON_IBAR));
  config_opt_set ("PopUpAfter", read_icon_selected (windows.choices, CHOICE_ICON_POPUP));
  config_opt_set ("PreProcess", read_icon_selected (windows.choices, CHOICE_ICON_PREPROCESS));

  config_int_set ("TaskMemory", atoi (indirected_icon_text (windows.choices, CHOICE_ICON_MEMORY)));

  config_str_set ("OwnerPasswd", encryption.owner_password);
  config_str_set ("UserPasswd", encryption.access_password);

  config_opt_set ("AllowPrint", encryption.allow_print);
  config_opt_set ("AllowFullPrint", encryption.allow_full_print);
  config_opt_set ("AllowExtraction", encryption.allow_extraction);
  config_opt_set ("AllowFullExtraction", encryption.allow_full_extraction);
  config_opt_set ("AllowForms", encryption.allow_forms);
  config_opt_set ("AllowAnnotation", encryption.allow_annotation);
  config_opt_set ("AllowModifications", encryption.allow_modifications);
  config_opt_set ("AllowAssembly", encryption.allow_assembly);

  config_str_set ("PDFMarkTitle", pdfmark.title);
  config_str_set ("PDFMarkAuthor", pdfmark.author);
  config_str_set ("PDFMarkSubject", pdfmark.subject);
  config_str_set ("PDFMarkKeywords", pdfmark.keywords);

  config_int_set ("Optimization", optimization.standard_preset);

  config_opt_set ("DownsampleMono", optimization.downsample_mono_images);
  config_int_set ("DownsampleMonoType", optimization.downsample_mono_type);
  config_int_set ("DownsampleMonoResolution", optimization.downsample_mono_resolution);
  config_int_set ("DownsampleMonoThreshold", optimization.downsample_mono_threshold);
  config_int_set ("DownsampleMonoDepth", optimization.downsample_mono_depth);

  config_opt_set ("DownsampleGrey", optimization.downsample_grey_images);
  config_int_set ("DownsampleGreyType", optimization.downsample_grey_type);
  config_int_set ("DownsampleGreyResolution", optimization.downsample_grey_resolution);
  config_int_set ("DownsampleGreyThreshold", optimization.downsample_grey_threshold);
  config_int_set ("DownsampleGreyDepth", optimization.downsample_grey_depth);

  config_opt_set ("DownsampleColour", optimization.downsample_colour_images);
  config_int_set ("DownsampleColourType", optimization.downsample_colour_type);
  config_int_set ("DownsampleColourResolution", optimization.downsample_colour_resolution);
  config_int_set ("DownsampleColourThreshold", optimization.downsample_colour_threshold);
  config_int_set ("DownsampleColourDepth", optimization.downsample_colour_depth);

  config_opt_set ("EncodeMono", optimization.encode_mono_images);
  config_int_set ("EncodeMonoType", optimization.encode_mono_type);

  config_opt_set ("EncodeGrey", optimization.encode_grey_images);
  config_int_set ("EncodeGreyType", optimization.encode_grey_type);

  config_opt_set ("EncodeColour", optimization.encode_colour_images);
  config_int_set ("EncodeColourType", optimization.encode_colour_type);

  config_int_set ("AutoPageRotation", optimization.auto_page_rotation);

  config_opt_set ("CompressPages", optimization.compress_pages);

  /* Make any immediate changes that depend on the choices. */

  set_iconbar_icon (config_opt_read ("IconBarIcon"));
}

/* ================================================================================================================== */

void redraw_choices_window (void)
{
  extern global_windows windows;

  /* Redraw the contents of the Choices window, as required, and re-fresh the caret if necessary.
   */

   wimp_set_icon_state (windows.choices, CHOICE_ICON_VERSION, 0, 0);
   wimp_set_icon_state (windows.choices, CHOICE_ICON_OPTIMIZE, 0, 0);
   wimp_set_icon_state (windows.choices, CHOICE_ICON_MEMORY, 0, 0);

   replace_caret_in_window (windows.choices);
}

/* ================================================================================================================== */

void handle_choices_icon_drop (wimp_full_message_data_xfer *datasave)
{
  char *extension, *leaf, path[256];

  extern global_windows windows;


  if (datasave != NULL && datasave->w == windows.choices && datasave->i == CHOICE_ICON_DEFFILE)
  {
    strcpy (path, datasave->file_name);

    extension = find_extension (path);
    leaf = lose_extension (path);
    find_pathname (path);

    sprintf (indirected_icon_text (windows.choices, CHOICE_ICON_DEFFILE), "%s.%s/pdf", path, leaf);

    replace_caret_in_window (datasave->w);
    wimp_set_icon_state (datasave->w, datasave->i, 0, 0);
  }
}

/* ==================================================================================================================
 * Handle pop-up menus from the dialogue.
 */

void open_choices_version_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon)
{
  open_version_menu (&version, pointer, window, icon, PARAM_MENU_CHOICES_VERSION);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_choices_version_menu (wimp_selection *selection)
{
  extern global_windows windows;

  process_version_menu (&version, selection);

  fill_version_field (windows.choices, CHOICE_ICON_VERSION, &version);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_choices_optimize_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon)
{
  open_optimize_menu (&optimization, pointer, window, icon, PARAM_MENU_CHOICES_OPTIMIZE);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_choices_optimize_menu (wimp_selection *selection)
{
  extern global_windows windows;

  process_optimize_menu (&optimization, selection);

  fill_optimization_field (windows.choices, CHOICE_ICON_OPTIMIZE, &optimization);
}

/* ==================================================================================================================
 * Handle Encryption dialogue.
 */

void open_choices_encrypt_dialogue (wimp_pointer *pointer)
{
  open_encrypt_dialogue (&encryption, version.standard_version >= 2, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_choices_encrypt_dialogue (void)
{
  extern global_windows windows;

  process_encrypt_dialogue (&encryption);

  fill_encryption_field (windows.choices, CHOICE_ICON_ENCRYPT, &encryption);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_choices_pdfmark_dialogue (wimp_pointer *pointer)
{
  open_pdfmark_dialogue (&pdfmark, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_choices_pdfmark_dialogue (void)
{
  extern global_windows windows;

  process_pdfmark_dialogue (&pdfmark);

  fill_pdfmark_field (windows.choices, CHOICE_ICON_INFO, &pdfmark);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_choices_optimize_dialogue (void)
{
  extern global_windows windows;

  process_optimize_dialogue (&optimization);

  fill_optimization_field (windows.choices, CHOICE_ICON_OPTIMIZE, &optimization);
}
