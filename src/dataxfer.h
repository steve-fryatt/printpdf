/* PrintPDF - dataxfer.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef _PRINTPDF_DATAXFER
#define _PRINTPDF_DATAXFER

/* ==================================================================================================================
 * Static constants
 */


#define PRINTPDF_FILE_TYPE 0x1d8
#define PDF_FILE_TYPE 0xadf
#define PS_FILE_TYPE 0xff5

#define DRAG_SAVE_PDF    1
#define DRAG_SAVE_SAVEAS 2

/* ==================================================================================================================
 * Data structures
 */

/* ==================================================================================================================
 * Function prototypes.
 */

/* Save box drag handling */

void start_save_window_drag(int type);

/* Save Box direct save. */

int immediate_window_save (void);

/* 'File load' handling. */

void message_data_save_reply (wimp_message *message);
void message_data_load_reply (wimp_message *message);

int start_data_open_load(wimp_message *message);

#endif
