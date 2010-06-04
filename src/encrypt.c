/* PrintPDF - encrypt.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/windows.h"

/* Application header files */

#include "encrypt.h"

#include "pmenu.h"
#include "windows.h"

/* ==================================================================================================================
 * Global variables.
 */

/* ==================================================================================================================
 *
 */
void initialise_encryption_settings (encrypt_params *params)
{
  strcpy(params->owner_password, read_config_str ("OwnerPasswd"));
  strcpy(params->access_password, read_config_str ("UserPasswd"));

  params->allow_print = read_config_opt ("AllowPrint");
  params->allow_full_print = read_config_opt ("AllowFullPrint");
  params->allow_extraction = read_config_opt ("AllowExtraction");
  params->allow_full_extraction = read_config_opt ("AllowFullExtraction");
  params->allow_forms = read_config_opt ("AllowForms");
  params->allow_annotation = read_config_opt ("AllowAnnotation");
  params->allow_modifications = read_config_opt ("AllowModifications");
  params->allow_assembly = read_config_opt ("AllowAssembly");
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_encrypt_dialogue (encrypt_params *params, int extended_opts, wimp_pointer *pointer)
{
  wimp_w encrypt_win;

  extern global_windows windows;


  encrypt_win = (extended_opts) ? windows.security3 : windows.security2;

  strcpy (indirected_icon_text (encrypt_win, ENCRYPT_ICON_OWNER_PW), params->owner_password);
  strcpy (indirected_icon_text (encrypt_win, ENCRYPT_ICON_ACCESS_PW), params->access_password);

  set_icon_selected (encrypt_win, ENCRYPT_ICON_PRINT, params->allow_print);
  set_icon_selected (encrypt_win, ENCRYPT_ICON_EXTRACT, params->allow_extraction);
  set_icon_selected (encrypt_win, ENCRYPT_ICON_FORMS, params->allow_forms);
  set_icon_selected (encrypt_win, ENCRYPT_ICON_MODS, params->allow_modifications);

  if (extended_opts)
  {
    set_icon_selected (encrypt_win, ENCRYPT_ICON_FULL_PRINT, params->allow_full_print);
    set_icon_selected (encrypt_win, ENCRYPT_ICON_FULL_EXTRACT, params->allow_full_extraction);
    set_icon_selected (encrypt_win, ENCRYPT_ICON_ANNOTATE, params->allow_annotation);
    set_icon_selected (encrypt_win, ENCRYPT_ICON_ASSEMBLY, params->allow_assembly);
  }

  shade_encrypt_dialogue (encrypt_win);

  create_standard_menu ((wimp_menu *) encrypt_win, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_encrypt_dialogue (encrypt_params *params)
{
  int    extended_opts;
  wimp_w encrypt_win;

  extern global_windows windows;


  if (window_is_open (windows.security2))
  {
    encrypt_win = windows.security2;
    extended_opts = FALSE;
  }
  else
  {
    encrypt_win = windows.security3;
    extended_opts = TRUE;
  }

  strcpy (params->owner_password, indirected_icon_text (encrypt_win, ENCRYPT_ICON_OWNER_PW));
  strcpy (params->access_password, indirected_icon_text (encrypt_win, ENCRYPT_ICON_ACCESS_PW));

  params->allow_print = read_icon_selected (encrypt_win, ENCRYPT_ICON_PRINT);
  params->allow_extraction = read_icon_selected (encrypt_win, ENCRYPT_ICON_EXTRACT);
  params->allow_forms = read_icon_selected (encrypt_win, ENCRYPT_ICON_FORMS);
  params->allow_modifications = read_icon_selected (encrypt_win, ENCRYPT_ICON_MODS);

  if (extended_opts)
  {
    params->allow_full_print = read_icon_selected (encrypt_win, ENCRYPT_ICON_FULL_PRINT);
    params->allow_full_extraction = read_icon_selected (encrypt_win, ENCRYPT_ICON_FULL_EXTRACT);
    params->allow_annotation = read_icon_selected (encrypt_win, ENCRYPT_ICON_ANNOTATE);
    params->allow_assembly = read_icon_selected (encrypt_win, ENCRYPT_ICON_ASSEMBLY);
  }

  wimp_create_menu ((wimp_menu *) -1, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void shade_encrypt_dialogue (wimp_w window)
{
  static int shaded = FALSE;
  int        new_state, changed, max_icon, icon;
  wimp_w     encrypt_win;

  extern global_windows windows;


  changed = FALSE;

  if (window != 0)
  {
    encrypt_win = window;

    shaded = (strlen (indirected_icon_text (encrypt_win, ENCRYPT_ICON_OWNER_PW)) == 0);
    max_icon = (encrypt_win == windows.security3) ? ENCRYPT_ICON_ASSEMBLY : ENCRYPT_ICON_FORMS;

    changed = TRUE;
  }
  else
  {
    if (window_is_open (windows.security2))
    {
      encrypt_win = windows.security2;
      new_state = (strlen (indirected_icon_text (encrypt_win, ENCRYPT_ICON_OWNER_PW)) == 0);
      max_icon = ENCRYPT_ICON_SHADE_MAX2;

      if (new_state != shaded)
      {
        shaded = new_state;
        changed = TRUE;
      }
    }
    else if (window_is_open (windows.security3))
    {
      encrypt_win = windows.security3;
      new_state = (strlen (indirected_icon_text (encrypt_win, ENCRYPT_ICON_OWNER_PW)) == 0);
      max_icon = ENCRYPT_ICON_SHADE_MAX3;

      if (new_state != shaded)
      {
        shaded = new_state;
        changed = TRUE;
      }
    }
  }

  if (changed)
  {
    for (icon = ENCRYPT_ICON_SHADE_BASE; icon <= max_icon; icon++)
    {
      set_icon_shaded (encrypt_win, icon, shaded);
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void fill_encryption_field (wimp_w window, wimp_i icon, encrypt_params *params)
{
  char token[20];


  if (strlen (params->owner_password) == 0)
  {
    strcpy (token, "Encrypt0");
  }
  else
  {
    strcpy (token, "Encrypt1");
  }

  msgs_lookup (token, indirected_icon_text (window, icon), 20);
  wimp_set_icon_state (window, icon, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void build_encryption_params (char *buffer, encrypt_params *params, int extended_opts)
{
  char user[100];
  int  level, permissions;

  *buffer = '\0';

  if (strlen (params->owner_password) > 0)
  {
    *user = '\0';

    if (strlen (params->access_password) > 0)
    {
      sprintf (user, "-sUserPassword=%s ", params->access_password);
    }

    level = (extended_opts) ? 3 : 2;

    if (level == 2)
    {
       permissions = ACCESS_REV2_BASE;

       if (params->allow_print)
       {
         permissions |= ACCESS_REV2_PRINT;
       }

       if (params->allow_extraction)
       {
         permissions |= ACCESS_REV2_COPY;
       }

       if (params->allow_forms)
       {
         permissions |= ACCESS_REV2_ANNOTATE;
       }

       if (params->allow_modifications)
       {
         permissions |= ACCESS_REV2_MODIFY;
       }
    }
    else if (level == 3)
    {
       permissions = ACCESS_REV3_BASE;

       if (params->allow_print)
       {
         permissions |= ACCESS_REV3_PRINT;
       }

       if (params->allow_extraction)
       {
         permissions |= ACCESS_REV3_COPYACCESS;
       }

       if (params->allow_forms)
       {
         permissions |= ACCESS_REV3_FORMS;
       }

       if (params->allow_modifications)
       {
         permissions |= ACCESS_REV3_MODIFY;
       }

       if (params->allow_full_print)
       {
         permissions |= ACCESS_REV3_PRINTFULL;
       }

       if (params->allow_full_extraction)
       {
         permissions |= ACCESS_REV3_COPYALL;
       }

       if (params->allow_annotation)
       {
         permissions |= ACCESS_REV3_ANNOTATE;
       }

       if (params->allow_assembly)
       {
         permissions |= ACCESS_REV3_ASSEMBLE;
       }
    }

    sprintf (buffer, "-sOwnerPassword=%s %s-dEncryptionR=%d -dPermissions=%d ",
             params->owner_password, user, level, permissions);
  }
}

