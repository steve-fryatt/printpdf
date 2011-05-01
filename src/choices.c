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

  sprintf (indirected_icon_text (windows.choices, CHOICE_ICON_DEFFILE), "%s", read_config_str ("FileName"));

  initialise_encryption_settings (&encryption);
  initialise_optimization_settings (&optimization);
  initialise_version_settings (&version);
  initialise_pdfmark_settings (&pdfmark);

  set_icon_selected (windows.choices, CHOICE_ICON_RESETEVERY, read_config_opt ("ResetParams"));
  set_icon_selected (windows.choices, CHOICE_ICON_IBAR, read_config_opt ("IconBarIcon"));
  set_icon_selected (windows.choices, CHOICE_ICON_POPUP, read_config_opt ("PopUpAfter"));
  set_icon_selected (windows.choices, CHOICE_ICON_PREPROCESS, read_config_opt ("PreProcess"));

  sprintf (indirected_icon_text (windows.choices, CHOICE_ICON_MEMORY), "%d", read_config_int ("TaskMemory"));

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

  set_config_str ("FileName", indirected_icon_text (windows.choices, CHOICE_ICON_DEFFILE));

  set_config_int ("PDFVersion", version.standard_version);

  set_config_int ("Optimization", optimization.standard_preset);

  set_config_opt ("ResetParams", read_icon_selected (windows.choices, CHOICE_ICON_RESETEVERY));
  set_config_opt ("IconBarIcon", read_icon_selected (windows.choices, CHOICE_ICON_IBAR));
  set_config_opt ("PopUpAfter", read_icon_selected (windows.choices, CHOICE_ICON_POPUP));
  set_config_opt ("PreProcess", read_icon_selected (windows.choices, CHOICE_ICON_PREPROCESS));

  set_config_int ("TaskMemory", atoi (indirected_icon_text (windows.choices, CHOICE_ICON_MEMORY)));

  set_config_str ("OwnerPasswd", encryption.owner_password);
  set_config_str ("UserPasswd", encryption.access_password);

  set_config_opt ("AllowPrint", encryption.allow_print);
  set_config_opt ("AllowFullPrint", encryption.allow_full_print);
  set_config_opt ("AllowExtraction", encryption.allow_extraction);
  set_config_opt ("AllowFullExtraction", encryption.allow_full_extraction);
  set_config_opt ("AllowForms", encryption.allow_forms);
  set_config_opt ("AllowAnnotation", encryption.allow_annotation);
  set_config_opt ("AllowModifications", encryption.allow_modifications);
  set_config_opt ("AllowAssembly", encryption.allow_assembly);

  set_config_str ("PDFMarkTitle", pdfmark.title);
  set_config_str ("PDFMarkAuthor", pdfmark.author);
  set_config_str ("PDFMarkSubject", pdfmark.subject);
  set_config_str ("PDFMarkKeywords", pdfmark.keywords);

  set_config_int ("Optimization", optimization.standard_preset);

  set_config_opt ("DownsampleMono", optimization.downsample_mono_images);
  set_config_int ("DownsampleMonoType", optimization.downsample_mono_type);
  set_config_int ("DownsampleMonoResolution", optimization.downsample_mono_resolution);
  set_config_int ("DownsampleMonoThreshold", optimization.downsample_mono_threshold);
  set_config_int ("DownsampleMonoDepth", optimization.downsample_mono_depth);

  set_config_opt ("DownsampleGrey", optimization.downsample_grey_images);
  set_config_int ("DownsampleGreyType", optimization.downsample_grey_type);
  set_config_int ("DownsampleGreyResolution", optimization.downsample_grey_resolution);
  set_config_int ("DownsampleGreyThreshold", optimization.downsample_grey_threshold);
  set_config_int ("DownsampleGreyDepth", optimization.downsample_grey_depth);

  set_config_opt ("DownsampleColour", optimization.downsample_colour_images);
  set_config_int ("DownsampleColourType", optimization.downsample_colour_type);
  set_config_int ("DownsampleColourResolution", optimization.downsample_colour_resolution);
  set_config_int ("DownsampleColourThreshold", optimization.downsample_colour_threshold);
  set_config_int ("DownsampleColourDepth", optimization.downsample_colour_depth);

  set_config_opt ("EncodeMono", optimization.encode_mono_images);
  set_config_int ("EncodeMonoType", optimization.encode_mono_type);

  set_config_opt ("EncodeGrey", optimization.encode_grey_images);
  set_config_int ("EncodeGreyType", optimization.encode_grey_type);

  set_config_opt ("EncodeColour", optimization.encode_colour_images);
  set_config_int ("EncodeColourType", optimization.encode_colour_type);

  set_config_int ("AutoPageRotation", optimization.auto_page_rotation);

  set_config_opt ("CompressPages", optimization.compress_pages);

  /* Make any immediate changes that depend on the choices. */

  set_iconbar_icon (read_config_opt ("IconBarIcon"));
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
