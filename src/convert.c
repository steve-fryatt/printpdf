/* PrintPDF - convert.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/fileswitch.h"
#include "oslib/osfscontrol.h"
#include "oslib/os.h"
#include "oslib/wimp.h"
#include "oslib/dragasprite.h"
#include "oslib/wimpspriteop.h"
#include "oslib/osspriteop.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/debug.h"
#include "sflib/event.h"
#include "sflib/string.h"
#include "sflib/windows.h"
#include "sflib/menus.h"
#include "sflib/icons.h"
#include "sflib/msgs.h"
#include "sflib/errors.h"

/* Application header files */

#include "convert.h"

#include "bookmark.h"
#include "encrypt.h"
#include "main.h"
#include "optimize.h"
#include "pdfmark.h"
#include "pmenu.h"
#include "popup.h"
#include "version.h"
#include "windows.h"

/* ****************************************************************************
 * Function Prototypes
 * ****************************************************************************/

/* Defer queue manipulation. */

void			redraw_queue_pane(wimp_draw *redraw);
void			queue_pane_click(wimp_pointer *pointer);
void			terminate_queue_entry_drag(wimp_dragged *drag, void *data);

/* ==================================================================================================================
 * Global variables.
 */

static queued_file	*queue = NULL;
static int		conversion_in_progress = FALSE;
static int		files_pending_attention = TRUE;
static wimp_t		conversion_task = 0;

static queued_file	**queue_redraw_list = NULL;
static int		queue_redraw_lines = 0;

static int		dragging_sprite;
static int		dragging_start_line;

/* Conversion parameters. */

static encrypt_params	encryption;
static optimize_params	optimization;
static version_params	version;
static pdfmark_params	pdfmark;
static bookmark_params	bookmark;

/* ==================================================================================================================
 * Initialisation
 */

/**
 * Test for the presence of the queue directory, and create it if it is not already there.
 *
 * At present, this assumes that it will be just the leaf directory missing; true assuming an address in the Scrap
 * folder.
 *
 * Also set up the conversion parameters for the first time, for each of the modules.
 */

void initialise_conversion(void)
{
	char			*queue_dir;
	fileswitch_object_type	type;

	extern global_windows	windows;


	/* Set handlers for the queue window. */

	event_add_window_redraw_event(windows.queue_pane, redraw_queue_pane);
	event_add_window_mouse_event(windows.queue_pane, queue_pane_click);

	/* Set up the queue directory */

	queue_dir = read_config_str("FileQueue");

	xosfile_read_no_path(queue_dir, &type, NULL, NULL, NULL, NULL);

	if (type == fileswitch_NOT_FOUND)
		osfile_create_dir(queue_dir, 0);
	else if (type != fileswitch_IS_DIR)
		wimp_msgtrans_error_report("NoQueueDir");

	/* Initialise the options. */

	strcpy (indirected_icon_text(windows.save_pdf, SAVE_PDF_ICON_NAME), read_config_str("FileName"));
	strcpy (indirected_icon_text(windows.save_pdf, SAVE_PDF_ICON_USERFILE), read_config_str("PDFMarkUserFile"));
	set_icon_selected(windows.save_pdf, SAVE_PDF_ICON_PREPROCESS, read_config_opt("PreProcess"));

	initialise_encryption_settings(&encryption);
	initialise_optimization_settings(&optimization);
	initialise_version_settings(&version);
	initialise_pdfmark_settings(&pdfmark);
	initialise_bookmark_settings(&bookmark);
}

/* ==================================================================================================================
 * Queueing files.
 */

/* Check the location of the 'print file' to see if one has appeared.  If it has, add it to the file queue.
 *
 * Called from NULL poll events.
 */

void check_for_ps_file (void)
{
  fileswitch_object_type type;
  int                    size;
  char                   check_file[512];

  sprintf (check_file, "%s.printout/ps", read_config_str("FileQueue"));

  xosfile_read_stamped_no_path(check_file, &type, NULL, NULL, &size, NULL, NULL);

  if (type == fileswitch_IS_FILE && size > 0)
  {
    if (queue_ps_file (check_file))
    {
      xosfile_delete (check_file, NULL, NULL, NULL, NULL, NULL);
    }
  }

  /* Handle PDFMaker jobs, for compatibility with R-Comp's system.  If PDFMaker: is set up, check
   * to see if there is a job file called PS on the path.
   */

  os_read_var_val_size ("PDFMaker$Path", 0, 0, &size, NULL);

  if (size != 0)
  {
    strcpy (check_file, "PDFMaker:PS");

    xosfile_read_stamped_no_path(check_file, &type, NULL, NULL, &size, NULL, NULL);

    if (type == fileswitch_IS_FILE && size > 0)
    {
      if (queue_ps_file (check_file))
      {
        xosfile_delete (check_file, NULL, NULL, NULL, NULL, NULL);
      }
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Take the file specified, copy it with a timestamp and add it to the queue of files.
 */

int queue_ps_file (char *filename)
{
  queued_file *new, **list = NULL;
  char        queued_filename[512];
  os_error    *error;
  os_fw       file;

  /* Try and open the file, to see if it is already open.  If we fail for any reason, return with an error to
   * show that the queuing failed.
   */

  error = xosfind_openupw (osfind_NO_PATH, filename, NULL, &file);

  if (error != NULL || file == 0)
  {
    return (0);
  }

  osfind_closew (file);

  /* Allocate memory and copy the file on to the queue. */

  new = malloc (sizeof (queued_file));

  if (new != NULL)
  {
    sprintf (new->filename, "%x", (int) os_read_monotonic_time ());
    *(new->display_name) = '\0';
    new->object_type = PENDING_ATTENTION;
    new->next = NULL;

    list = &queue;

    while (*list != NULL)
    {
      list = &((*list)->next);
    }

    *list = new;

    sprintf (queued_filename, "%s.%s", read_config_str("FileQueue"), new->filename);
    error = xosfscontrol_copy (filename, queued_filename, osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

    if (error != NULL)
    {
      *list = NULL;
      free (new);
      return (0);
    }
    else
    {
      files_pending_attention = TRUE;
    }
  }

  return (1);
}

/* ==================================================================================================================
 * Starting conversions
 */

/* Test to see if there is a file queued and no conversion taking place.  If these are both true, select the next
 * pending file in the queue and open the Save PDF dialogue.
 *
 * Called from NULL poll events.
 */

void check_for_pending_files (void)
{
  queued_file  *list;

  extern global_windows windows;


  /* We can't start a conversion if:
   *
   * - The Choices window is open (the options menus would get confused)
   * - There is a conversion in progress
   * - There isn't anything to convert (Duh!)
   */

  if (!window_is_open (windows.choices) && !conversion_in_progress && files_pending_attention && queue != NULL)
  {
    list = queue;

    files_pending_attention = FALSE;
    conversion_in_progress = FALSE;

    /* Scan thtough the queue.  The first file PENDING_ATTENTION is turned into BEING_PROCESSED.  If there are
     * more files PENDING_ATTENTION, this is reflected in the files_pending_attention flag to save re-scanning
     * the queue each NULL Poll.
     */

    while (list != NULL)
    {
      if (list->object_type == PENDING_ATTENTION)
      {
        if (!conversion_in_progress)
        {
          list->object_type = BEING_PROCESSED;
          conversion_in_progress = TRUE;
        }
        else
        {
          files_pending_attention = TRUE;
        }
      }
      list = list->next;
    }

    /* If a file was found to convert, open the Save PDF dialogue. */

    if (conversion_in_progress)
    {
      open_conversion_dialogue ();
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Start a conversion on files held in the deferred queue.  This is called by a user action, probably clicking
 * Convert in the queue dialogue.
 */

void start_held_conversion (void)
{
  queued_file  *list;

  extern global_windows windows;


  /* We can't start a conversion if:
   *
   * - The Choices window is open (the options menus would get confused)
   * - There is a conversion in progress
   */

  if (!window_is_open (windows.choices) && !conversion_in_progress && queue != NULL)
  {
    list = queue;

    files_pending_attention = FALSE;
    conversion_in_progress = FALSE;

    /* Scan thtough the queue.  The any files HELD_IN_QUEUE are turned into BEING_PROCESSED.  If there are
     * files PENDING_ATTENTION, this is reflected in the files_pending_attention flag to save re-scanning
     * the queue each NULL Poll.
     */

    while (list != NULL)
    {
      if (list->object_type == PENDING_ATTENTION)
      {
        files_pending_attention = TRUE;
      }
      if (list->object_type == HELD_IN_QUEUE && list->include == TRUE)
      {
        list->object_type = BEING_PROCESSED;
        conversion_in_progress = TRUE;
      }
      list = list->next;
    }

    /* If a file or files were found to convert, open the Save PDF dialogue. */

    if (conversion_in_progress)
    {
      open_conversion_dialogue ();
    }
  }
}

/* ==================================================================================================================
 * Handling Save PDF dialogue
 */

/**
 * Open the Save PDF dialogue on screen, at the pointer.
 */

void open_conversion_dialogue(void)
{
	wimp_pointer		pointer;

	extern global_windows	windows;

	/* Set up and open the conversion window. */

	if (read_config_opt ("ResetParams")) {
		strcpy(indirected_icon_text (windows.save_pdf, SAVE_PDF_ICON_NAME), read_config_str ("FileName"));
		strcpy(indirected_icon_text (windows.save_pdf, SAVE_PDF_ICON_USERFILE), read_config_str ("PDFMarkUserFile"));
		set_icon_selected(windows.save_pdf, SAVE_PDF_ICON_PREPROCESS, read_config_opt ("PreProcess"));
		initialise_encryption_settings(&encryption);
		initialise_optimization_settings(&optimization);
		initialise_version_settings(&version);
		initialise_pdfmark_settings(&pdfmark);
		initialise_bookmark_settings(&bookmark);
	}

	fill_version_field(windows.save_pdf, SAVE_PDF_ICON_VERSION_FIELD, &version);
	fill_optimization_field(windows.save_pdf, SAVE_PDF_ICON_OPT_FIELD, &optimization);
	fill_encryption_field(windows.save_pdf, SAVE_PDF_ICON_ENCRYPT_FIELD, &encryption);
	fill_pdfmark_field(windows.save_pdf, SAVE_PDF_ICON_PDFMARK_FIELD, &pdfmark);
	fill_bookmark_field(windows.save_pdf, SAVE_PDF_ICON_BOOKMARK_FIELD, &bookmark);

	wimp_get_pointer_info(&pointer);

	open_window_centred_at_pointer(windows.save_pdf, &pointer);
	put_caret_at_end(windows.save_pdf, SAVE_PDF_ICON_NAME);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Handle the closure of the file save dialogue following a successful data xfer protocol or similar.
 * The settings are retrieved, and a conversion process is started.
 */

void conversion_dialogue_end (char *output_file)
{
  conversion_params params;

  extern global_windows windows;

  /* Sort out the filenames. */

  terminate_ctrl_str (output_file);

  strcpy (params.output_filename, output_file);
  strcpy (indirected_icon_text (windows.save_pdf, SAVE_PDF_ICON_NAME), output_file);

  /* Read and store the options from the window. */

  params.preprocess_in_ps2ps = read_icon_selected (windows.save_pdf, SAVE_PDF_ICON_PREPROCESS);
  ctrl_strcpy (params.pdfmark_userfile, indirected_icon_text (windows.save_pdf, SAVE_PDF_ICON_USERFILE));

  /* Launch the conversion process. */

  conversion_in_progress = conversion_progress (&params);

  if (!conversion_in_progress)
  {
    conversion_task = 0;
    conversion_in_progress = FALSE;
    remove_current_conversion ();
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Handle the closure of the file save dialogue, following a click on the Queue icon.  Any files in the queue being
 * processed are changed to HELD_IN_QUEUE.
 */

void conversion_dialogue_queue (void)
{
  char        *leafname, filename[MAX_FILENAME];
  queued_file *list;

  extern global_windows windows;


  /* Sort out the filenames. */

  ctrl_strcpy (filename, indirected_icon_text (windows.save_pdf, SAVE_PDF_ICON_NAME));
  leafname = find_leafname (filename);

  list = queue;

  while (list != NULL)
  {
    if (list->object_type == BEING_PROCESSED)
    {
      list->object_type = HELD_IN_QUEUE;

      strcpy (list->display_name, leafname);
      (list->display_name)[MAX_DISPLAY_NAME-1] = '\0';

      list->include = TRUE;
    }

    list = list->next;
  }

  if (window_is_open (windows.queue))
  {
    reorder_queue_from_index ();
    rebuild_queue_index ();
    force_visible_window_redraw (windows.queue_pane);
  }

  conversion_in_progress = FALSE;
}

/* ------------------------------------------------------------------------------------------------------------------ */

void handle_save_icon_drop (wimp_full_message_data_xfer *dataload)
{
  char *extension, *leaf, path[256];

  extern global_windows windows;


  if (dataload != NULL && dataload->w == windows.save_pdf)
  {
    switch (dataload->i)
    {
    case SAVE_PDF_ICON_NAME:
      strcpy (path, dataload->file_name);

      extension = find_extension (path);
      leaf = lose_extension (path);
      find_pathname (path);

      if (strcmp_no_case (extension, "pdf") != 0)
      {
        snprintf (indirected_icon_text (windows.save_pdf, SAVE_PDF_ICON_NAME), 256, "%s.%s/pdf", path, leaf);

        replace_caret_in_window (dataload->w);
        wimp_set_icon_state (dataload->w, dataload->i, 0, 0);
      }
      break;

    case SAVE_PDF_ICON_USERFILE:
      if (dataload->file_type == 0xfff)
      {
        strcpy (indirected_icon_text (windows.save_pdf, SAVE_PDF_ICON_USERFILE), dataload->file_name);
        replace_caret_in_window (dataload->w);
        wimp_set_icon_state (dataload->w, dataload->i, 0, 0);
      }
      break;
    }
  }
}

/* ==================================================================================================================
 * Ghostscript conversion progress
 */

/* Start or progress a conversion.
 *
 * This function maintains state between calls, so it can be called to move the conversion on from one stage to
 * the next as the child tasks terminate.
 */

int conversion_progress (conversion_params *params)
{
  static int             conversion_state = CONVERSION_STOPPED;
  static char            output_file[MAX_FILENAME];
  static char            pdfmark_file[MAX_FILENAME];
  static int             preprocess_in_ps2ps;

  char                   intermediate_file[MAX_FILENAME], *intermediate_leaf="inter";
  queued_file            *list, *new, **end = NULL;
  os_error               *err;

  /* If conversion parameters have been passed in and the conversion is stopped, reset and start a new process.
   */

  if (conversion_state == CONVERSION_STOPPED && params != NULL)
  {
    strcpy (output_file, params->output_filename);
    strcpy (pdfmark_file, params->pdfmark_userfile);

    preprocess_in_ps2ps = params->preprocess_in_ps2ps;

    conversion_state = CONVERSION_STARTING;
  }

  /* The state machine to handle the steps in the process. */

  switch (conversion_state)
  {
    case CONVERSION_STARTING:
      err = xosfile_create (output_file, 0xdeaddead, 0xdeaddead, 0);

      if (err == NULL)
      {
        if (preprocess_in_ps2ps)
        {
          sprintf (intermediate_file, "%s.%s", read_config_str("FileQueue"), intermediate_leaf);
          conversion_state = (launch_ps2ps (intermediate_file)) ? CONVERSION_STOPPED : CONVERSION_PS2PS;
        }
        else
        {
          conversion_state = (launch_ps2pdf (output_file, pdfmark_file)) ? CONVERSION_STOPPED : CONVERSION_PS2PDF;
        }
      }
      else
      {
        wimp_msgtrans_error_report ("FOpenFailed");
        conversion_state = CONVERSION_STOPPED;
      }
      break;

    case CONVERSION_PS2PS:
      list = queue;

      while (list != NULL)
      {
        if (list->object_type == BEING_PROCESSED)
        {
          list->object_type = DISCARDED;
        }

        end = &(list->next);

        list = list->next;
      }

      new = malloc (sizeof (queued_file));

      if (new != NULL)
      {
        strcpy (new->filename, intermediate_leaf);
        *(new->display_name) = '\0';
        new->object_type = BEING_PROCESSED;
        new->next = NULL;

        if (end != NULL)
        {
          *end = new;
        }

        conversion_state = (launch_ps2pdf (output_file, pdfmark_file)) ? CONVERSION_STOPPED : CONVERSION_PS2PDF;
      }
      else
      {
        conversion_state = CONVERSION_STOPPED;
      }

      break;

    case CONVERSION_PS2PDF:
      osfile_set_type (output_file, 0xadf);

      if (read_config_opt ("PopUpAfter"))
      {
        open_popup (read_config_int("PopUpTime"));
      }
      conversion_state = CONVERSION_STOPPED;
      break;
  }

  /* Exit, signalling true if the process has ended. */

  return (conversion_state != CONVERSION_STOPPED);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Launch ps2ps.
 */

int launch_ps2ps (char *file_out)
{
  char        command[1024], taskname[32];
  queued_file *list;

  FILE        *param_file;
  os_error    *error = NULL;

  msgs_lookup ("ChildTaskName", taskname, sizeof (taskname));

  param_file = fopen (read_config_str("ParamFile"), "w");
  if (param_file != NULL)
  {
    /* Write all the conversion options and filename details to the gs parameters file. */

    fprintf (param_file, "-dSAFER -q -dNOPAUSE -dBATCH -sDEVICE=pswrite -sOutputFile=%s", file_out);

    list = queue;

    while (list != NULL)
    {
      if (list->object_type == BEING_PROCESSED)
      {
        fprintf (param_file, " %s.%s", read_config_str("FileQueue"), list->filename);
      }

      list = list->next;
    }

    fclose (param_file);

    /* Write all the taskwindow command line details to the command string. */

    sprintf (command,
             "TaskWindow \"gs @%s\" %dk -name \"%s\" -quit",
             read_config_str("ParamFile"), read_config_int("TaskMemory"), taskname);

    /* Launch the conversion task. */

    error = xwimp_start_task (command, &conversion_task);
  }

  return (error != NULL || conversion_task == 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Launch ps2pdf, using the file at the top of the queue and the filename retrieved from the Save dialogue.
 * This will either be at a drag end, or as a result of the user clicking 'OK' on a full filename.
 *
 * To get around command line length restrictions on RISC OS 3.x, we dump the bulk of the parameters into a file
 * in PipeFS and pass this in to gs as a parameters file using the @ parameter.
 */

int launch_ps2pdf (char *file_out, char *user_pdfmark_file)
{
  char        command[1024], taskname[32], encrypt_buf[1024], optimize_buf[1024], version_buf[1024];
  queued_file *list;

  FILE         *param_file, *pdfmark_file;
  os_error     *error = NULL;

  msgs_lookup ("ChildTaskName", taskname, sizeof (taskname));

  param_file = fopen (read_config_str("ParamFile"), "w");
  if (param_file != NULL)
  {
    /* Generate a PDFMark file if necessary. */

    if (pdfmark_data_available(&pdfmark) || bookmark_data_available(&bookmark))
    {
      pdfmark_file = fopen (read_config_str ("PDFMarkFile"), "w");

      if (pdfmark_file != NULL)
      {
        write_pdfmark_docinfo_file(pdfmark_file, &pdfmark);
        write_pdfmark_out_file(pdfmark_file, &bookmark);

        fclose(pdfmark_file);
      }
    }

    /* Write all the conversion options and filename details to the gs parameters file. */

    build_version_params (version_buf, &version);
    build_optimization_params (optimize_buf, &optimization);
    build_encryption_params (encrypt_buf, &encryption, version.standard_version >= 2);

    fprintf (param_file,
             "-dSAFER %s%s%s"
             "-q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite "
             "-sOutputFile=%s -c .setpdfwrite save pop -f",
             version_buf, optimize_buf, encrypt_buf, file_out);

    list = queue;

    while (list != NULL)
    {
      if (list->object_type == BEING_PROCESSED)
      {
        fprintf (param_file, " %s.%s", read_config_str("FileQueue"), list->filename);
      }

      list = list->next;
    }

    if (osfile_read_stamped_no_path (read_config_str ("PDFMarkFile"), NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_FILE)
    {
      /* If there is a PDFMark file, pass that in too. */

      fprintf (param_file, " %s", read_config_str ("PDFMarkFile"));
    }

    if (*user_pdfmark_file != '\0' &&
            osfile_read_stamped_no_path (user_pdfmark_file, NULL, NULL, NULL, NULL, NULL) == fileswitch_IS_FILE)
   {
      /* If there is a PDFMark User File, pass that in too. */

      fprintf (param_file, " %s", user_pdfmark_file);
    }

    fclose (param_file);

    /* Write all the taskwindow command line details to the command string. */

    sprintf (command,
             "TaskWindow \"gs @%s\" %dk -name \"%s\" -quit",
             read_config_str("ParamFile"), read_config_int("TaskMemory"), taskname);

    #ifdef DEBUG
    debug_printf ("Command (length %d): '%s'", strlen(command), command);
    #endif

    /* Launch the conversion task. */

    error = xwimp_start_task (command, &conversion_task);
  }

  return (error != NULL || conversion_task == 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Called from Message_TaskCloseDown, to see if the task that ended had the same task handle as the current
 * conversion task.  If it did, establish what kind of conversion was underway:
 *
 * - If it was *ps2ps, take the intermediate file and pass it on to *ps2pdf.
 * - If it was *ps2pdf, reset the flags and take the original queued object from the queue head.
 */

void check_for_conversion_end (wimp_t ending_task)
{
  if (ending_task == conversion_task)
  {
    if (!conversion_progress (NULL))
    {
      conversion_task = 0;
      conversion_in_progress = CONVERSION_STOPPED;
      remove_current_conversion ();
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Called to cancel the conversion that is being set up.  Reset the flags, close the window and remove the item from
 * the queue.
 */

void cancel_conversion (void)
{
  extern global_windows windows;

  wimp_close_window (windows.save_pdf);
  conversion_task = 0;
  conversion_in_progress = FALSE;

  remove_current_conversion ();
}

/**
 * Called by modules to ask the converion system to re-validate its parameters.
 */

void convert_validate_params(void)
{
	extern global_windows	windows;

	if (bookmark_validate_params(&bookmark))
		fill_bookmark_field(windows.save_pdf, SAVE_PDF_ICON_BOOKMARK_FIELD, &bookmark);
}

/* ==================================================================================================================
 * Handle pop-up menus from the dialogue.
 */

void open_convert_version_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon)
{
  open_version_menu (&version, pointer, window, icon, PARAM_MENU_CONVERT_VERSION);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_convert_version_menu (wimp_selection *selection)
{
  extern global_windows windows;

  process_version_menu (&version, selection);

  fill_version_field (windows.save_pdf, SAVE_PDF_ICON_VERSION_FIELD, &version);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_convert_optimize_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon)
{
  open_optimize_menu (&optimization, pointer, window, icon, PARAM_MENU_CONVERT_OPTIMIZE);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_convert_optimize_menu (wimp_selection *selection)
{
  extern global_windows windows;

  process_optimize_menu (&optimization, selection);

  fill_optimization_field (windows.save_pdf, SAVE_PDF_ICON_OPT_FIELD, &optimization);
}

/**
 * Open the bookmark pop-up menu in the save window.
 */

void open_convert_bookmark_menu(wimp_pointer *pointer, wimp_w window, wimp_i icon)
{
	open_bookmark_menu(&bookmark, pointer, window, icon);
}

/**
 * Process menu selections from the bookmark pop-up in the save window.
 */

void process_convert_bookmark_menu(wimp_selection *selection)
{
	extern global_windows	windows;

	process_bookmark_menu(&bookmark, selection);
	fill_bookmark_field(windows.save_pdf, SAVE_PDF_ICON_BOOKMARK_FIELD, &bookmark);
}

/* ==================================================================================================================
 * Handle Encryption and Optimization dialogues.
 */

void open_convert_pdfmark_dialogue (wimp_pointer *pointer)
{
  open_pdfmark_dialogue (&pdfmark, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_convert_pdfmark_dialogue (void)
{
  extern global_windows windows;

  process_pdfmark_dialogue (&pdfmark);

  fill_pdfmark_field (windows.save_pdf, SAVE_PDF_ICON_PDFMARK_FIELD, &pdfmark);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void open_convert_encrypt_dialogue (wimp_pointer *pointer)
{
  open_encrypt_dialogue (&encryption, version.standard_version >= 2, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_convert_encrypt_dialogue (void)
{
  extern global_windows windows;

  process_encrypt_dialogue (&encryption);

  fill_encryption_field (windows.save_pdf, SAVE_PDF_ICON_ENCRYPT_FIELD, &encryption);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void process_convert_optimize_dialogue (void)
{
  extern global_windows windows;

  process_optimize_dialogue (&optimization);

  fill_optimization_field (windows.save_pdf, SAVE_PDF_ICON_OPT_FIELD, &optimization);
}

/* ==================================================================================================================
 * Dequeueing files.
 */

/* Remove the current item or items from the queue, deleting them from the Scrap directory.
 */

void remove_current_conversion (void)
{
  queued_file           **list, *old;
  char                  old_file[512];

  list = &queue;

  while (*list != NULL)
  {
    if ((*list)->object_type == BEING_PROCESSED || (*list)->object_type == DISCARDED)
    {
      old = (*list);
      sprintf (old_file, "%s.%s", read_config_str("FileQueue"), old->filename);
      xosfile_delete (old_file, NULL, NULL, NULL, NULL, NULL);

      *list = ((*list)->next);

      free (old);
    }
    else
    {
      list = &((*list)->next);
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Remove deleted items from the queue, deleting them from the Scrap directory.
 */

void remove_deleted_files (void)
{
  queued_file           **list, *old;
  char                  old_file[512];

  list = &queue;

  while (*list != NULL)
  {
    if ((*list)->object_type == DELETED)
    {
      old = (*list);
      sprintf (old_file, "%s.%s", read_config_str("FileQueue"), old->filename);
      xosfile_delete (old_file, NULL, NULL, NULL, NULL, NULL);

      *list = ((*list)->next);

      free (old);
    }
    else
    {
      list = &((*list)->next);
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Remove the first item from the queue, deleting it from the Scrap directory.
 */

void remove_first_conversion (void)
{
  queued_file           *old;
  char                  old_file[512];

  old = queue;
  sprintf (old_file, "%s.%s", read_config_str("FileQueue"), old->filename);
  xosfile_delete (old_file, NULL, NULL, NULL, NULL, NULL);

  queue = old->next;
  free (old);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Remove all the items from the queue, and delete their files.
 */

void remove_all_remaining_conversions (void)
{
  while (queue != NULL)
  {
    remove_first_conversion ();
  }
}

/* ==================================================================================================================
 * External interfaces
 */

/* Return an indication that a conversion is underway. */

int pdf_conversion_in_progress (void)
{
  return (conversion_in_progress);
}

/**
 * Test for queued files and ask the user if they need to be saved.
 *
 * \return			1 if there are files to be saved; else 0.
 */

int pending_files_in_queue(void)
{
	int		button = -1;

	if (queue != NULL)
		button = wimp_msgtrans_question_report("PendingJobs", "PendingJobsB");

	return (button == 2);
}

/* ==================================================================================================================
 * Defer queue manipulation
 */

void open_queue_window (wimp_pointer *pointer)
{
  extern global_windows windows;

  open_pane_dialogue_centred_at_pointer (windows.queue, windows.queue_pane, QUEUE_ICON_PANE, 40, pointer);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void close_queue_window (void)
{
  extern global_windows windows;

  reorder_queue_from_index ();
  remove_deleted_files ();
  wimp_close_window (windows.queue);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* We need to fix this so that memory allocation is done correctly.
 * This code should also resize the queue window pane.
 */

void rebuild_queue_index (void)
{
  wimp_window_state state;
  os_box            extent;
  queued_file       *list;
  int               length, visible_extent, new_extent, new_scroll;

  extern global_windows windows;


  /* Get the length of the queue. */

  length = 0;
  list = queue;

  while (list != NULL)
  {
    length++;
    list = list->next;
  }

  /* If there is already a redraw list, free it... */

  if (queue_redraw_list != NULL)
  {
    free (queue_redraw_list);
  }

  /* ... and then allocate memory to store all the entries. */

  queue_redraw_list = (queued_file **) malloc (length * sizeof(queued_file **));

  /* Populate the list. */

  list = queue;
  queue_redraw_lines = 0;

  while (list != NULL)
  {
    if (list->object_type == HELD_IN_QUEUE || list->object_type == DELETED)
    {
      queue_redraw_list[queue_redraw_lines++] = list;
    }

    list = list->next;
  }

  /* Set the window extent. */

  state.w = windows.queue_pane;
  wimp_get_window_state (&state);

  visible_extent = state.yscroll + (state.visible.y0 - state.visible.y1);

  new_extent = -QUEUE_ICON_HEIGHT * length;

  if (new_extent > (state.visible.y0 - state.visible.y1))
  {
    new_extent = state.visible.y0 - state.visible.y1;
  }

  if (new_extent > visible_extent)
  {
    /* Calculate the required new scroll offset.  If this is greater than zero, the current window is too
     * big and will need shrinking down.  Otherwise, just set the new scroll offset.
     */

    new_scroll = new_extent - (state.visible.y0 - state.visible.y1);

    if (new_scroll > 0)
    {
      state.visible.y0 += new_scroll;
      state.yscroll = 0;
    }
    else
    {
      state.yscroll = new_scroll;
    }

    wimp_open_window ((wimp_open *) &state);
  }

  extent.x0 = 0;
  extent.y1 = 0;
  extent.x1 = state.visible.x1 - state.visible.x0;
  extent.y0 = new_extent;

  wimp_set_extent (windows.queue_pane, &extent);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Re-order the linked list so that entries from the queue appear in the same order that they do in the
 * queue list window.
 */

void reorder_queue_from_index (void)
{
  queued_file **list = NULL;
  int         line;

  if(queue_redraw_lines > 0)
  {
    for (line=queue_redraw_lines - 1; line >= 0; line--)
    {
      list = &queue;

      while (*list != NULL && *list != queue_redraw_list[line])
      {
        list = &((*list)->next);
      }

      if (*list != NULL)
      {
        *list = (*list)->next;
        (queue_redraw_list[line])->next = queue;
        queue = queue_redraw_list[line];
      }
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void redraw_queue_pane (wimp_draw *redraw)
{
  int                ox, oy, top, base, y;
  osbool             more;
  wimp_icon          *icon;

  extern global_windows windows;
  extern osspriteop_area *wimp_sprites;


  /* Perform the redraw if a window was found. */

  if (redraw->w == windows.queue_pane)
  {
    more = wimp_redraw_window (redraw);

    ox = redraw->box.x0 - redraw->xscroll;
    oy = redraw->box.y1 - redraw->yscroll;

    icon = windows.queue_pane_def->icons;

    while (more)
    {
      top = (oy - redraw->clip.y1) / QUEUE_ICON_HEIGHT;
      if (top < 0)
        top = 0;

      base = (QUEUE_ICON_HEIGHT + (QUEUE_ICON_HEIGHT / 2) + oy - redraw->clip.y0) / QUEUE_ICON_HEIGHT;

      for (y = top; y < queue_redraw_lines && y <= base; y++)
      {
        icon[QUEUE_PANE_INCLUDE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
        icon[QUEUE_PANE_INCLUDE].extent.y0 = icon[QUEUE_PANE_INCLUDE].extent.y1 - QUEUE_ICON_HEIGHT;
        icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.id =
                                 (osspriteop_id) (((queue_redraw_list[y])->include) ? "opton" : "optoff");
        icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.area = (osspriteop_area *) 1;
        icon[QUEUE_PANE_INCLUDE].data.indirected_sprite.size = 12;

        wimp_plot_icon (&(icon[QUEUE_PANE_INCLUDE]));

        icon[QUEUE_PANE_FILE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
        icon[QUEUE_PANE_FILE].extent.y0 = icon[QUEUE_PANE_FILE].extent.y1 - QUEUE_ICON_HEIGHT;
        icon[QUEUE_PANE_FILE].data.indirected_text_and_sprite.text = (queue_redraw_list[y])->display_name;
        icon[QUEUE_PANE_FILE].data.indirected_text_and_sprite.size = MAX_DISPLAY_NAME;

        wimp_plot_icon (&(icon[QUEUE_PANE_FILE]));

        icon[QUEUE_PANE_DELETE].extent.y1 = -(y * QUEUE_ICON_HEIGHT);
        icon[QUEUE_PANE_DELETE].extent.y0 = icon[QUEUE_PANE_DELETE].extent.y1 - QUEUE_ICON_HEIGHT;
        icon[QUEUE_PANE_DELETE].data.indirected_sprite.id =
                  (osspriteop_id) (((queue_redraw_list[y])->object_type == DELETED) ? "del1" : "del0");
        icon[QUEUE_PANE_DELETE].data.indirected_sprite.area = wimp_sprites;
        icon[QUEUE_PANE_DELETE].data.indirected_sprite.size = 12;

        wimp_plot_icon (&(icon[QUEUE_PANE_DELETE]));
      }

      more = wimp_get_rectangle (redraw);
    }
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void queue_pane_click (wimp_pointer *pointer)
{
  int               line, column, xpos;
  wimp_window_state window;
  wimp_icon          *icon;

  extern global_windows windows;


  window.w = pointer->w;
  wimp_get_window_state (&window);

  icon = windows.queue_pane_def->icons;

  line = ((window.visible.y1 - pointer->pos.y) - window.yscroll) / QUEUE_ICON_HEIGHT;

  if (line < 0 || line >= queue_redraw_lines)
  {
    line = -1;
  }

  column = -1;

  if (line > -1)
  {
    xpos = (pointer->pos.x - window.visible.x0) + window.xscroll;

    if (icon[QUEUE_PANE_INCLUDE].extent.x0 <= xpos && icon[QUEUE_PANE_INCLUDE].extent.x1 >= xpos)
    {
      column = QUEUE_PANE_INCLUDE;
    }
    else if (icon[QUEUE_PANE_FILE].extent.x0 <= xpos && icon[QUEUE_PANE_FILE].extent.x1 >= xpos)
    {
      column = QUEUE_PANE_FILE;
    }
    else if (icon[QUEUE_PANE_DELETE].extent.x0 <= xpos && icon[QUEUE_PANE_DELETE].extent.x1 >= xpos)
    {
      column = QUEUE_PANE_DELETE;
    }
  }

  if (pointer->buttons == wimp_CLICK_SELECT && column == QUEUE_PANE_INCLUDE && line != -1)
  {
    (queue_redraw_list[line])->include = !(queue_redraw_list[line])->include;
    wimp_force_redraw (pointer->w,
                       icon[QUEUE_PANE_INCLUDE].extent.x0, -((line + 1)* QUEUE_ICON_HEIGHT),
                       icon[QUEUE_PANE_INCLUDE].extent.x1, -(line * QUEUE_ICON_HEIGHT));
  }
  else if (pointer->buttons == wimp_CLICK_SELECT && column == QUEUE_PANE_DELETE && line != -1)
  {
    if ((queue_redraw_list[line])->object_type == HELD_IN_QUEUE)
    {
      (queue_redraw_list[line])->object_type = DELETED;
    }
    else
    {
      (queue_redraw_list[line])->object_type = HELD_IN_QUEUE;
    }
    wimp_force_redraw (pointer->w,
                       icon[QUEUE_PANE_DELETE].extent.x0, -((line + 1)* QUEUE_ICON_HEIGHT),
                       icon[QUEUE_PANE_DELETE].extent.x1, -(line * QUEUE_ICON_HEIGHT));
  }
  else if (pointer->buttons == wimp_DRAG_SELECT && column == QUEUE_PANE_FILE && line != -1)
  {
    start_queue_entry_drag (line);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void start_queue_entry_drag(int line)
{
  wimp_window_state     window;
  wimp_auto_scroll_info auto_scroll;
  wimp_drag             drag;
  int                   ox, oy;

  extern global_windows windows;


  if (1)
  {
    /* Get the basic information about the window. */

    window.w = windows.queue_pane;
    wimp_get_window_state (&window);

    ox = window.visible.x0 - window.xscroll;
    oy = window.visible.y1 - window.yscroll;

    /* Set up the drag parameters. */

    drag.w = windows.queue_pane;
    drag.type = wimp_DRAG_USER_FIXED;

    drag.initial.x0 = ox;
    drag.initial.y0 = oy + -(line * QUEUE_ICON_HEIGHT + QUEUE_ICON_HEIGHT);
    drag.initial.x1 = ox + (window.visible.x1 - window.visible.x0);
    drag.initial.y1 = oy + -(line * QUEUE_ICON_HEIGHT);

    drag.bbox.x0 = window.visible.x0;
    drag.bbox.y0 = window.visible.y0;
    drag.bbox.x1 = window.visible.x1;
    drag.bbox.y1 = window.visible.y1;

    /* Read CMOS RAM to see if solid drags are required. */

    dragging_sprite = ((osbyte2 (osbyte_READ_CMOS, osbyte_CONFIGURE_DRAG_ASPRITE, 0) &
                       osbyte_CONFIGURE_DRAG_ASPRITE_MASK) != 0);

    if (0 && dragging_sprite) /* This is never used, though it could be... */
    {
      dragasprite_start (dragasprite_HPOS_CENTRE | dragasprite_VPOS_CENTRE | dragasprite_NO_BOUND |
                         dragasprite_BOUND_POINTER | dragasprite_DROP_SHADOW, wimpspriteop_AREA,
                         "", &(drag.initial), &(drag.bbox));
    }
    else
    {
      wimp_drag_box (&drag);
    }

    /* Initialise the autoscroll. */

    if (xos_swi_number_from_string ("Wimp_AutoScroll", NULL) == NULL)
    {
      auto_scroll.w = windows.queue_pane;
      auto_scroll.pause_zone_sizes.x0 = 0;
      auto_scroll.pause_zone_sizes.y0 = AUTO_SCROLL_MARGIN;
      auto_scroll.pause_zone_sizes.x1 = 0;
      auto_scroll.pause_zone_sizes.y1 = AUTO_SCROLL_MARGIN;
      auto_scroll.pause_duration = 0;
      auto_scroll.state_change = (void *) 1;

      wimp_auto_scroll (wimp_AUTO_SCROLL_ENABLE_VERTICAL, &auto_scroll);
    }

    dragging_start_line = line;
    event_set_drag_handler(terminate_queue_entry_drag, NULL, NULL);
  }
}

/**
 * Callback handler for queue window drag termination.
 *
 * \param  *drag		The Wimp poll block from termination.
 * \param  *data		NULL (unused).
 */

void terminate_queue_entry_drag(wimp_dragged *drag, void *data)
{
	wimp_pointer		pointer;
	wimp_window_state	window;
	int			line, i;
	queued_file		*moved;

	extern global_windows	windows;


	/* Terminate the drag and end the autoscroll. */

	if (xos_swi_number_from_string ("Wimp_AutoScroll", NULL) == NULL)
		wimp_auto_scroll (0, NULL);

	if (dragging_sprite)
		dragasprite_stop ();

	/* Get the line at which the drag ended. */

	wimp_get_pointer_info (&pointer);

	window.w = windows.queue_pane;
	wimp_get_window_state (&window);

	line = ((window.visible.y1 - pointer.pos.y) - window.yscroll) / QUEUE_ICON_HEIGHT;

	if (line < 0)
		line = 0;
	if (line >= queue_redraw_lines)
		line = queue_redraw_lines - 1;

	moved = queue_redraw_list[dragging_start_line];

	if (dragging_start_line < line)
		for (i=dragging_start_line; i<line; i++)
			queue_redraw_list[i] = queue_redraw_list[i+1];
	else if (dragging_start_line > line)
		for (i=dragging_start_line; i>line; i--)
			queue_redraw_list[i] = queue_redraw_list[i-1];

	queue_redraw_list[line] = moved;

	force_visible_window_redraw (windows.queue_pane);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void decode_queue_pane_help (char *buffer, wimp_w w, wimp_i i, os_coord pos, wimp_mouse_state buttons)
{
  int                 xpos, ypos, column;
  wimp_window_state   window;
  wimp_icon           *icon;
  extern global_windows windows;


  icon = windows.queue_pane_def->icons;

  *buffer = '\0';
  column = 0;

  window.w = w;
  wimp_get_window_state (&window);

  xpos = (pos.x - window.visible.x0) + window.xscroll;
  ypos = (window.visible.y1 - pos.y) - window.yscroll;

  if (ypos / QUEUE_ICON_HEIGHT < queue_redraw_lines)
  {
    if (icon[QUEUE_PANE_INCLUDE].extent.x0 <= xpos && icon[QUEUE_PANE_INCLUDE].extent.x1 >= xpos)
    {
      column = QUEUE_PANE_INCLUDE;
    }
    else if (icon[QUEUE_PANE_FILE].extent.x0 <= xpos && icon[QUEUE_PANE_FILE].extent.x1 >= xpos)
    {
      column = QUEUE_PANE_FILE;
    }
    else if (icon[QUEUE_PANE_DELETE].extent.x0 <= xpos && icon[QUEUE_PANE_DELETE].extent.x1 >= xpos)
    {
      column = QUEUE_PANE_DELETE;
    }

    sprintf (buffer, "Col%d", column);
  }
}




void traverse_queue (void)
{
  queued_file *list;
  char        status[100];

  list = queue;

  debug_printf ("\\BQueue Contents");

  while (list != NULL)
  {
    switch (list->object_type)
    {
      case PENDING_ATTENTION:
        sprintf (status, "Pending");
        break;

      case BEING_PROCESSED:
        sprintf (status, "Process");
        break;

      case HELD_IN_QUEUE:
        sprintf (status, "Held");
        break;

      case DISCARDED:
        sprintf (status, "Discard");
        break;

      case DELETED:
        sprintf (status, "Deleted");
        break;
    }

    debug_printf ("%7s %15s %s", status, list->filename, list->display_name);

    list = list->next;
  }
  debug_printf ("\\rEnd of queue");
}
