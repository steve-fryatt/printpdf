/* PrintPDF - taskman.c
 *
 * (c) Stephen Fryatt, 2005-2011
 */

/* ANSI C header files */

#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/taskmanager.h"
#include "oslib/wimp.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "sflib/event.h"
#include "sflib/msgs.h"

/* Application header files */

#include "taskman.h"

#include "choices.h"
#include "main.h"


static osbool taskman_check_new_task(wimp_message *message);


/**
 * Initialise the Task Manager module.
 */

void taskman_initialise(void)
{
	event_add_message_handler(message_TASK_INITIALISE, EVENT_MESSAGE_INCOMING, taskman_check_new_task);
}


/**
 * Check Message_TaskInitialise to see if the new tasks are PrintPDF.  If they are, respond with
 * a suitable action on the assumption that they will themselves just quit silently.
 *
 * \param *message		The message data block.
 * \return			FALSE so that other claimants can see the message.
 */

static osbool taskman_check_new_task(wimp_message *message)
{
	wimp_full_message_task_initialise *task_init = (wimp_full_message_task_initialise *) message;
	wimp_pointer                      pointer;
	char                              task_name[256];


	msgs_lookup("TaskName", task_name, sizeof(task_name));
	if (task_init->sender != main_task_handle && strcmp(task_name, task_init->task_name) == 0) {
		wimp_get_pointer_info(&pointer);
		open_choices_window(&pointer);
	}

	return FALSE;
}


/**
 * Check if a named task is running.
 *
 * \param *task_name		The name to test against.
 * \param ignore_task		A task handle to ignore, even if the name matches.
 * \return			TRUE if a match was found; else FALSE.
 */

osbool taskman_task_is_running(char *task_name, wimp_t ignore_task)
{
	taskmanager_task	task_data;
	int			next;
	osbool			found;
	char			*end;


	next = 0;
	found = FALSE;

	while (next >= 0 && !found) {
		next = taskmanager_enumerate_tasks(next, &task_data, sizeof(taskmanager_task), &end);

		if (end > (char *) &task_data && strcmp(task_data.name, task_name) == 0 && task_data.task != ignore_task)
			found = TRUE;
	}

	return found;
}

