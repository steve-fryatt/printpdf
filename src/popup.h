/* Copyright 2005-2012, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of PrintPDF:
 *
 *   http://www.stevefryatt.org.uk/software/
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
 * \file: popup.h
 *
 * Pop-Up Menu implementation.
 */

#ifndef PRINTPDF_POPUP
#define PRINTPDF_POPUP

/**
 * Initialise the popup window module.
 */

void popup_initialise(void);


/**
 * Test the time given against the time to close the popup, and if that time has passed, close the window.
 *
 * \param current		The current OS Monotonic Time.
 */

void popup_test_and_close(os_t current);


/**
 * Open the popup for a minimum of the given time (this may be longer, as checks are only made at the externally
 * selected poll interval).
 *
 * \param open_time	The time to open the window for.
 */

void popup_open(int open_time);

#endif

