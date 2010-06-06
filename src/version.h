/* PrintPDF - version.h
 *
 * (c) Stephen Fryatt, 2007
 */

#ifndef _PRINTPDF_VERSION
#define _PRINTPDF_VERSION

/* ==================================================================================================================
 * Static constants
 */

/* ==================================================================================================================
 * Data structures
 */

typedef struct version_params
{
  int  standard_version;
}
version_params;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Handle the optimization window and menu. */

void initialise_version_settings (version_params *params);
void open_version_menu (version_params *params, wimp_pointer *pointer, wimp_w window, wimp_i icon, int ident);
void process_version_menu (version_params *params, wimp_selection *selection);
void fill_version_field (wimp_w window, wimp_i icon, version_params *params);
void build_version_params (char *buffer, version_params *params);


#endif
