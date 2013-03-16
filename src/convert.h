/* Copyright 2005-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: convert.h
 *
 * Conversion queue implementation.
 */

#ifndef PRINTPDF_CONVERT
#define PRINTPDF_CONVERT

/* ==================================================================================================================
 * Static constants
 */

#define AUTO_SCROLL_MARGIN 100

/* Save PDF Window icons. */

#define SAVE_PDF_ICON_OK             0
#define SAVE_PDF_ICON_CANCEL         1
#define SAVE_PDF_ICON_NAME           2
#define SAVE_PDF_ICON_FILE           3
#define SAVE_PDF_ICON_VERSION_MENU   4
#define SAVE_PDF_ICON_VERSION_FIELD  5
#define SAVE_PDF_ICON_OPT_MENU       7
#define SAVE_PDF_ICON_OPT_FIELD      8
#define SAVE_PDF_ICON_PREPROCESS     10
#define SAVE_PDF_ICON_ENCRYPT_MENU   11
#define SAVE_PDF_ICON_ENCRYPT_FIELD  12
#define SAVE_PDF_ICON_QUEUE          14
#define SAVE_PDF_ICON_PDFMARK_MENU   15
#define SAVE_PDF_ICON_PDFMARK_FIELD  16
#define SAVE_PDF_ICON_USERFILE       18
#define SAVE_PDF_ICON_BOOKMARK_MENU  20
#define SAVE_PDF_ICON_BOOKMARK_FIELD 21



/**
 * Test for the presence of the queue directory, and create it if it is not already there.
 *
 * At present, this assumes that it will be just the leaf directory missing; true assuming an address in the Scrap
 * folder.
 *
 * Also set up the conversion parameters for the first time, for each of the modules.
 */

void convert_initialise(void);


/**
 * Check the location of the 'print file' to see if one has appeared.  If it
 * has, add it to the file queue.
 *
 * Called from NULL poll events.
 */

void convert_check_for_ps_file(void);


/**
 * Take the file specified, copy it with a timestamp and add it to the queue of files.
 *
 * \param *filename		The file to copy.
 * \return			TRUE if successful; else FALSE.
 */

osbool convert_queue_ps_file(char *filename);


/**
 * Test to see if there is a file queued and no conversion taking place.  If these are both true, select the next
 * pending file in the queue and open the Save PDF dialogue.
 *
 * Called from NULL poll events.
 */

void convert_check_for_pending_files(void);


/**
 * Callback handler for DataSave completion on PDF save drags: start the
 * conversion using the filename returned.
 *
 * \param *filename		The filename returned by the DataSave protocol.
 * \return			0 if the save was started OK.
 */

int drag_end_save_pdf(char *filename);


/**
 * Handle the closure of the file save dialogue following a successful data xfer protocol or similar.
 * The settings are retrieved, and a conversion process is started.
 *
 * \param *output_file		The file to save the PDF as.
 */

void convert_save_dialogue_end(char *filename);


/**
 * Called by modules to ask the converion system to re-validate its parameters.
 */

void convert_validate_params(void);


/**
 * Remove all the items from the queue, and delete their files.
 */

void convert_remove_all_remaining_conversions(void);


/**
 * Return an indication that a conversion is underway.
 *
 * \return		TRUE if a conversion is in progress; else FALSE.
 */

osbool convert_pdf_conversion_in_progress(void);


/**
 * Return an indication of whether any queued files are left.
 *
 * \return			TRUE if there are files to be saved; else FALSE.
 */

osbool convert_pending_files_in_queue(void);


/**
 * Open the queue dialogue at the pointer.
 *
 * \param *pointer		The wimp pointer data.
 */

void convert_open_queue_window(wimp_pointer *pointer);

#endif

