/* PrintPDF - pmenu.h
 *
 * (c) Stephen Fryatt, 2007-2011
 */

#ifndef PRINTPDF_PMENU
#define PRINTPDF_PMENU


/**
 * Return an entry from a comma-separated list of parameters in a message
 * token.
 *
 * \param *buffer		Pointer to a buffer to take the entry.
 * \param *param_list		The list message token.
 * \param entry			The number of the item to return.
 * \return			Pointer to the \0 terminator in the buffer.
 */

char *pmenu_list_entry(char *buffer, char* param_list, int entry);

#endif

