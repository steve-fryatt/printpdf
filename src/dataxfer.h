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
 * \param w		The window where the drag is starting.
 * \param i		The icon to be dragged.
 * \param *filename	The filename to be used as a starting point.
 */

void start_save_window_drag(int type, wimp_w w, wimp_i i, char *filename);

#endif

