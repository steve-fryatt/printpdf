/* PrintPDF - ihelp.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef _PRINTPDF_IHELP
#define _PRINTPDF_IHELP

/* ==================================================================================================================
 * Static constants
 */

#define IHELP_LENGTH 236
#define IHELP_INAME_LEN 64

/* ==================================================================================================================
 * Data structures
 */

typedef struct ihelp_window
{
  wimp_w window;
  char   name[13];
  void   (*pointer_location) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state);

  struct ihelp_window *next;
}
ihelp_window;

typedef struct ihelp_menu {
	wimp_menu	*menu;
	char		name[13];
	void		(*pointer_location) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state);

  struct ihelp_menu	*next;
} ihelp_menu;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Adding and removing windows */

void add_ihelp_window (wimp_w window, char* name, void (*decode) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state));
void remove_ihelp_window (wimp_w window);

/* Finding windows */

ihelp_window *find_ihelp_window (wimp_w window);

/* Token generation */

char *find_ihelp (char *buffer, wimp_w window, wimp_i icon, os_coord pos, wimp_mouse_state buttons);

/* Help request handling. */

int send_reply_help_request (wimp_message *message);

#endif
