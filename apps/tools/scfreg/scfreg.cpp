/*
    Copyright (C) 1999 by Andrew Zabolotny

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>

#define CS_SYSDEF_PROVIDE_PATH
#define CS_SYSDEF_PROVIDE_GETOPT
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"

CS_IMPLEMENT_APPLICATION

char *programversion = "3";
char *programname;

static struct option long_options[] =
{
  {"help", no_argument, 0, 'h'},
  {"register", no_argument, 0, 'r'},
  {"unregister", no_argument, 0, 'u'},
  {"version", no_argument, 0, 'V'},
  {"verbose", no_argument, 0, 'v'},
  {0, no_argument, 0, 0}
};

static int display_help ()
{
  printf ("Crystal Space SCF object registration utility v%s\n", programversion);
  printf ("Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>\n\n");
  printf ("Usage: %s {option/s} [plug-in module] [...]\n\n", programname);
  printf ("By default the program will register the SCF components contained\n");
  printf ("in the plug-in module passed on command line. Registration occurs\n" );
  printf ("in the SCF registration file scf.cfg.\n\n");
  printf ("  -r   --register    Register SCF components contained in file #\n");
  printf ("  -u   --unregister  Unregister SCF components contained in file #\n");
  printf ("  -h   --help        Display this help text\n");
  printf ("  -v   --verbose     Comment on what's happening\n");
  printf ("  -V   --version     Display program version\n");
  return 1;
}

static bool verbose = false;

static bool RegisterServer (char *SharedLibraryFilename, csConfigFile &cfg,
  bool Register)
{
  static bool DisplayedHeader = false;
  if (!DisplayedHeader && !verbose)
  {
    DisplayedHeader = true;
    printf ("Library | Class ID                     | Description\n");
    printf ("--------+------------------------------+---------------------------------------\n");
  }

  if (verbose)
  {
    printf ("Loading shared library %s ...", SharedLibraryFilename);
    fflush (stdout);
  } /* endif */

  char name [200], base [200];
  char *tmp = SharedLibraryFilename + strlen (SharedLibraryFilename);
  while ((tmp > SharedLibraryFilename)
      && (tmp [-1] != '/')
      && (tmp [-1] != PATH_SEPARATOR))
    tmp--;

  if ((tmp [0] == 'l')
   && (tmp [1] == 'i')
   && (tmp [2] == 'b'))
    tmp += 3;
  strcpy (base, tmp);
  if ((tmp = strchr (base, '.')))
    *tmp = 0;
  strcpy (name, base);
  strcat (name, "_scfInitialize");

  csLibraryHandle Handle = csFindLoadLibrary (base);
  if (!Handle)
    Handle = csLoadLibrary (base);

  if (!Handle)
  {
    if (verbose)
      printf (" FAILED\n");
    else
      printf ("ERROR: Failed to load library \"%s\"\n", SharedLibraryFilename);
    fflush (stdout);
    return false;
  } /* endif */

  if (verbose)
  {
    printf ("OK, handle = %08X\n", (int)Handle);
    fflush (stdout);
  } /* endif */

  if (verbose)
  {
    printf ("Looking for \"%s\" entry ...", name);
    fflush (stdout);
  } /* endif */

  // This is the prototype for the only function that
  // a shared library should export
  typedef scfClassInfo *(*scfInitializeFunc) (iSCF*);
  scfInitializeFunc func = (scfInitializeFunc)csGetLibrarySymbol (Handle, name);
  if (!func)
  {
    if (verbose)
      printf (" FAILED\n");
    else
      printf ("ERROR: Failed to find entry \"%s\" in library \"%s\"\n", name, SharedLibraryFilename);
    fflush (stdout);
    return false;
  } /* endif */

  if (verbose)
  {
    printf ("OK, address = %08lX\n", (unsigned long)func);
    fflush (stdout);
  } /* endif */

  scfClassInfo *ClassTable = func (NULL);
  while (ClassTable->ClassID)
  {
    if (verbose)
      printf ("Library:      %s\nClassID:      %s\nDescription:  %s\nDependencies: %s\n",
        SharedLibraryFilename, ClassTable->ClassID, ClassTable->Description,
        ClassTable->Dependencies ? ClassTable->Dependencies : "<none>");
    else
    {
      char lib [8+1], cls [30+1], desc [39+1];
      strncpy (lib, base, sizeof (lib) - 1);
      lib [sizeof (lib) - 1] = 0;
      strncpy (cls, ClassTable->ClassID, sizeof (cls) - 1);
      cls [sizeof (cls) - 1] = 0;
      strncpy (desc, ClassTable->Description, sizeof (desc) - 1);
      desc [sizeof (desc) - 1] = 0;

      printf ("%-8s|%-30s|%s\n", lib, cls, desc);
    }

    if (Register)
    {
      char text [1000];
      bool AddComment = !cfg.KeyExists (ClassTable->ClassID);
      strcpy (text, base);
      if (ClassTable->Dependencies)
      {
        strcat (text, ":");
        strcat (text, ClassTable->Dependencies);
      }
      cfg.SetStr (ClassTable->ClassID, text);
      if (AddComment)
        cfg.SetComment (ClassTable->ClassID, ClassTable->Description);
    }
    else
      cfg.DeleteKey (ClassTable->ClassID);
    ClassTable++;
  }

  if (verbose)
  {
    printf ("Unloading library \"%s\"...", SharedLibraryFilename);
    fflush (stdout);
  }

  if (!csUnloadLibrary (Handle))
  {
    if (verbose)
      printf (" FAILED\n");
    else
      printf ("WARNING: Unable to unload library \"%s\"!\n", SharedLibraryFilename);
  }
  else if (verbose)
    printf ("OK\n");

  if (verbose)
    printf ("Library \"%s\" %sregistered.\n", SharedLibraryFilename,
      Register ? "" : "de");
  fflush (stdout);
  return true;
}

int main (int argc, char *argv[])
{
#if defined (__EMX__)	// Expand wildcards on OS/2+GCC+EMX
  _wildcard (&argc, &argv);
#endif

  int operation = 1;
  programname = argv [0];

#ifndef COMP_BC
  int c;
  while ((c = getopt_long (argc, argv, "ruhvV", long_options, NULL)) != EOF)
    switch (c)
    {
      case '?':
        // unknown option
        return -1;
      case 'r':
        operation = 1;
        break;
      case 'u':
        operation = 2;
        break;
      case 'h':
        return display_help ();
      case 'v':
        verbose = true;
        break;
      case 'V':
        printf ("%s version %s\n\n", programname, programversion);
        printf ("This program is distributed in the hope that it will be useful,\n");
        printf ("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
        printf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n");
        printf ("GNU Library General Public License for more details.\n");
        return 0;
    } /* endswitch */
#else
  int optind=1;
#endif

  csConfigFile cfg ("scf.cfg");

  // Interpret the non-option arguments as file names
  bool did_something = false;
  for (; optind < argc; ++optind)
    switch (operation)
    {
      case 1:
        did_something = true;
        RegisterServer (argv [optind], cfg, true);
        break;
      case 2:
        did_something = true;
        RegisterServer (argv [optind], cfg, false);
        break;
    } /* endswitch */

  if (!did_something)
    return display_help ();

  cfg.Save ("scf.cfg");

  return 0;
}
