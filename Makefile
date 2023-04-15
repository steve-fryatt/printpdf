# Copyright 2010-2016, Stephen Fryatt (info@stevefryatt.org.uk)
#
# This file is part of PrintPDF:
#
#   http://www.stevefryatt.org.uk/software/
#
# Licensed under the EUPL, Version 1.2 only (the "Licence");
# You may not use this work except in compliance with the
# Licence.
#
# You may obtain a copy of the Licence at:
#
#   http://joinup.ec.europa.eu/software/page/eupl
#
# Unless required by applicable law or agreed to in
# writing, software distributed under the Licence is
# distributed on an "AS IS" basis, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#
# See the Licence for the specific language governing
# permissions and limitations under the Licence.

# This file really needs to be run by GNUMake.
# It is intended for native compilation on Linux (for use in a GCCSDK
# environment) or cross-compilation under the GCCSDK.

ARCHIVE := printpdf

APP := !PrintPDF
SHHELP := PrintPDF,3d6
HTMLHELP := manual.html

ADDITIONS := Printers

PACKAGE := PrintPDF
PACKAGELOC := Printing

OBJS := api.o		\
	bookmark.o	\
	choices.o	\
	convert.o	\
	encrypt.o	\
	iconbar.o	\
	main.o		\
	optimize.o	\
	paper.o		\
	pdfmark.o	\
	pmenu.o		\
	popup.o		\
	taskman.o	\
	version.o

SUBS := !Boot,feb !Run,feb

include $(SFTOOLS_MAKE)/CApp

