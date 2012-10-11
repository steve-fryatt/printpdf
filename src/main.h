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
 * \file: main.h
 *
 * Application core and initialisation.
 */

#ifndef PRINTPDF_MAIN
#define PRINTPDF_MAIN


#define ENCRYPTION_SAVE 1
#define ENCRYPTION_CHOICE 2

#define OPTIMIZATION_SAVE 1
#define OPTIMIZATION_CHOICE 2

#define PDFMARK_SAVE 1
#define PDFMARK_CHOICE 2

#define BOOKMARK_SAVE 1
#define BOOKMARK_CHOICE 2

#define DRAG_SAVE  1
#define DRAG_QUEUE 2


/**
 * Application-wide global variables.
 */

extern wimp_t			main_task_handle;
extern int			main_quit_flag;
extern osspriteop_area		*main_wimp_sprites;

/**
 * Main code entry point.
 */

int main(int argc, char *argv[]);

#endif

