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
	CONTROL_REPORT_FAILURE = 2,		/**< Conversion failed.			*/
	CONTROL_CLEAR_FILENAME = 3		/**< Clear an already set filename.	*/
};

#define CONTROL_COMMAND_BASIC_LENGTH 24
#define CONTROL_COMMAND_FAILURE_LENGTH 28

typedef struct {
	wimp_MESSAGE_HEADER_MEMBERS
	enum control_reason	reason;
	union {
		char			filename[232];
		enum api_failure	failure;
	};
} control_message;

/* Function Prototypes. */

static osbool api_message_task_close_down_handler(wimp_message *message);
static osbool api_message_printpdf_control_handler(wimp_message *message);
static void api_send_conversion_failure(wimp_t task, int your_ref, enum api_failure failure);


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
		if (api_request_task == NULL) {
			if (*(control->filename) != '\0') {
				api_request_task = control->sender;
				api_request_ref = control->my_ref;
				string_copy(api_request_filename, control->filename, CONVERT_MAX_FILENAME);

				control->your_ref = control->my_ref;
				wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message, message->sender);

			} else {
				api_send_conversion_failure(message->sender, message->my_ref, API_FAILURE_NULL_FILENAME);
			}
		} else {
			api_send_conversion_failure(message->sender, message->my_ref, API_FAILURE_IN_USE);
		}

		return TRUE;

	case CONTROL_CLEAR_FILENAME:
		if (api_request_task == NULL || api_request_task == control->sender) {
			api_request_task = NULL;
			api_request_ref = 0;

			control->your_ref = control->my_ref;
			wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message, message->sender);
		} else {
			api_send_conversion_failure(message->sender, message->my_ref, API_FAILURE_NOT_OWNER);
		}

		return TRUE;
	}

	return FALSE;
}


/**
 * On a successful completion of a conversion, check if the API is in
 * use. If it is, notify the client task of the result.
 * 
 * \param success	TRUE if the conversion was successful;
 *			else FALSE.
 */

void api_notify_conversion_success(void)
{
	if (api_request_task == NULL)
		return;

	control_message control;

	control.your_ref = api_request_ref;
	control.action = message_PRINTPDF_CONTROL;
	control.reason = CONTROL_REPORT_SUCCESS;
	control.size = CONTROL_COMMAND_BASIC_LENGTH;

	wimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &control, api_request_task);
	api_request_task = NULL;
	api_request_ref = 0;
}


/**
 * On an unsuccessful completion of a conversion, check if the API is in
 * use. If it is, notify the client task of the result.
 * 
 * \param failure	The reason for failure to pass to the client.
 */

void api_notify_conversion_failure(enum api_failure failure)
{
	if (api_request_task == NULL)
		return;

	api_send_conversion_failure(api_request_task, api_request_ref, failure);

	api_request_task = NULL;
	api_request_ref = 0;
}


/**
 * Send a conversion failure message to a client application
 *
 * \param task		The task handle of the client.
 * \param your_ref	The YourRef value for the message to be sent.
 * \param failure	The reason for failure to pass to the client.
 */

static void api_send_conversion_failure(wimp_t task, int your_ref, enum api_failure failure)
{
	control_message control;

	control.your_ref = your_ref;
	control.action = message_PRINTPDF_CONTROL;
	control.reason = CONTROL_REPORT_FAILURE;
	control.failure = failure;
	control.size = CONTROL_COMMAND_FAILURE_LENGTH;

	wimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &control, task);
}
