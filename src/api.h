/* Copyright 2022-2023, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: api.h
 *
 * Conversion queue implementation.
 */

#ifndef PRINTPDF_API
#define PRINTPDF_API

#include "oslib/types.h"

/**
 * The reasons which can be returned by Control Failure.
 */

enum api_failure {
	API_FAILURE_IN_USE = 0,		/**< There API is already in use.	*/
	API_FAILURE_NULL_FILENAME = 1,	/**< The supplied filename was empty.	*/
	API_FAILURE_CONVERSION = 2		/**< The conversion failed.		*/
};

/**
 * Initialise the control API.
 */

void api_initialise(void);


/**
 * Test for a filename from the control API.
 * 
 * \return		A pointer to the filename, or NULL if none has
 *			been configured.
 */

char *api_get_filename(void);


/**
 * On a successful completion of a conversion, check if the API is in
 * use. If it is, notify the client task of the result.
 * 
 * \param success	TRUE if the conversion was successful;
 *			else FALSE.
 */

void api_notify_conversion_success(void);


/**
 * On an unsuccessful completion of a conversion, check if the API is in
 * use. If it is, notify the client task of the result.
 * 
 * \param failure	The reason for failure to pass to the client.
 */

void api_notify_conversion_failure(enum api_failure failure);

#endif
