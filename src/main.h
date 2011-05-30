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

extern wimp_t		task_handle;
extern int		quit_flag;
extern osspriteop_area	*wimp_sprites;

/**
 * Main code entry point.
 */

int main(int argc, char *argv[]);

#endif

