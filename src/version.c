/* PrintPDF - version.c
 *
 * (C) Stephen Fryatt, 2007
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

#include "version.h"

#include "menus.h"
#include "pmenu.h"
#include "windows.h"

/* ==================================================================================================================
 * Global variables.
 */


/* Function Prototypes. */

int version_menu_tick (version_params *params);


/* ==================================================================================================================
 *
 */

void initialise_version_settings (version_params *params)
{
  params->standard_version = read_config_int ("PDFVersion");
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_version_menu (version_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon, int ident)
{
  if (build_param_menu ("VersionMenu", ident, version_menu_tick (params)) != NULL)
  {
    open_param_menu (pointer, window, icon);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_version_menu (version_params *params, wimp_selection *selection)
{
  params->standard_version = selection->items[0];

  build_param_menu ("VersionMenu", param_menu_ident (), version_menu_tick (params));
}

/* ------------------------------------------------------------------------------------------------------------------ */

int version_menu_tick (version_params *params)
{
  return (params->standard_version);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void fill_version_field (wimp_w window, wimp_i icon, version_params *params)
{
  param_menu_entry (indirected_icon_text (window, icon), "VersionMenu", params->standard_version + 1);
  wimp_set_icon_state (window, icon, 0, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void build_version_params (char *buffer, version_params *params)
{
  char level[100];

  param_menu_entry (level, "VersionList", params->standard_version);

  *buffer = '\0';

  sprintf (buffer, "-dCompatibilityLevel=%s ", level);
}

