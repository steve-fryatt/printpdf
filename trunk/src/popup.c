/* Copyright 2005-2016, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/software/
 *
 * Licensed under the EUPL, Version 1.1 only (the "Licence");
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
 * \file: popup.c
 *
 * Pop-Up Menu implementation.
 */

/* ANSI C header files */

/* Acorn C header files */

/* OSLib header files */

#include "oslib/os.h"
#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/event.h"
#include "sflib/ihelp.h"
#include "sflib/templates.h"
#include "sflib/windows.h"

/* Application header files */

#include "popup.h"


static int	popup_is_open = FALSE;
static os_t	popup_close_time = 0;
static wimp_w	popup_window = NULL;


static void	popup_click_handler(wimp_pointer *pointer);
static void	popup_close(void);


/**
 * Initialise the popup window module.
 */

void popup_initialise(void)
{
	popup_window = templates_create_window("PopUp");
	ihelp_add_window(popup_window, "PopUp", NULL);
	event_add_window_mouse_event(popup_window, popup_click_handler);
}


/**
 * Process clicks in the popup window.
 *
 * \param *pointer		The relevant wimp pointer event block.
 */

static void popup_click_handler(wimp_pointer *pointer)
{
	if (pointer!= NULL && pointer->buttons == wimp_CLICK_SELECT)
		popup_close();
}


/**
 * Test the time given against the time to close the popup, and if that time has passed, close the window.
 *
 * \param current		The current OS Monotonic Time.
 */

void popup_test_and_close(os_t current)
{
	if (popup_is_open == FALSE)
		return;

	if (current >= popup_close_time)
		popup_close();
}


/**
 * Close the popup window.
 */

static void popup_close(void)
{
	if (popup_is_open == FALSE)
		return;

	wimp_close_window(popup_window);

	popup_close_time = 0;
	popup_is_open = FALSE;
}


/**
 * Open the popup for a minimum of the given time (this may be longer, as checks are only made at the externally
 * selected poll interval).
 *
 * \param open_time	The time to open the window for.
 */

void popup_open(int open_time)
{
	popup_is_open = TRUE;
	popup_close_time = os_read_monotonic_time() + open_time;

	windows_open_centred_on_screen(popup_window);
}
