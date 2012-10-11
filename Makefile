# Copyright 2010-2012, Stephen Fryatt
#
# This file is part of PrintPDF:
#
#   http://www.stevefryatt.org.uk/software/
#
# Licensed under the EUPL, Version 1.1 only (the "Licence");
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

# Set VERSION to build using a version number and not an SVN revision.

.PHONY: all clean application documentation release backup

# The build date.

BUILD_DATE := $(shell date "+%d %b %Y")
HELP_DATE := $(shell date "+%-d %B %Y")

# Construct version or revision information.

ifeq ($(VERSION),)
  RELEASE := $(shell svnversion --no-newline)
  VERSION := r$(RELEASE)
  RELEASE := $(subst :,-,$(RELEASE))
  HELP_VERSION := ----
else
  RELEASE := $(subst .,,$(VERSION))
  HELP_VERSION := $(VERSION)
endif

$(info Building with version $(VERSION) ($(RELEASE)) on date $(BUILD_DATE))

# The archive to assemble the release files in.  If $(RELEASE) is set, then the file can be given
# a standard version number suffix.

ZIPFILE := printpdf$(RELEASE).zip
SRCZIPFILE := printpdf$(RELEASE)src.zip
BUZIPFILE := printpdf$(shell date "+%Y%m%d").zip

# Build Tools

CC := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*gcc)

MKDIR := mkdir
RM := rm -rf
CP := cp

ZIP := /home/steve/GCCSDK/env/bin/zip

SFBIN := /home/steve/GCCSDK/sfbin

TEXTMAN := $(SFBIN)/textman
STRONGMAN := $(SFBIN)/strongman
HTMLMAN := $(SFBIN)/htmlman
DDFMAN := $(SFBIN)/ddfman
BINDHELP := $(SFBIN)/bindhelp
TEXTMERGE := $(SFBIN)/textmerge
MENUGEN := $(SFBIN)/menugen


# Build Flags

CCFLAGS := -mlibscl -mhard-float -static -mthrowback -Wall -O2 -D'BUILD_VERSION="$(VERSION)"' -D'BUILD_DATE="$(BUILD_DATE)"' -fno-strict-aliasing -mpoke-function-name
ZIPFLAGS := -x "*/.svn/*" -r -, -9
SRCZIPFLAGS := -x "*/.svn/*" -r -9
BUZIPFLAGS := -x "*/.svn/*" -r -9
BINDHELPFLAGS := -f -r -v
MENUGENFLAGS := -d


# Includes and libraries.

INCLUDES := -I$(GCCSDK_INSTALL_ENV)/include -I$(GCCSDK_LIBS)/OSLib-Hard/ -I$(GCCSDK_LIBS)/OSLibSupport/ -I$(GCCSDK_LIBS)/SFLib/ -I$(GCCSDK_LIBS)/FlexLib/
LINKS := -L$(GCCSDK_LIBS)/OSLib-Hard/ -lOSLib32 -L$(GCCSDK_INSTALL_ENV)/lib -L$(GCCSDK_LIBS)/SFLib/ -lSFLib32 -L$(GCCSDK_LIBS)/FlexLib/ -lFlexLib32


# Set up the various build directories.

SRCDIR := src
MENUDIR := menus
MANUAL := manual
OBJDIR := obj
OUTDIR := build


# Set up the named target files.

APP := !PrintPDF
UKRES := Resources/UK
RUNIMAGE := !RunImage,ff8
MENUS := Menus,ffd
TEXTHELP := HelpText,fff
SHHELP := PrintPDF,3d6
HTMLHELP := manual.html
README := ReadMe,fff
LICENSE := License,fff
PRINTERS := Printers


# Set up the source files.

MANSRC := Source
MANSPR := ManSprite
READMEHDR := Header
MENUSRC := menudef

OBJS := bookmark.o choices.o convert.o dataxfer.o encrypt.o ihelp.o iconbar.o main.o \
        optimize.o pdfmark.o pmenu.o popup.o taskman.o templates.o version.o


# Build everything, but don't package it for release.

all: application documentation


# Build the application and its supporting binary files.

application: $(OUTDIR)/$(APP)/$(RUNIMAGE) $(OUTDIR)/$(APP)/$(UKRES)/$(MENUS)


# Build the complete !RunImage from the object files.

OBJS := $(addprefix $(OBJDIR)/, $(OBJS))

$(OUTDIR)/$(APP)/$(RUNIMAGE): $(OBJS) $(OBJDIR)
	$(CC) $(CCFLAGS) $(LINKS) -o $(OUTDIR)/$(APP)/$(RUNIMAGE) $(OBJS)


# Create a folder to hold the object files.

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

# Build the object files, and identify their dependencies.

-include $(OBJS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $< -o $@
	@$(CC) -MM $(CCFLAGS) $(INCLUDES) $< > $(@:.o=.d)
	@mv -f $(@:.o=.d) $(@:.o=.d).tmp
	@sed -e 's|.*:|$@:|' < $(@:.o=.d).tmp > $(@:.o=.d)
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d).tmp


# Build the menus file.

$(OUTDIR)/$(APP)/$(UKRES)/$(MENUS): $(MENUDIR)/$(MENUSRC)
	$(MENUGEN) $(MENUDIR)/$(MENUSRC) $(OUTDIR)/$(APP)/$(UKRES)/$(MENUS) $(MENUGENFLAGS)


# Build the documentation

documentation: $(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP) $(OUTDIR)/$(APP)/$(UKRES)/$(SHHELP) $(OUTDIR)/$(README) $(OUTDIR)/$(HTMLHELP)

$(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP): $(MANUAL)/$(MANSRC)
	$(TEXTMAN) -I$(MANUAL)/$(MANSRC) -O$(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP) -D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'

$(OUTDIR)/$(APP)/$(UKRES)/$(SHHELP): $(MANUAL)/$(MANSRC) $(MANUAL)/$(MANSPR)
	$(STRONGMAN) -I$(MANUAL)/$(MANSRC) -OSHTemp -D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'
	$(CP) $(MANUAL)/$(MANSPR) SHTemp/Sprites,ff9
	$(BINDHELP) SHTemp $(OUTDIR)/$(APP)/$(UKRES)/$(SHHELP) $(BINDHELPFLAGS)
	$(RM) SHTemp

$(OUTDIR)/$(README): $(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP) $(MANUAL)/$(READMEHDR)
	$(TEXTMERGE) $(OUTDIR)/$(README) $(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP) $(MANUAL)/$(READMEHDR) 5

$(OUTDIR)/$(HTMLHELP): $(MANUAL)/$(MANSRC)
	$(HTMLMAN) -I$(MANUAL)/$(MANSRC) -O$(OUTDIR)/$(HTMLHELP) -D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'


# Build the release Zip file.

release: clean all
	$(RM) ../$(ZIPFILE)
	(cd $(OUTDIR) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) $(APP) $(README) $(LICENSE) $(PRINTERS))
	$(RM) ../$(SRCZIPFILE)
	$(ZIP) $(SRCZIPFLAGS) ../$(SRCZIPFILE) $(OUTDIR) $(SRCDIR) $(MENUDIR) $(MANUAL) Makefile


# Build a backup Zip file

backup:
	$(RM) ../$(BUZIPFILE)
	$(ZIP) $(BUZIPFLAGS) ../$(BUZIPFILE) *


# Clean targets

clean:
	$(RM) $(OBJDIR)/*
	$(RM) $(OUTDIR)/$(APP)/$(RUNIMAGE)
	$(RM) $(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP)
	$(RM) $(OUTDIR)/$(APP)/$(UKRES)/$(SHHELP)
	$(RM) $(OUTDIR)/$(APP)/$(UKRES)/$(MENUS)
	$(RM) $(OUTDIR)/$(README)

