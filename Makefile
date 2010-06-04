# Makefile for PrintPDF
#
# Copyright 2010, Stephen Fryatt
#
# This file really needs to be run by GNUMake.
# It is intended for cross-compilation under the GCCSDK.

.PHONY: all clean documentation release

BUILD_DATE := $(shell date "+%-d %b %Y")

# Build Tools

CC := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*gcc)

RM := rm -rf
CP := cp

ZIP := /home/steve/GCCSDK/env/bin/zip

SFBIN := /home/steve/GCCSDK/sfbin/

TEXTMAN := $(SFBIN)textman
STRONGMAN := $(SFBIN)strongman
HTMLMAN := $(SFBIN)htmlman
DDFMAN := $(SFBIN)ddfman
BINDHELP := $(SFBIN)bindhelp

# Build Flags

CCFLAGS := -mlibscl -mhard-float -static -mthrowback -Wall -O2 -D'BUILD_DATE="$(BUILD_DATE)"' -fno-strict-aliasing -mpoke-function-name
ZIPFLAGS := -r -, -9 -j
BINDHELPFLAGS := -f -r -v

# Includes and libraries.

INCLUDES := -I$(GCCSDK_INSTALL_ENV)/include -I$(GCCSDK_LIBS)/OSLib-Hard/ -I$(GCCSDK_LIBS)/OSLibSupport/ -I$(GCCSDK_LIBS)/SFLib/ -I$(GCCSDK_LIBS)/FlexLib/
LINKS := -L$(GCCSDK_LIBS)/OSLib-Hard/ -lOSLib32 -L$(GCCSDK_INSTALL_ENV)/lib -L$(GCCSDK_LIBS)/SFLib/ -lSFLib32 -L$(GCCSDK_LIBS)/FlexLib/ -lFlexLib32

# Set up the various build directories.

SRCDIR := src
MANUAL := manual
OBJDIR := obj
OUTDIR := build

# Set up the named target files.

APP := !PrintPDF
UKRES := Resources/UK
RUNIMAGE := !RunImage,ffa
TEXTHELP := HelpText,fff
SHHELP := PrintPDF,3d6
README := ReadMe,fff

# Set up the source files.

MANSRC := Source
MANSPR := ManSprite
READMEHDR := Header

OBJS := bookmark.o choices.o convert.o dataxfer.o encrypt.o ihelp.o init.o main.o menus.o \
        optimize.o pdfmark.o pmenu.o popup.o taskman.o version.o windows.o

# Start to define the targets.

all: $(OUTDIR)/$(APP)/$(RUNIMAGE)

# Build the complete !RunImage from the object files.

OBJS := $(addprefix $(OBJDIR)/, $(OBJS))

$(OUTDIR)/$(APP)/!RunImage,ffa: $(OBJS)
	$(CC) $(CCFLAGS) $(LINKS) -o $(OUTDIR)/$(APP)/$(RUNIMAGE) $(OBJS)

# Build the object files, and identify their dependencies.

-include $(OBJS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $< -o $@
	@$(CC) -MM $(CCFLAGS) $(INCLUDES) $< > $(@:.o=.d)
	@mv -f $(@:.o=.d) $(@:.o=.d).tmp
	@sed -e 's|.*:|$*.0:|' < $(@:.o=.d).tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d).tmp

# Build the documentation

documentation: $(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP) $(OUTDIR)/$(APP)/$(UKRES)/$(SHHELP) $(OUTDIR)/$(README)

$(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP): $(MANUAL)/$(MANSRC)
	$(TEXTMAN) $(MANUAL)/$(MANSRC) $(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP)

$(OUTDIR)/$(APP)/$(UKRES)/$(SHHELP): $(MANUAL)/$(MANSRC) $(MANUAL)/$(MANSPR)
	$(STRONGMAN) $(MANUAL)/$(MANSRC) SHTemp
	$(CP) $(MANUAL)/$(MANSPR) SHTemp/Sprites,ff9
	$(BINDHELP) SHTemp $(OUTDIR)/$(APP)/$(UKRES)/$(SHHELP) $(BINDHELPFLAGS)
	$(RM) SHTemp

$(OUTDIR)/$(README):$(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP)
	$(CP) $(OUTDIR)/$(APP)/$(UKRES)/$(TEXTHELP) $(OUTDIR)/$(README)

# Clean targets

clean:
	$(RM) $(OBJDIR)/*

