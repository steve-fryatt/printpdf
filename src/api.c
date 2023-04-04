/* Copyright 2023, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: api.c
 *
 * External control API implementation.
 */

/* ANSI C header files */

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/debug.h"
#include "sflib/event.h"
#include "sflib/string.h"

/* Application header files */

#include "api.h"

#include "convert.h"


/* Message for remote control operation. */

#define message_PRINTPDF_CONTROL	0x5A480	/**< Wimp Message number */

enum control_reason {
	CONTROL_SET_FILENAME = 0,		/**< Set up filename to save to.	*/
	CONTROL_REPORT_SUCCESS = 1,		/**< Conversion completed correctly.	*/
	CONTROL_REPORT_FAILURE = 2		/**< Conversion failed.			*/
};

#define CONTROL_COMMAND_BASIC_LENGTH 24

typedef struct {
	wimp_MESSAGE_HEADER_MEMBERS
	enum control_reason	reason;
	char			filename[232];
} control_message;

/* Function Prototypes. */

static osbool api_message_task_close_down_handler(wimp_message *message);
static osbool api_message_printpdf_control_handler(wimp_message *message);


/**
 * The handle of the task which is currently in control of the API.
 */
static wimp_t api_request_task = NULL;

/**
 * The MyRef from the most recent API request message.
*/

static int api_request_ref = 0;

/**
 * The filename supplied through the API.
 */

static char api_request_filename[CONVERT_MAX_FILENAME];


/**
 * Initialise the control API.
 */

void api_initialise(void)
{
	event_add_message_handler(message_TASK_CLOSE_DOWN, EVENT_MESSAGE_INCOMING, api_message_task_close_down_handler);
	event_add_message_handler(message_PRINTPDF_CONTROL, EVENT_MESSAGE_INCOMING, api_message_printpdf_control_handler);
}


/**
 * Test for a filename from the control API.
 * 
 * \return		A pointer to the filename, or NULL if none has
 *			been configured.
 */

char *api_get_filename(void)
{
	if (api_request_task == NULL)
		return NULL;

	return api_request_filename;
}


/**
 * On completion of a conveapi_notify_conversion_completersion, check if the API is in use. If it
 * is, notify the client task of the result.
 * 
 * \param success	TRUE if the conversion was successful;
 *			else FALSE.
 */

void api_notify_conversion_complete(osbool success)
{
	if (api_request_task == NULL)
		return;

	control_message control;

	control.your_ref = api_request_ref;
	control.action = message_PRINTPDF_CONTROL;
	control.reason = success ? CONTROL_REPORT_SUCCESS : CONTROL_REPORT_FAILURE;
	control.size = CONTROL_COMMAND_BASIC_LENGTH;

	wimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &control, api_request_task);
	api_request_task = NULL;
	api_request_ref = 0;
}


/**
 * Process Message_TaskCloseDown, in case the current API client has
 * quit.
 *
 * \param *message	The message data block.
 * \return		FALSE to allow other claimants to see the message.
 */

static osbool api_message_task_close_down_handler(wimp_message *message)
{
	if (message != NULL && api_request_task != NULL && message->sender == api_request_task) {
		api_request_task = NULL;
		api_request_ref = 0;
	}

	return FALSE;
}


/**
 * Process Message_PrintPDFControl, which is sent by tasks wishing to
 * control the conversion process.
 *
 * \param *message	The message data block.
 * \return		FALSE to allow other claimants to see the message.
 */

static osbool api_message_printpdf_control_handler(wimp_message *message)
{
	control_message *control = (control_message *) message;

	if (control == NULL)
		return FALSE;
	
	switch (control->reason) {
	case CONTROL_SET_FILENAME:
//FIXME: Would need to check for null filename. What if conversion in progress?
		if (api_request_task == NULL) {
			api_request_task = control->sender;
			api_request_ref = control->my_ref;
			string_copy(api_request_filename, control->filename, CONVERT_MAX_FILENAME);

			control->reason = CONTROL_REPORT_SUCCESS;
			control->size = CONTROL_COMMAND_BASIC_LENGTH;
		} else {
			control->reason = CONTROL_REPORT_FAILURE;
			control->size = CONTROL_COMMAND_BASIC_LENGTH;
		}

		message->your_ref = message->my_ref;
		wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

		return TRUE;
	}

	return FALSE;
}
