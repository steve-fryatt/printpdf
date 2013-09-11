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
 * \file: ps2paper.h
 *
 * Postscript 2 paper dialogue implementation.
 */

#ifndef PRINTPDF_PS2PAPER
#define PRINTPDF_PS2PAPER


//enum paper_units {
//	PAPER_UNITS_MM = 0,
//	PAPER_UNITS_INCH = 1,
//	PAPER_UNITS_POINT = 2
//};

//typedef struct paper_params {
//	osbool			override_document;	/**< TRUE to override the document's page size.		*/
//	int			preset_size;		/**< Index into the list of Ghostscript paper sizes.	*/
//	int			width;			/**< Custom page width.					*/
//	int			height;			/**< Custom page height.				*/
//	enum paper_units	units;			/**< Units for custom page dimensions.			*/
//} paper_params;


/**
 * Initialise the ps2paper dialogue.
 */

void ps2paper_initialise(void);


/**
 * Open the paper editing window on screen.
 *
 * \param *pointer	The pointer location at which to open the window.
 */

void ps2paper_open_window(wimp_pointer *pointer);

#endif

