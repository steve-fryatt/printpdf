/* PrintPDF - encrypt.h
 *
 * (c) Stephen Fryatt, 2005
 */

#ifndef _PRINTPDF_ENCRYPT
#define _PRINTPDF_ENCRYPT

/* ==================================================================================================================
 * Static constants
 */

/* Encryption Window icons. */

#define ENCRYPT_ICON_CANCEL 0
#define ENCRYPT_ICON_OK 1
#define ENCRYPT_ICON_OWNER_PW 3
#define ENCRYPT_ICON_ACCESS_PW 5
#define ENCRYPT_ICON_PRINT 8
#define ENCRYPT_ICON_MODS 9
#define ENCRYPT_ICON_EXTRACT 10
#define ENCRYPT_ICON_FORMS 11
#define ENCRYPT_ICON_FULL_PRINT 12
#define ENCRYPT_ICON_FULL_EXTRACT 13
#define ENCRYPT_ICON_ANNOTATE 14
#define ENCRYPT_ICON_ASSEMBLY 15

#define ENCRYPT_ICON_SHADE_BASE 4
#define ENCRYPT_ICON_SHADE_MAX2 11
#define ENCRYPT_ICON_SHADE_MAX3 15

/* Access permission bitmasks */

#define MAX_PASSWORD 50

#define ACCESS_REV2_BASE  0xffffffc0
#define ACCESS_REV2_PRINT 0x04
#define ACCESS_REV2_MODIFY 0x08
#define ACCESS_REV2_COPY 0x10
#define ACCESS_REV2_ANNOTATE 0x20

#define ACCESS_REV3_BASE 0xfffff0c0
#define ACCESS_REV3_PRINT 0x04
#define ACCESS_REV3_MODIFY 0x08
#define ACCESS_REV3_COPYALL 0x10
#define ACCESS_REV3_ANNOTATE 0x20
#define ACCESS_REV3_FORMS 0x100
#define ACCESS_REV3_COPYACCESS 0x200
#define ACCESS_REV3_ASSEMBLE 0x400
#define ACCESS_REV3_PRINTFULL 0x800

/* ==================================================================================================================
 * Data structures
 */

typedef struct encrypt_params
{
  char owner_password[MAX_PASSWORD];
  char access_password[MAX_PASSWORD];

  int  allow_print;
  int  allow_full_print;
  int  allow_extraction;
  int  allow_full_extraction;
  int  allow_forms;
  int  allow_annotation;
  int  allow_modifications;
  int  allow_assembly;
}
encrypt_params;

/* ==================================================================================================================
 * Function prototypes.
 */

/* Handle the encryption window. */

void initialise_encryption_settings (encrypt_params *params);
void open_encrypt_dialogue (encrypt_params *params, int extended_opts, wimp_pointer *pointer);
void process_encrypt_dialogue (encrypt_params *params);
void shade_encrypt_dialogue (wimp_w window);
void fill_encryption_field (wimp_w window, wimp_i icon, encrypt_params *params);
void build_encryption_params (char *buffer, encrypt_params *params, int extended_opts);


#endif
