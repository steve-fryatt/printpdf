/* PrintPDF - choices.h
 * (c) Stephen Fryatt, 2007
 *
 */

#ifndef _PRINTPDF_CHOICES
#define _PRINTPDF_CHOICES

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

/* ------------------------------------------------------------------------------------------------------------------ */

void open_choices_window (wimp_pointer *pointer);
void close_choices_window (void);

void set_choices_window (void);
void read_choices_window (void);

void redraw_choices_window (void);

void handle_choices_icon_drop (wimp_full_message_data_xfer *datasave);

void open_choices_version_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_choices_version_menu (wimp_selection *selection);
void open_choices_optimize_menu (wimp_pointer *pointer, wimp_w window, wimp_i icon);
void process_choices_optimize_menu (wimp_selection *selection);

void open_choices_encrypt_dialogue (wimp_pointer *pointer);
void process_choices_encrypt_dialogue (void);
void open_choices_pdfmark_dialogue (wimp_pointer *pointer);
void process_choices_pdfmark_dialogue (void);
void process_choices_optimize_dialogue (void);

#endif
