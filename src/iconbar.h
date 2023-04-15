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
 * \file: iconbar.h
 *
 * IconBar icon and menu implementation.
 */

#ifndef PRINTPDF_ICONBAR
#define PRINTPDF_ICONBAR


/**
 * Initialise the iconbar icon and its associated menus and dialogues.
 */

void iconbar_initialise(void);


/**
 * Create or delete the icon on the iconbar, as appropriate.
 *
 * \param new_state		TRUE to create an icon; FALSE to remove it.
 */

void iconbar_set_icon(osbool new_state);

#endif

