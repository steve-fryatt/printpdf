/* Copyright 2007-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: choices.h
 *
 * Choices dialogue implementation.
 */

#ifndef PRINTPDF_CHOICES
#define PRINTPDF_CHOICES


/**
 * Initialise the Choices module.
 */

void choices_initialise(void);


/**
 * Open the Choices window at the mouse pointer.
 *
 * \param *pointer		The details of the pointer to open the window at.
 */

void choices_open_window(wimp_pointer *pointer);


/**
 * Identify whether the Choices window is currently open.
 *
 * \return		TRUE if the window is open; else FALSE.
 */

osbool choices_window_is_open(void);

#endif

