/* PrintPDF - ihelp.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"
#include "oslib/help.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "sflib/general.h"
#include "sflib/errors.h"
#include "sflib/icons.h"
#include "sflib/msgs.h"

/* Application header files */

#include "ihelp.h"

#include "menus.h"

/* #include "mainmenu.h" */

/* ==================================================================================================================
 * Global variables.
 */

static ihelp_window	*windows = NULL;

/* ==================================================================================================================
 * Adding and removing windows
 */

void add_ihelp_window(wimp_w window, char* name, void (*decode) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state))
{
	ihelp_window	*new;

	if (find_ihelp_window(window) == NULL) {
		new = malloc(sizeof(ihelp_window));

		if (new != NULL) {
			new->window = window;
			strcpy (new->name, name);
			new->pointer_location = decode;

			new->next = windows;
			windows = new;
		}
	}
}

/* ------------------------------------------------------------------------------------------------------------------ */

void remove_ihelp_window(wimp_w window)
{
	ihelp_window	**list, *del;

	/* Delink the block and delete it. */

	list = &windows;

	while (*list != NULL && (*list)->window != window)
		list = &((*list)->next);

	if (*list != NULL) {
		del = *list;

		*list = del->next;
		free(del);
	}
}

/* ==================================================================================================================
 * Finding windows
 */

ihelp_window *find_ihelp_window(wimp_w window)
{
	ihelp_window		*list;

	list = windows;

	while (list != NULL && list->window != window)
		list = list->next;

	return list;
}

/* ==================================================================================================================
 * Message generation
 */

char *find_ihelp (char *buffer, wimp_w window, wimp_i icon, os_coord pos, wimp_mouse_state buttons)
{
  char           help_text[IHELP_LENGTH], token[128], icon_name[IHELP_INAME_LEN];
  ihelp_window   *win_data;
  wimp_selection menu_selection;
  int            found, i;

  /* Set the buffer to a null string, to return no text if a result isn't found. */

  *buffer = '\0';


  /* Special case, if the window is the iconbar. */

  if (window == wimp_ICON_BAR)
  {
     if (msgs_lookup_result ("Help.IconBar", help_text, IHELP_LENGTH) == 0)
     {
       strcpy (buffer, help_text);
     }
  }


  /* Otherwise, if the window is one of the windows registered for interactive help. */

  else if ((win_data = find_ihelp_window (window)) != NULL)
  {
    found = 0;
    *icon_name = '\0';


    /* If the window supplied a decoding function, call that to get an 'icon name'. */

    if (win_data->pointer_location != NULL)
    {
      (win_data->pointer_location) (icon_name, window, icon, pos, buttons);
    }

    /* If there wasn't a decoding function, or it didn't help, try to get the icon name from the validation string.
     * If the icon isn't validated, make a string of the form IconX where X is the number.
     */

    if (*icon_name == '\0' && icon >= 0)
    {
      if (get_validation_command (icon_name, window, icon, 'N'))
      {
        sprintf (icon_name, "Icon%d", icon);
      }
    }

    /* If an icon name was found from somewhere, look up a token based on that name. */

    if (*icon_name != '\0')
    {
      sprintf (token, "Help.%s.%s", win_data->name, icon_name);
      found = (msgs_lookup_result (token, help_text, IHELP_LENGTH) == 0);
    }

    /* If the icon did not have a name, or it is the window background, look up a token for the window. */

    if (!found)
    {
      sprintf (token, "Help.%s", win_data->name);
      found = (msgs_lookup_result (token, help_text, IHELP_LENGTH) == 0);
    }

    /* If a message was found, return it. */

    if (found)
    {
      strcpy (buffer, help_text);
    }
  }

  /* Otherwise, try the window as a menu structure. */

  else
  {
    wimp_get_menu_state (0, &menu_selection, 0, 0);

    /* The list will be null if this isn't a menu belonging to us (or it isn't a menu at all...). */

    if (menu_selection.items[0] != -1)
    {
      sprintf (token, "Help.%s.", get_current_menu_name (icon_name));

      for (i=0; menu_selection.items[i] != -1; i++)
      {
        sprintf (icon_name, "%02x", menu_selection.items[i]);
        strcat (token, icon_name);
      }

      if (msgs_lookup_result (token, help_text, IHELP_LENGTH) == 0)
      {
        strcpy (buffer, help_text);
      }
    }
  }

  return (buffer);
}

/* ==================================================================================================================
 * Help request handling
 */

int send_reply_help_request (wimp_message *message)
{
  os_error *error;

  help_full_message_request *help_request = (help_full_message_request *) message;
  help_full_message_reply    help_reply;

  find_ihelp (help_reply.reply, help_request->w, help_request->i,  help_request->pos,  help_request->buttons);

  if (*help_reply.reply != '\0')
  {
    help_reply.size = WORDALIGN(21 + strlen (help_reply.reply));
    help_reply.your_ref = help_request->my_ref;
    help_reply.action = message_HELP_REPLY;

    error = xwimp_send_message (wimp_USER_MESSAGE, (wimp_message *) &help_reply, help_request->sender);
    if (error != NULL)
    {
      wimp_os_error_report (error, wimp_ERROR_BOX_CANCEL_ICON);
      return (-1);
    }
  }

  return (0);
}
