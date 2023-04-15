/* Copyright 2005-2012, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/risc-os/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 *
 * You may obtain a copy of the Licence at:
 *
 *   http://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

/**
 * \file: taskman.h
 *
 * Task Manager support.
 */

#ifndef PRINTPDF_TASKMAN
#define PRINTPDF_TASKMAN

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

