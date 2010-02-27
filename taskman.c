/* PrintPDF - taskman.c
 *
 * (C) Stephen Fryatt, 2005
 */

/* ANSI C header files */

#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/taskmanager.h"
#include "oslib/wimp.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "sflib/msgs.h"

/* Application header files */

#include "taskman.h"

#include "choices.h"

/* ==================================================================================================================
 * Global variables.
 */

/* ==================================================================================================================
 * Test new tasks to see if they are copies of PrintPDF.
 */

void check_new_task (wimp_message *message)
{
  wimp_full_message_task_initialise *task_init = (wimp_full_message_task_initialise *) message;
  wimp_pointer                      pointer;
  char                              task_name[256];

  extern wimp_t                     task_handle;


  msgs_lookup ("TaskName", task_name, sizeof (task_name));
  if (task_init->sender != task_handle && strcmp (task_name, task_init->task_name) == 0)
  {
    wimp_get_pointer_info (&pointer);
    open_choices_window (&pointer);
  }
}

/* ==================================================================================================================
 * Check if a named task is running.
 */

/* Check to see if a task with the given name is running.  The specified task handle can be ignored.
 */

int task_is_running (char *task_name, wimp_t ignore_task)
{
  taskmanager_task task_data;
  int              next, found;
  char             *end;


  next = 0;
  found = 0;

  while (next >= 0)
  {
    next = taskmanager_enumerate_tasks (next, &task_data, sizeof (taskmanager_task), &end);

    if (end > (char *) &task_data)
    {
      if (strcmp (task_data.name, task_name) == 0 && task_data.task != ignore_task)
      {
        found = 1;
      }
    }
  }

  return (found);
}
