/* PrintPDF - main.h
 *
 * (c) Stephen Fryatt, 2007-2011
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

extern int		global_encryption_dialogue_location;
extern int		global_optimization_dialogue_location;
extern int		global_pdfmark_dialogue_location;
extern int		global_bookmark_dialogue_location;

extern wimp_t		main_task_handle;
extern int		main_quit_flag;
extern osspriteop_area	*main_wimp_sprites;

/**
 * Main code entry point.
 */

int main(int argc, char *argv[]);

#endif

