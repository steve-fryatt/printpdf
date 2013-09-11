/* Copyright 2013, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file: ps2paper.c
 *
 * Postscript 2 paper dialogue implementation.
 */

/* ANSI C header files */

#include <string.h>

/* Acorn C header files */

/* OSLib header files */

#include "oslib/wimp.h"

/* SF-Lib header files. */

#include "sflib/config.h"
#include "sflib/debug.h"
#include "sflib/event.h"
#include "sflib/icons.h"
#include "sflib/menus.h"
#include "sflib/msgs.h"
#include "sflib/string.h"
#include "sflib/windows.h"

/* Application header files */

#include "ps2paper.h"

#include "ihelp.h"
#include "templates.h"

#define PS2PAPER_NAME_LEN 128

/* Paper Window icons. */

//#define PAPER_ICON_CANCEL 0
//#define PAPER_ICON_OK 1
//#define PAPER_ICON_SIZE_X 3
//#define PAPER_ICON_SIZE_Y 4
//#define PAPER_ICON_MM 5
//#define PAPER_ICON_INCH 6
//#define PAPER_ICON_POINT 7


struct ps2paper_size {
	char			name[PS2PAPER_NAME_LEN];/**< The Printers name for the paper		*/
	int			width;			/**< The Printers width of the paper		*/
	int			height;			/**< The Printers height of the paper		*/
	
	struct ps2paper_size	*next;			/**< Link to the next paper size.		*/
};

static struct ps2paper_size	*paper_sizes = NULL;	/**< Linked list of paper sizes.		*/
static unsigned			paper_size_count;	/**< Number of defined paper sizes.		*/

static osbool ps2paper_read_def_file(char *file);

/**
 * Initialise the ps2paper dialogue.
 */

void ps2paper_initialise(void)
{
	//paper_window = templates_create_window("Paper");
	//ihelp_add_window(paper_window, "Paper", NULL);

	//event_add_window_mouse_event(paper_window, paper_click_handler);
	//event_add_window_key_event(paper_window, paper_keypress_handler);
	
	//event_add_window_icon_radio(paper_window, PAPER_ICON_MM, TRUE);
	//event_add_window_icon_radio(paper_window, PAPER_ICON_INCH, TRUE);
	//event_add_window_icon_radio(paper_window, PAPER_ICON_POINT, TRUE);
}


/**
 * Open the paper editing window on screen.
 *
 * \param *pointer	The pointer location at which to open the window.
 */

void ps2paper_open_window(wimp_pointer *pointer)
{
	struct ps2paper_size	*paper;

	debug_printf("\\YPrinters:PaperRO");
	ps2paper_read_def_file("Printers:PaperRO");
	debug_printf("\\YPrinterChoices:PaperRW");
	ps2paper_read_def_file("PrinterChoices:PaperRW");
	debug_printf("\\YPrinters:ps.Resources.PaperRO");
	ps2paper_read_def_file("Printers:ps.Resources.PaperRO");
	
	paper = paper_sizes;
	
	while (paper != NULL) {
		debug_printf("Paper '%s', width %u, height %u", paper->name, paper->width, paper->height);
		
		paper = paper->next;
	}

}


/**
 * Process the contents of a Printers paper file, reading the paper definitions
 * and adding them to the list of sizes.
 *
 * \param *file		The name of the file to be read in.
 */

static osbool ps2paper_read_def_file(char *file)
{
	FILE			*in;
	char			line[1024], *clean, *data, paper_name[PS2PAPER_NAME_LEN];
	unsigned		paper_width, paper_height;
	struct ps2paper_size	*paper_definition;


	if (file == NULL || *file == '\0')
		return FALSE;

	in = fopen(file, "r");

	if (in == NULL)
		return FALSE;
	
	*paper_name = '\0';
	paper_width = 0;
	paper_height = 0;

	while (fgets(line, sizeof(line), in) != NULL) {
		string_ctrl_zero_terminate(line);
		clean = string_strip_surrounding_whitespace(line);
	
		if (*clean == '\0' || *clean == '#')
			continue;
		
		if (strstr(clean, "pn:") == clean) {
			data = string_strip_surrounding_whitespace(clean + 3);
			strncpy(paper_name, data, PS2PAPER_NAME_LEN);
		} else if (strstr(clean, "pw:") == clean) {
			data = string_strip_surrounding_whitespace(clean + 3);
			paper_width = atoi(data);
		} else if (strstr(clean, "ph:") == clean) {
			data = string_strip_surrounding_whitespace(clean + 3);
			paper_height = atoi(data);
		}
		
		if (paper_name != '\0' && paper_width != 0 && paper_height != 0) {
			paper_definition = malloc(sizeof(struct ps2paper_size));
			
			if (paper_definition != NULL) {
				strncpy(paper_definition->name, paper_name, PS2PAPER_NAME_LEN);
				paper_definition->width = paper_width;
				paper_definition->height = paper_height;
				
				paper_definition->next = paper_sizes;
				paper_sizes = paper_definition;
			}

			*paper_name = '\0';
			paper_width = 0;
			paper_height = 0;
		}
	}

	fclose(in);

	return TRUE;
}

