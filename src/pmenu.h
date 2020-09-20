/* Copyright 2007-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: pmenu.h
 *
 * Parameter Menu implementation.
 */

#ifndef PRINTPDF_PMENU
#define PRINTPDF_PMENU


/**
 * Return an entry from a comma-separated list of parameters in a message
 * token.
 *
 * \param *buffer		Pointer to a buffer to take the entry.
 * \param len			The size of the buffer.
 * \param *param_list		The list message token.
 * \param entry			The number of the item to return.
 * \return			Pointer to the \0 terminator in the buffer.
 */

char *pmenu_list_entry(char *buffer, size_t len, char* param_list, int entry);

#endif

