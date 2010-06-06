/* PrintPDF - main.h
 *
 * (c) Stephen Fryatt, 2007
 */

#ifndef _PRINTPDF_MAIN
#define _PRINTPDF_MAIN

/* ==================================================================================================================
 * Static constants
 */

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

/* ------------------------------------------------------------------------------------------------------------------ */

int main (int argc, char *argv[]);
int poll_loop (void);
void mouse_click_handler (wimp_pointer *);
void key_press_handler (wimp_key *key);
void menu_selection_handler (wimp_selection *);
void user_message_handler (wimp_message *);
void bounced_message_handler (wimp_message *);

#endif
