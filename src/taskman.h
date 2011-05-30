/* PrintPDF - taskman.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef PRINTPDF_TASKMAN
#define PRINTPDF_TASKMAN

/* ==================================================================================================================
 * Static constants
 */


/* ==================================================================================================================
 * Data structures
 */

/* ==================================================================================================================
 * Function prototypes.
 */

/**
 * Initialise the Task Manager module.
 */

void taskman_initialise(void);

/**
 * Check if a named task is running.
 *
 * \param *task_name		The name to test against.
 * \param ignore_task		A task handle to ignore, even if the name matches.
 * \return			TRUE if a match was found; else FALSE.
 */

osbool taskman_task_is_running(char *task_name, wimp_t ignore_task);

#endif
