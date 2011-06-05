/* PrintPDF - choices.h
 * (c) Stephen Fryatt, 2007-2011
 *
 */

#ifndef PRINTPDF_CHOICES
#define PRINTPDF_CHOICES

/* Main window icons. */

#define CHOICE_ICON_APPLY 0
#define CHOICE_ICON_SAVE 1
#define CHOICE_ICON_CANCEL 2
#define CHOICE_ICON_DEFFILE 6
#define CHOICE_ICON_VERSION 8
#define CHOICE_ICON_VERSION_MENU 9
#define CHOICE_ICON_OPTIMIZE 11
#define CHOICE_ICON_OPTIMIZE_MENU 12
#define CHOICE_ICON_INFO 24
#define CHOICE_ICON_INFO_MENU 25
#define CHOICE_ICON_ENCRYPT 14
#define CHOICE_ICON_ENCRYPT_MENU 15
#define CHOICE_ICON_PREPROCESS 16
#define CHOICE_ICON_POPUP 17
#define CHOICE_ICON_RESETEVERY 18
#define CHOICE_ICON_IBAR 19
#define CHOICE_ICON_MEMORY 21


/**
 * Initialise the Choices module.
 */

void choices_initialise(void);


/**
 * Open the Choices window at the mouse pointer.
 *
 * \param *pointer		The details of the pointer to open the window at.
 */

void choices_open_window(wimp_pointer *pointer);





void process_choices_version_menu (wimp_selection *selection);
void process_choices_optimize_menu (wimp_selection *selection);

#endif

