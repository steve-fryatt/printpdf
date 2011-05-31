/* PrintPDF - dataxfer.h
 *
 * (c) Stephen Fryatt, 2005-2011
 */

#ifndef PRINTPDF_DATAXFER
#define PRINTPDF_DATAXFER

/* ==================================================================================================================
 * Static constants
 */


#define PRINTPDF_FILE_TYPE 0x1d8
#define PDF_FILE_TYPE 0xadf
#define PS_FILE_TYPE 0xff5

#define DRAG_SAVE_PDF    1
#define DRAG_SAVE_SAVEAS 2


/**
 * Initialise the data transfer system.
 */

void dataxfer_initialise(void);


/**
 * Start dragging the icon from the save dialogue.  Called in response to an attempt to drag the icon.
 *
 * \param type		The drag type to start.
 */

void start_save_window_drag(int type);


/**
 * Try to save in response to a click on 'OK' in the Save dialogue.
 *
 * \return 		0 if the process completed OK.
 */

int immediate_window_save(void);

#endif

