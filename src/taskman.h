/* PrintPDF - taskman.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef _PRINTPDF_TASKMAN
#define _PRINTPDF_TASKMAN

/* ==================================================================================================================
 * Static constants
 */


/* ==================================================================================================================
 * Data structures
 */

/* ==================================================================================================================
 * Function prototypes.
 */

/* Test new tasks to see if they are copies of PrintPDF. */

void check_new_task (wimp_message *message);

/* Check if a named task is running. */

int task_is_running (char *task_name, wimp_t ignore_task);

#endif
