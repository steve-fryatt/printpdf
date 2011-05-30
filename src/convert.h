/* PrintPDF - convert.h
 *
 * (c) Stephen Fryatt, 2007
 */

#ifndef _PRINTPDF_CONVERT
#define _PRINTPDF_CONVERT

/* ==================================================================================================================
 * Static constants
 */

#define MAX_QUEUE_NAME 32
#define MAX_FILENAME 512
#define MAX_DISPLAY_NAME 64

#define QUEUE_ICON_HEIGHT 48
#define AUTO_SCROLL_MARGIN 100

/* Conversion progress stages. */

#define CONVERSION_STOPPED     0
#define CONVERSION_STARTING    1
#define CONVERSION_PS2PS       2
#define CONVERSION_PS2PDF      3

/* Save PDF Window icons. */

#define SAVE_PDF_ICON_OK             0
#define SAVE_PDF_ICON_CANCEL         1
#define SAVE_PDF_ICON_NAME           2
#define SAVE_PDF_ICON_FILE           3
#define SAVE_PDF_ICON_VERSION_MENU   4
#define SAVE_PDF_ICON_VERSION_FIELD  5
#define SAVE_PDF_ICON_OPT_MENU       7
#define SAVE_PDF_ICON_OPT_FIELD      8
#define SAVE_PDF_ICON_PREPROCESS     10
#define SAVE_PDF_ICON_ENCRYPT_MENU   11
#define SAVE_PDF_ICON_ENCRYPT_FIELD  12
#define SAVE_PDF_ICON_QUEUE          14
#define SAVE_PDF_ICON_PDFMARK_MENU   15
#define SAVE_PDF_ICON_PDFMARK_FIELD  16
#define SAVE_PDF_ICON_USERFILE       18
#define SAVE_PDF_ICON_BOOKMARK_MENU  20
#define SAVE_PDF_ICON_BOOKMARK_FIELD 21

/* Defer queue window icons. */

#define QUEUE_ICON_PANE   0
#define QUEUE_ICON_CLOSE  1
#define QUEUE_ICON_CREATE 2

#define QUEUE_PANE_INCLUDE 0
#define QUEUE_PANE_FILE    1
#define QUEUE_PANE_DELETE  2

/* Queue entry types. */

#define PENDING_ATTENTION 1
#define BEING_PROCESSED   2
#define HELD_IN_QUEUE     3
#define DISCARDED         4
#define DELETED           5

/* ==================================================================================================================
 * Data structures
 */

typedef struct queued_file
{
  char               filename[MAX_QUEUE_NAME];
  char               display_name[MAX_DISPLAY_NAME];
  int                object_type;
  int                include;

  struct queued_file *next;
}
queued_file;

typedef struct conversion_params
{
  char  input_filename[MAX_FILENAME];
  char  output_filename[MAX_FILENAME];
  char  pdfmark_userfile[MAX_FILENAME];

  int   preprocess_in_ps2ps;
}
conversion_params;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Initialisation. */

void initialise_conversion (void);

/* Queueing files */

void check_for_ps_file (void);
int queue_ps_file (char *filename);

/* Starting conversions */

void check_for_pending_files (void);
void start_held_conversion (void);

/* Handling Save PDF dialogue */

void open_conversion_dialogue (void);
void conversion_dialogue_end (char *filename);
void conversion_dialogue_queue (void);
void handle_save_icon_drop (wimp_full_message_data_xfer *dataload);

/* Ghostscript conversion progress */

int conversion_progress (conversion_params *params);
int launch_ps2ps (char *file_out);
int launch_ps2pdf (char *file_out, char *pdfmark_file);
void cancel_conversion (void);

/* Handling parameters. */

void convert_validate_params(void);

/* Handling pop-up menus from the dialogue. */

void open_convert_version_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_convert_version_menu (wimp_selection *selection);
void open_convert_optimize_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_convert_optimize_menu (wimp_selection *selection);
void open_convert_bookmark_menu(wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_convert_bookmark_menu(wimp_selection *selection);

/* Handle the encryption and optimization windows. */

void open_convert_pdfmark_dialogue (wimp_pointer *pointer);
void process_convert_pdfmark_dialogue (void);
void open_convert_encrypt_dialogue (wimp_pointer *pointer);
void process_convert_encrypt_dialogue (void);
void process_convert_optimize_dialogue (void);

/* Dequeueing files. */

void remove_deleted_files (void);
void remove_current_conversion (void);
void remove_first_conversion (void);
void remove_all_remaining_conversions (void);

/* External interfaces */

int pdf_conversion_in_progress (void);
int pending_files_in_queue(void);

/* Defer queue manipulation. */

void open_queue_window (wimp_pointer *pointer);
void close_queue_window (void);
void rebuild_queue_index (void);
void reorder_queue_from_index (void);
void start_queue_entry_drag (int line);
void decode_queue_pane_help (char *buffer, wimp_w w, wimp_i i, os_coord pos, wimp_mouse_state buttons);

void traverse_queue (void);

#endif
