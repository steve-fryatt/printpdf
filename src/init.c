/* PrintPDF - init.c
 * (c) Stephen Fryatt, 2005
 */

/* ANSI C Header files. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Acorn C Header files. */

#include "flex.h"

/* OSLib Header files. */

#include "oslib/wimp.h"
#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"
#include "oslib/uri.h"
#include "oslib/hourglass.h"
#include "oslib/pdriver.h"
#include "oslib/help.h"

/* SF-Lib Header files. */

#include "sflib/config.h"
#include "sflib/resources.h"
#include "sflib/menus.h"
#include "sflib/errors.h"
#include "sflib/url.h"
#include "sflib/heap.h"
#include "sflib/msgs.h"
#include "sflib/debug.h"
#include "sflib/windows.h"

/* Application header files. */

#include "init.h"

#include "bookmark.h"
#include "convert.h"
#include "menus.h"
#include "pmenu.h"
#include "taskman.h"
#include "windows.h"

/* ================================================================================================================== */

int initialise (void)
{
  static char                       task_name[255];
  char                              resources[255], res_temp[255], filename[256];
  osspriteop_area                   *sprites;
  extern wimp_t                     task_handle;
  extern int                        quit_flag;
  extern osspriteop_area            *wimp_sprites;

  wimp_MESSAGE_LIST(13)             message_list;
  wimp_version_no                   wimp_version;


  hourglass_on ();

  strcpy (resources, "<PrintPDF$Dir>.Resources");
  find_resource_path (resources, sizeof (resources));

  /* Load the messages file. */

  sprintf (res_temp, "%s.Messages", resources);
  msgs_init (res_temp);

  /* Initialise the error message system. */

  error_initialise ("TaskName", "TaskSpr", NULL);

  /* Initialise with the Wimp. */

  message_list.messages[0]=message_URI_RETURN_RESULT;
  message_list.messages[1]=message_ANT_OPEN_URL;
  message_list.messages[2]=message_DATA_SAVE;
  message_list.messages[3]=message_DATA_SAVE_ACK;
  message_list.messages[4]=message_DATA_LOAD;
  message_list.messages[5]=message_DATA_OPEN;
  message_list.messages[6]=message_MENU_WARNING;
  message_list.messages[7]=message_MENUS_DELETED;
  message_list.messages[8]=message_HELP_REQUEST;
  message_list.messages[9]=message_TASK_INITIALISE;
  message_list.messages[10]=message_TASK_CLOSE_DOWN;
  message_list.messages[11]=message_PRE_QUIT;
  message_list.messages[12]=message_QUIT;
  msgs_lookup ("TaskName", task_name, sizeof (task_name));
  task_handle = wimp_initialise (wimp_VERSION_RO3, task_name, (wimp_message_list *) &message_list, &wimp_version);

  /* Test to see if any other copies of PrintPDF are running, and set to quit if they are. */

  if (task_is_running (task_name, task_handle))
  {
    quit_flag = TRUE;
  }

  /* Initialise the configuration. */

  initialise_configuration (task_name, "PrintPDF", "<PrintPDF$Dir>");

  init_config_str ("FileQueue", "<Wimp$ScrapDir>.PrintPDF");
  init_config_str ("ParamFile", "Pipe:$.PrintPDF");
  init_config_str ("PDFMarkFile", "Pipe:$.PrintPDFMark");
  init_config_str ("FileName", msgs_lookup ("FileName", filename, sizeof (filename)));
  init_config_int ("PollDelay", 500);
  init_config_int ("PopUpTime", 200);
  init_config_int ("TaskMemory", 8192);
  init_config_int ("PDFVersion", 0);
  init_config_int ("Optimization", 0);
  init_config_opt ("DownsampleMono", FALSE);
  init_config_int ("DownsampleMonoType", 0);
  init_config_int ("DownsampleMonoResolution", 300);
  init_config_int ("DownsampleMonoThreshold", 15);
  init_config_int ("DownsampleMonoDepth", -1);
  init_config_opt ("DownsampleGrey", FALSE);
  init_config_int ("DownsampleGreyType", 0);
  init_config_int ("DownsampleGreyResolution", 72);
  init_config_int ("DownsampleGreyThreshold", 15);
  init_config_int ("DownsampleGreyDepth", -1);
  init_config_opt ("DownsampleColour", FALSE);
  init_config_int ("DownsampleColourType", 0);
  init_config_int ("DownsampleColourResolution", 72);
  init_config_int ("DownsampleColourThreshold", 15);
  init_config_int ("DownsampleColourDepth", -1);
  init_config_opt ("EncodeMono", TRUE);
  init_config_int ("EncodeMonoType", 2);
  init_config_opt ("EncodeGrey", TRUE);
  init_config_int ("EncodeGreyType", 0);
  init_config_opt ("EncodeColour", TRUE);
  init_config_int ("EncodeColourType", 0);
  init_config_int ("AutoPageRotation", 2);
  init_config_opt ("CompressPages", TRUE);
  init_config_str ("OwnerPasswd", "");
  init_config_str ("UserPasswd", "");
  init_config_opt ("AllowPrint", TRUE);
  init_config_opt ("AllowFullPrint", TRUE);
  init_config_opt ("AllowExtraction", TRUE);
  init_config_opt ("AllowFullExtraction", TRUE);
  init_config_opt ("AllowForms", TRUE);
  init_config_opt ("AllowAnnotation", TRUE);
  init_config_opt ("AllowModifications", TRUE);
  init_config_opt ("AllowAssembly", TRUE);
  init_config_str ("PDFMarkTitle", "");
  init_config_str ("PDFMarkAuthor", "");
  init_config_str ("PDFMarkSubject", "");
  init_config_str ("PDFMarkKeywords", "");
  init_config_str ("PDFMarkUserFile", "");
  init_config_opt ("PreProcess", FALSE);
  init_config_opt ("ResetParams", FALSE);
  init_config_opt ("IconBarIcon", TRUE);
  init_config_opt ("PopUpAfter", TRUE);

  load_configuration ();

/*  initialise_param_menu ("PDFVersion", read_config_int ("PDFVersion"));
  initialise_param_menu ("Optimization", read_config_int ("Optimization")); */

  /* Load the window templates. */

  sprites = load_user_sprite_area ("<PrintPDF$Dir>.Sprites");

  wimp_sprites = sprites;

  sprintf (res_temp, "%s.Templates", resources);
  load_window_templates (res_temp, sprites);

  /* Load the menu structure. */

  sprintf (res_temp, "%s.Menus", resources);
  load_menu_definitions (res_temp);

  /* Create an icon-bar icon. */

  set_iconbar_icon (read_config_opt ("IconBarIcon") && (quit_flag == FALSE));

  /* Initialise the postscript file queue and save box. */

  initialise_conversion ();
  initialise_bookmarks();

  hourglass_off ();

  return (0);
}

/* ================================================================================================================== */

/* Take the command line and parse it for useful arguments. */

void parse_command_line (int argc, char *argv[])
{
  int i;

  if (argc > 1)
  {
    for (i=1; i<argc; i++)
    {
      if (strcmp (argv[i], "-file") == 0 && i+1 < argc)
      {
        load_bookmark_file(argv[i+1]);
      }
    }
  }
}

