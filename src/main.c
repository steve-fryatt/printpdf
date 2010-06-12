/* PrintPDF - main.c
 *
 * (C) Stephen Fryatt, 2007
 */

/* ANSI C header files */


/* OSLib header files */

#include "oslib/wimp.h"
#include "oslib/uri.h"
#include "oslib/os.h"
#include "oslib/osspriteop.h"
#include "oslib/pdriver.h"
#include "oslib/help.h"

/* SF-Lib header files. */

#include "sflib/windows.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/transfer.h"
#include "sflib/url.h"
#include "sflib/msgs.h"
#include "sflib/debug.h"
#include "sflib/config.h"
#include "sflib/errors.h"
#include "sflib/string.h"
#include "sflib/colpick.h"
#include "sflib/event.h"

/* Application header files */

#include "main.h"

#include "bookmark.h"
#include "choices.h"
#include "convert.h"
#include "dataxfer.h"
#include "encrypt.h"
#include "init.h"
#include "ihelp.h"
#include "menus.h"
#include "optimize.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "popup.h"
#include "taskman.h"
#include "version.h"
#include "windows.h"

/* ------------------------------------------------------------------------------------------------------------------ */

/* Declare the global variables that are used. */

int			global_drag_type;
int			global_encryption_dialogue_location;
int			global_optimization_dialogue_location;
int			global_pdfmark_dialogue_location;
int			global_bookmark_dialogue_location;

/* Cross file global variables */

wimp_t			task_handle;
int			quit_flag = FALSE;

osspriteop_area		*wimp_sprites;

/* ==================================================================================================================
 * Main function
 */

int main (int argc, char *argv[])

{
  extern wimp_t task_handle;


  initialise();

  parse_command_line(argc, argv);

  poll_loop();

  terminate_bookmarks();
  msgs_close_file();
  wimp_close_down (task_handle);
  remove_all_remaining_conversions();

  return (0);
}

/* ==================================================================================================================
 * Wimp_Poll loop
 */

int poll_loop (void)
{
	os_t			poll_time;
	wimp_event_no		reason;
	wimp_block		blk;

	extern int		quit_flag;

	poll_time = os_read_monotonic_time();

	while (!quit_flag) {
		reason = wimp_poll_idle(0, &blk, poll_time, 0);

		/* Events are passed to Event Lib first; only if this fails
		 * to handle them do they get passed on to the internal
		 * inline handlers shown here.
		 */

		if (event_process_event(reason, &blk, 0)) {
			switch (reason) {
			case wimp_NULL_REASON_CODE:
				test_and_close_popup (poll_time);
				check_for_ps_file ();
				check_for_pending_files ();
				poll_time += read_config_int ("PollDelay");
				break;

			case wimp_REDRAW_WINDOW_REQUEST:
				redraw_queue_pane (&(blk.redraw));

			case wimp_OPEN_WINDOW_REQUEST:
				wimp_open_window (&(blk.open));
				break;

			case wimp_CLOSE_WINDOW_REQUEST:
				wimp_close_window (blk.close.w);
				break;

			case wimp_MOUSE_CLICK:
				mouse_click_handler (&(blk.pointer));
				break;

			case wimp_KEY_PRESSED:
				key_press_handler (&(blk.key));
				break;

			case wimp_MENU_SELECTION:
				menu_selection_handler (&(blk.selection));
				break;

			case wimp_USER_DRAG_BOX:
				switch (global_drag_type) {
				case DRAG_SAVE:
					terminate_user_drag (&(blk.dragged));
					break;

				case DRAG_QUEUE:
					terminate_queue_entry_drag (&(blk.dragged));
					break;
				}
				break;

			case wimp_USER_MESSAGE:
			case wimp_USER_MESSAGE_RECORDED:
				user_message_handler (&(blk.message));
				break;

			case wimp_USER_MESSAGE_ACKNOWLEDGE:
				bounced_message_handler (&(blk.message));
				break;
			}
		}
	}

	return 0;
}

/* ==================================================================================================================
 * Mouse click handler
 */

void mouse_click_handler (wimp_pointer *pointer)
{
  extern global_windows windows;


  /* Iconbar icon. */

  if (pointer->w == wimp_ICON_BAR)
  {
    switch ((int) pointer->buttons)
    {
      case wimp_CLICK_SELECT:
        create_new_bookmark_window(pointer);
        break;

      case wimp_CLICK_MENU:
        open_iconbar_menu (pointer);
        break;
    }
  }

  /* Program information window. */

  else if (pointer->w == windows.prog_info)
  {
    char temp_buf[256];

    switch ((int) pointer->i)
    {
      case 8: /* Website. */
        msgs_lookup ("SupportURL:http://www.stevefryatt.org.uk/software/", temp_buf, sizeof (temp_buf));
        launch_url (temp_buf);
        if (pointer->buttons == wimp_CLICK_SELECT)
        {
          wimp_create_menu ((wimp_menu *) -1, 0, 0);
        }
        break;
    }
  }

  /* Save PDF Window. */

  else if (pointer->w == windows.save_pdf)
  {
    switch ((int) pointer->i)
    {
      case SAVE_PDF_ICON_FILE:
        if (pointer->buttons == wimp_DRAG_SELECT)
        {
          start_save_window_drag(DRAG_SAVE_PDF);
        }
        break;

      case SAVE_PDF_ICON_OK:
        if (pointer->buttons == wimp_CLICK_SELECT)
        {
          immediate_window_save ();
        }
        break;

      case SAVE_PDF_ICON_CANCEL:
        if (pointer->buttons == wimp_CLICK_SELECT)
        {
          cancel_conversion ();
        }
        break;

      case SAVE_PDF_ICON_QUEUE:
        if (pointer->buttons == wimp_CLICK_SELECT)
        {
          conversion_dialogue_queue ();
          wimp_close_window (windows.save_pdf);
        }
        break;

      case SAVE_PDF_ICON_VERSION_MENU:
        open_convert_version_menu (pointer, windows.save_pdf, SAVE_PDF_ICON_VERSION_FIELD);
        break;

      case SAVE_PDF_ICON_OPT_MENU:
        global_optimization_dialogue_location = OPTIMIZATION_SAVE;
        open_convert_optimize_menu (pointer, windows.save_pdf, SAVE_PDF_ICON_OPT_FIELD);
        break;

      case SAVE_PDF_ICON_ENCRYPT_MENU:
        global_encryption_dialogue_location = ENCRYPTION_SAVE;
        open_convert_encrypt_dialogue (pointer);
        break;

      case SAVE_PDF_ICON_PDFMARK_MENU:
        global_pdfmark_dialogue_location = PDFMARK_SAVE;
        open_convert_pdfmark_dialogue (pointer);
        break;

      case SAVE_PDF_ICON_BOOKMARK_MENU:
        global_bookmark_dialogue_location = BOOKMARK_SAVE;
        open_convert_bookmark_menu(pointer, windows.save_pdf, SAVE_PDF_ICON_BOOKMARK_FIELD);
        break;
    }
  }

  /* Either of the Security Windows. */

  else if (pointer->w == windows.security2 || pointer->w == windows.security3)
  {
    switch ((int) pointer->i)
    {
      case ENCRYPT_ICON_CANCEL:
        wimp_create_menu ((wimp_menu *) -1, 0, 0);
        break;

      case ENCRYPT_ICON_OK:
        switch (global_encryption_dialogue_location)
        {
          case ENCRYPTION_SAVE:
            process_convert_encrypt_dialogue ();
            break;

          case ENCRYPTION_CHOICE:
            process_choices_encrypt_dialogue ();
            break;
        }
        break;
    }
  }

  /* The PDFMark Windows. */

  else if (pointer->w == windows.pdfmark)
  {
    switch ((int) pointer->i)
    {
      case PDFMARK_ICON_CANCEL:
        wimp_create_menu ((wimp_menu *) -1, 0, 0);
        break;

      case PDFMARK_ICON_OK:
        switch (global_pdfmark_dialogue_location)
        {
          case PDFMARK_SAVE:
            process_convert_pdfmark_dialogue ();
            break;

          case PDFMARK_CHOICE:
            process_choices_pdfmark_dialogue ();
            break;
        }
        break;
    }
  }

  /* The Optimization Window. */

  else if (pointer->w == windows.optimization)
  {
    switch ((int) pointer->i)
    {
      case OPTIMIZE_ICON_CANCEL:
        wimp_create_menu ((wimp_menu *) -1, 0, 0);
        break;

      case OPTIMIZE_ICON_OK:
        switch (global_optimization_dialogue_location)
        {
          case OPTIMIZATION_SAVE:
            process_convert_optimize_dialogue ();
            break;

          case OPTIMIZATION_CHOICE:
            process_choices_optimize_dialogue ();
            break;
        }
        break;

      case OPTIMIZE_ICON_COLOUR_RESOLUTION_UP:
        update_resolution_icon(OPTIMIZE_ICON_COLOUR_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_COLOUR_RESOLUTION_DOWN:
        update_resolution_icon(OPTIMIZE_ICON_COLOUR_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_GREY_RESOLUTION_UP:
        update_resolution_icon(OPTIMIZE_ICON_GREY_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_GREY_RESOLUTION_DOWN:
        update_resolution_icon(OPTIMIZE_ICON_GREY_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_MONO_RESOLUTION_UP:
        update_resolution_icon(OPTIMIZE_ICON_MONO_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_MONO_RESOLUTION_DOWN:
        update_resolution_icon(OPTIMIZE_ICON_MONO_RESOLUTION, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_COLOUR_THRESHOLD_UP:
        update_threshold_icon(OPTIMIZE_ICON_COLOUR_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_COLOUR_THRESHOLD_DOWN:
        update_threshold_icon(OPTIMIZE_ICON_COLOUR_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_GREY_THRESHOLD_UP:
        update_threshold_icon(OPTIMIZE_ICON_GREY_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_GREY_THRESHOLD_DOWN:
        update_threshold_icon(OPTIMIZE_ICON_GREY_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_MONO_THRESHOLD_UP:
        update_threshold_icon(OPTIMIZE_ICON_MONO_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_MONO_THRESHOLD_DOWN:
        update_threshold_icon(OPTIMIZE_ICON_MONO_THRESHOLD, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_COLOUR_DEPTH_UP:
        update_depth_icon(OPTIMIZE_ICON_COLOUR_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_COLOUR_DEPTH_DOWN:
        update_depth_icon(OPTIMIZE_ICON_COLOUR_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_GREY_DEPTH_UP:
        update_depth_icon(OPTIMIZE_ICON_GREY_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_GREY_DEPTH_DOWN:
        update_depth_icon(OPTIMIZE_ICON_GREY_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_MONO_DEPTH_UP:
        update_depth_icon(OPTIMIZE_ICON_MONO_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? -1 : 1);
        break;

      case OPTIMIZE_ICON_MONO_DEPTH_DOWN:
        update_depth_icon(OPTIMIZE_ICON_MONO_DEPTH, (pointer->buttons == wimp_CLICK_ADJUST) ? 1 : -1);
        break;

      case OPTIMIZE_ICON_COLOUR_DOWNSAMPLE:
      case OPTIMIZE_ICON_GREY_DOWNSAMPLE:
      case OPTIMIZE_ICON_MONO_DOWNSAMPLE:
      case OPTIMIZE_ICON_COLOUR_ENCODE:
      case OPTIMIZE_ICON_GREY_ENCODE:
      case OPTIMIZE_ICON_MONO_ENCODE:
        shade_optimize_dialogue ();
        break;

      case OPTIMIZE_ICON_COLOUR_SUBSAMPLE:
      case OPTIMIZE_ICON_GREY_SUBSAMPLE:
      case OPTIMIZE_ICON_MONO_SUBSAMPLE:
      case OPTIMIZE_ICON_COLOUR_AVERAGE:
      case OPTIMIZE_ICON_GREY_AVERAGE:
      case OPTIMIZE_ICON_MONO_AVERAGE:
      case OPTIMIZE_ICON_COLOUR_DCT:
      case OPTIMIZE_ICON_GREY_DCT:
      case OPTIMIZE_ICON_MONO_CCITT:
      case OPTIMIZE_ICON_COLOUR_FLATE:
      case OPTIMIZE_ICON_GREY_FLATE:
      case OPTIMIZE_ICON_MONO_FLATE:
      case OPTIMIZE_ICON_MONO_RUNLENGTH:
      case OPTIMIZE_ICON_ROTATE_NONE:
      case OPTIMIZE_ICON_ROTATE_ALL:
      case OPTIMIZE_ICON_ROTATE_PAGE:
        if (pointer->buttons == wimp_CLICK_ADJUST)
        {
          set_icon_selected (pointer->w, pointer->i, TRUE);
        }
        break;
    }
  }

  /* Queue window. */

  else if (pointer->w == windows.queue)
  {
    switch ((int) pointer->i)
    {
      case QUEUE_ICON_CLOSE:
        close_queue_window ();
        break;

      case QUEUE_ICON_CREATE:
        close_queue_window ();
        start_held_conversion ();
        break;
    }
  }

  /* Queue pane. */

  else if (pointer->w == windows.queue_pane)
  {
    queue_pane_click (pointer);
  }

  /* Choices Window. */

  else if (pointer->w == windows.choices)
  {
    switch ((int) pointer->i)
    {
      case CHOICE_ICON_APPLY:
        if (pointer->buttons == wimp_CLICK_SELECT || pointer->buttons == wimp_CLICK_ADJUST)
        {
          read_choices_window ();

          if (pointer->buttons == wimp_CLICK_SELECT)
          {
            close_choices_window ();
          }
        }
        break;

      case CHOICE_ICON_SAVE:
        if (pointer->buttons == wimp_CLICK_SELECT || pointer->buttons == wimp_CLICK_ADJUST)
        {
          read_choices_window ();
          save_configuration ();

          if (pointer->buttons == wimp_CLICK_SELECT)
          {
            close_choices_window ();
          }
        }
        break;

      case CHOICE_ICON_CANCEL:
        if (pointer->buttons == wimp_CLICK_SELECT)
        {
          close_choices_window ();
        }
        else if (pointer->buttons == wimp_CLICK_ADJUST)
        {
          set_choices_window ();
          redraw_choices_window ();
        }
        break;

      case CHOICE_ICON_VERSION_MENU:
        open_choices_version_menu (pointer, windows.choices, CHOICE_ICON_VERSION);
        break;

      case CHOICE_ICON_OPTIMIZE_MENU:
        global_optimization_dialogue_location = OPTIMIZATION_CHOICE;
        open_choices_optimize_menu (pointer, windows.choices, CHOICE_ICON_OPTIMIZE);
        break;

      case CHOICE_ICON_ENCRYPT_MENU:
        global_encryption_dialogue_location = ENCRYPTION_CHOICE;
        open_choices_encrypt_dialogue (pointer);
        break;

      case CHOICE_ICON_INFO_MENU:
        global_pdfmark_dialogue_location = PDFMARK_CHOICE;
        open_choices_pdfmark_dialogue (pointer);
        break;
    }
  }

  /* PopUp Info Window */

  else if (pointer->w == windows.popup)
  {
    if (pointer->buttons == wimp_CLICK_SELECT)
    {
      close_popup ();
    }
  }
}

/* ==================================================================================================================
 * Keypress handler
 */

void key_press_handler (wimp_key *key)
{
  extern global_windows windows;


  /* Save PDF window */

  if (key->w == windows.save_pdf)
  {
    switch (key->c)
    {
      case wimp_KEY_RETURN:
        immediate_window_save ();
        break;

      case wimp_KEY_ESCAPE:
        cancel_conversion ();
        break;

      default:
        wimp_process_key (key->c);
        break;
    }
  }

  /* The PDFMark Window. */

  else if (key->w == windows.pdfmark)
  {
    switch (key->c)
    {
      case wimp_KEY_RETURN:
        switch (global_pdfmark_dialogue_location)
        {
          case PDFMARK_SAVE:
            process_convert_pdfmark_dialogue ();
            break;

          case PDFMARK_CHOICE:
            process_choices_pdfmark_dialogue ();
            break;
        }
        break;

      case wimp_KEY_ESCAPE:
        wimp_create_menu ((wimp_menu *) -1, 0, 0);
        break;

      default:
        wimp_process_key (key->c);
        break;
    }
  }

  /* Either of the Security Windows. */

  else if (key->w == windows.security2 || key->w == windows.security3)
  {
    if (key->i == ENCRYPT_ICON_OWNER_PW)
    {
      shade_encrypt_dialogue (0);
    }

    switch (key->c)
    {
      case wimp_KEY_RETURN:
        switch (global_encryption_dialogue_location)
        {
          case ENCRYPTION_SAVE:
            process_convert_encrypt_dialogue ();
            break;

          case ENCRYPTION_CHOICE:
            process_choices_encrypt_dialogue ();
            break;
        }
        break;

      case wimp_KEY_ESCAPE:
        wimp_create_menu ((wimp_menu *) -1, 0, 0);
        break;

      default:
        wimp_process_key (key->c);
        break;
    }
  }

  /* The Optimization Window. */

  else if (key->w == windows.optimization)
  {
    switch (key->c)
    {
      case wimp_KEY_RETURN:
        switch (global_optimization_dialogue_location)
        {
          case OPTIMIZATION_SAVE:
            process_convert_optimize_dialogue ();
            break;

          case OPTIMIZATION_CHOICE:
            process_choices_optimize_dialogue ();
            break;
        }
        break;

      case wimp_KEY_ESCAPE:
        wimp_create_menu ((wimp_menu *) -1, 0, 0);
        break;

      default:
        wimp_process_key (key->c);
        break;
    }
  }


  /* Choices window */

  else if (key->w == windows.choices)
  {
    switch (key->c)
    {
      case wimp_KEY_RETURN:
        read_choices_window ();
        save_configuration ();
        close_choices_window ();
        break;

      case wimp_KEY_ESCAPE:
        close_choices_window ();
        break;

      default:
        wimp_process_key (key->c);
        break;
    }
  }
}

/* ==================================================================================================================
 * Menu selection handler
 */

void menu_selection_handler (wimp_selection *selection)
{
	wimp_pointer		pointer;

	extern global_menus	menus;


	/* Store the mouse status before decoding the menu. */

	wimp_get_pointer_info (&pointer);

	/* Decode the individual menus. */

	if (menus.menu_up == menus.icon_bar)
		decode_iconbar_menu (selection, &pointer);
	else if (menus.menu_up == menus.params)
		decode_param_menu (selection, &pointer);
	else if (menus.menu_up == menus.bookmarks_list)
		process_convert_bookmark_menu(selection);

	/* If Adjust was used, reopen the menu. */

	if (pointer.buttons == wimp_CLICK_ADJUST)
		wimp_create_menu (menus.menu_up, 0, 0);
}

/* ==================================================================================================================
 * User message handlers
 */

void user_message_handler (wimp_message *message)
{
  extern int          quit_flag;
  extern wimp_t       task_handle;

  switch (message->action)
  {
    case message_QUIT:
      quit_flag=TRUE;
      break;

    case message_URI_RETURN_RESULT:
      url_bounce (message);
      break;

    case message_DATA_SAVE:
      if (message->sender != task_handle) /* We don't want to respond to our own save requests. */
      {
        message_data_save_reply (message);
      }
      break;

    case message_DATA_SAVE_ACK:
      send_reply_data_save_ack (message);
      break;

    case message_DATA_LOAD:
      message_data_load_reply (message);
      break;

    case message_DATA_OPEN:
      start_data_open_load(message);
      break;

    case message_HELP_REQUEST:
      send_reply_help_request (message);
      break;

    case message_TASK_INITIALISE:
      check_new_task (message);
      break;

    case message_TASK_CLOSE_DOWN:
      check_for_conversion_end (message->sender);
      break;
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void bounced_message_handler (wimp_message *message)
{
  switch (message->action)
  {
    case message_ANT_OPEN_URL:
      url_bounce (message);
      break;
  }
}
