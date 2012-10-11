/* Copyright 2005-2012, Stephen Fryatt
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
 * \file: ihelp.h
 *
 * Interactive help implementation.
 */

#ifndef PRINTPDF_IHELP
#define PRINTPDF_IHELP


#define IHELP_LENGTH 236
#define IHELP_INAME_LEN 64


/**
 * Initialise the interactive help system.
 */

void ihelp_initialise(void);


/**
 * Add a new interactive help window definition.
 *
 * \param window		The window handle to attach help to.
 * \param *name			The token name to associate with the window.
 * \param *decode		A function to use to help decode clicks in the window.
 */

void ihelp_add_window(wimp_w window, char* name, void (*decode) (char *, wimp_w, wimp_i, os_coord, wimp_mouse_state));

/**
 * Remove an interactive help definition from the window list.
 *
 * \param window		The window handle to remove from the list.
 */

void ihelp_remove_window(wimp_w window);

#endif

