/* Copyright 2010-2012, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: bookmark.h
 *
 * Bookmark editor implementation.
 */

#ifndef PRINTPDF_BOOKMARK
#define PRINTPDF_BOOKMARK

#include <stdio.h>
#include "sflib/config.h"

/* ==================================================================================================================
 * Static constants
 */

#define MAX_BOOKMARK_LEN 64  /* The real maximum is 256, but Adobe recommend 32 max for practicality. */
#define MAX_BOOKMARK_NUM_LEN 10
#define MAX_BOOKMARK_BLOCK_NAME 64
#define MAX_BOOKMARK_FIELD_LEN 20
#define MAX_BOOKMARK_FILENAME 256
#define MAX_BOOKMARK_FILESPR 9

#define BOOKMARK_TOOLBAR_HEIGHT 82
#define BOOKMARK_LINE_HEIGHT 56
#define BOOKMARK_ICON_HEIGHT 48
#define BOOKMARK_LINE_OFFSET 4
#define BOOKMARK_TOOLBAR_OFFSET 2
#define BOOKMARK_WINDOW_MARGIN 4
#define BOOKMARK_MIN_LINES 10
#define BOOKMARK_HORIZONTAL_SCROLL 4
#define BOOKMARK_COLUMN_WIDTH 160
#define BOOKMARK_WINDOW_WIDTH 1600
#define BOOKMARK_WINDOW_STANDOFF 400
#define BOOKMARK_WINDOW_OPENSTEP 100

#define BOOKMARK_FILE_LINE_LEN (sf_MAX_CONFIG_FILE_BUFFER)

#define BOOKMARK_ABOVE 1
#define BOOKMARK_BELOW 2

/* Bookmarks Window Menu Structure. */

#define BOOKMARK_MENU_FILE   0
#define BOOKMARK_MENU_VIEW   1
#define BOOKMARK_MENU_LEVEL  2
#define BOOKMARK_MENU_INSERT 3
#define BOOKMARK_MENU_DELETE 4

#define BOOKMARK_MENU_FILE_INFO 0
#define BOOKMARK_MENU_FILE_SAVE 1

#define BOOKMARK_MENU_VIEW_EXPAND   0
#define BOOKMARK_MENU_VIEW_CONTRACT 1

#define BOOKMARK_MENU_LEVEL_PROMOTE  0
#define BOOKMARK_MENU_LEVEL_DEMOTE   1
#define BOOKMARK_MENU_LEVEL_PROMOTEG 2
#define BOOKMARK_MENU_LEVEL_DEMOTEG  3

#define BOOKMARK_MENU_INSERT_ABOVE 0
#define BOOKMARK_MENU_INSERT_BELOW 1

/* Bookmark Window icons. */

#define BOOKMARK_WINDOW_COLUMNS 3

#define BOOKMARK_ICON_EXPAND  0
#define BOOKMARK_ICON_TITLE   1
#define BOOKMARK_ICON_PAGE    2
#define BOOKMARK_ICON_YEXTENT 3

/* Bookmark Toolbar icons. */

#define BOOKMARK_TB_NAME 0

#define BOOKMARK_TB_SAVE     1
#define BOOKMARK_TB_DEMOTEG  2
#define BOOKMARK_TB_DEMOTE   3
#define BOOKMARK_TB_PROMOTE  4
#define BOOKMARK_TB_PROMOTEG 5
#define BOOKMARK_TB_EXPAND   6
#define BOOKMARK_TB_CONTRACT 7

/* Save As Dialogue icons. */

#define SAVEAS_ICON_OK     0
#define SAVEAS_ICON_CANCEL 1
#define SAVEAS_ICON_NAME   2
#define SAVEAS_ICON_FILE   3

/* File Info Dialogue icons. */

#define FILEINFO_ICON_NAME     1
#define FILEINFO_ICON_LOCATION 3
#define FILEINFO_ICON_MODIFIED 5
#define FILEINFO_ICON_DATE     7


/* ****************************************************************************
 * Data structures
 * ****************************************************************************/

typedef struct bookmark_block bookmark_block;

typedef struct bookmark_params {
	bookmark_block		*bookmarks;
} bookmark_params;



/**
 * Initialise the bookmarks system.
 */

void bookmarks_initialise(void);


/**
 * Terminate the bookmarks system, freeing up the resources used.
 */

void bookmarks_terminate(void);


/**
 * Initialise a bookmarks settings block with default parameters.
 *
 * \param  *params		The parameter block to initialise.
 */

void bookmark_initialise_settings(bookmark_params *params);


/**
 * Handle selection events from the bookmarks pop-up menu.
 *
 * \param  *params		The associated bookmarks parameters.
 * \param  *selection		The Wimp Menu selection block.
 */

void bookmark_process_menu(bookmark_params *params, wimp_selection *selection);


/**
 * Build the bookmarks pop-up menu used for selecting a bookmark set to use, and
 * register it with the global_menus structure.
 *
 * \param  *params		Bookamrks param block to use to set ticks.
 * \return			The menu block, or NULL.
 */

wimp_menu *bookmark_build_menu(bookmark_params *params);


/**
 * Load a bookmark file and set it as the current conversion file.
 *
 * \param  *params		The bookmark parameters.
 * \param  *filename		The file to load.
 * \return			TRUE if the file loaded OK; else FALSE.
 */

osbool bookmark_load_and_select_file(bookmark_params *params, char *filename);


/**
 * Fill the Bookmark info field based on the supplied parameters.
 *
 * \param  window		The window the field is in.
 * \param  icon			The icon for the field.
 * \param  *params		The parameters to use.
 */

void bookmark_fill_field(wimp_w window, wimp_i icon, bookmark_params *params);

/**
 * Indicate if the supplied bookmark parameters have data available for a
 * conversion.
 *
 * \param  *params		The parameter block to check.
 * \return			1 if data is available; else 0.
 */

int bookmark_data_available(bookmark_params *params);


/**
 * Check the status of the supplied parameter block, and update it if anything
 * is invalid.
 *
 * \param  *params		The parameter block to check.
 * \return			1 if parameters were chnaged; else 0.
 */

int bookmark_validate_params(bookmark_params *params);


/**
 * Check for any unsaved bookmark files and prompt the user if found.
 *
 * \return			TRUE if there are unsaved files to rescue; else FALSE.
 */

osbool bookmark_files_unsaved(void);


/**
 * Create and open a new bookmark window.
 *
 * \return			The address of the new block; else NULL.
 */

bookmark_block *bookmark_create_new_window(void);


/**
 * Callback to terminate a dragging filesave.
 * \TODO -- Move this into an internal message event handler.
 *
 * \param  *filename	The filename to save under.
 * \return		0 if the save completes OK; else non-0.
 */

int drag_end_save_saveas(char *filename);


/**
 * Load a bookmark file into memory, storing the data it contains in a new
 * bookmark_block structure and opening a bookmark window.
 *
 * \param  *filename	The file to load.
 * \return		The bookmark block containing the file; else NULL.
 */

bookmark_block *bookmarks_load_file(char *filename);


/**
 * Write document info to a PSDMark file, reflecting the data in the supplied
 * PDFMark parameter block.
 *
 * \param *file			The file handle to write to.
 * \param *params		The PRDMark parameter block to translate.
 */

void bookmarks_write_pdfmark_out_file(FILE *pdfmark_file, bookmark_params *params);

#endif

