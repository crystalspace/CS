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

/*
    This program is a wrapper around linker (currently only gcc/emx
    linker is supported, but not much work is required to support other)
    that allows on-the-fly generation of .DEF files driven through
    command-line arguments. This wrapper is highly Crystal-Space specific
    in the terms that it generates .def file for DLLs explicitly for
    Crystal Space SCF model. It was originally a REXX script, but it
    was rewriten as a C++ program because CMD.EXE has an annoying
    limit of 1024 characters on the length of the command line passed
    to any subprocess (including REXX scripts); if this limit is exceeded
    it can quietly fail, it can lock up such that the process becomes
    "unkillable" (!) or it can do other "neat" things.
*/

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/param.h>

static struct
{
  char *linker;
  char *description;
  bool console;
  bool fullscreen;
  bool verbose;
  bool dll;
  char *modname;
  char *outdir;
} opt =
{
  "gcc",
  NULL,
  false,
  false,
  false,
  false,
  NULL,
  NULL
};

static char *program_name;
static int new_argc;
static char **new_argv;

static void display_version ()
{
  printf ("OS/2 linker wrapper  Version 0.0.1  Copyright (C) 1999 Andrew Zabolotny\n");
}

static void display_help ()
{
  display_version ();
  printf ("\nUsage: os2link {option/s} {linker command line}\n");
  printf ("  --linker=#      Set linker to use (default: gcc)\n");
  printf ("  --description=# Set output file description\n");
  printf ("  --out=#         Set .def file output directory\n");
  printf ("  --console       Create a .DEF file for console application\n");
  printf ("  --fullscreen    Create a .DEF file for a full-screen console application\n");
  printf ("  --verbose       Display the linker command line\n");
  printf ("  --help          Display usage help\n");
  printf ("  --version       Show program version\n");
}

static char *optarg (char *opt, size_t optlen)
{
  char *ret = opt + optlen;
  if (*ret != '=')
  {
    fprintf (stderr, "%s: No '=' separator in option: %s", program_name, opt);
    exit (-1);
  }
  return ret + 1;
}

static void addopt (char *opt)
{
  new_argv = (char **)realloc (new_argv, ++new_argc * sizeof (char *));
  new_argv [new_argc - 1] = strdup (opt);
}

int main (int argc, char *argv[])
{
  program_name = _getname (argv [0]);
  new_argc = 1;
  new_argv = (char **)malloc (sizeof (char *));

  int idx;
  for (idx = 1; idx < argc; idx++)
  {
    char *arg = argv [idx];

    if ((strcmp (arg, "-o") == 0) && (idx + 1 < argc))
    {
      addopt (arg);
      arg = argv [++idx];
      addopt (arg);
      char *name = _getname (arg);
      char *ext = _getext (name);
      if (ext)
      {
        *ext++ = 0;
        if (stricmp (ext, "dll") == 0)
          opt.dll = true;
      }
      opt.modname = strdup (name);
      if (!opt.outdir)
      {
        *name = 0;
        opt.outdir = strdup (arg);
      }
    }
    else if ((arg [0] == '-') && (arg [1] == '-'))
    {
      size_t sl;
      arg += 2;
      #define ISOPT(name)  (strncmp (name, arg, sl = strlen (name)) == 0)

      if (ISOPT ("linker"))
        opt.linker = optarg (arg, sl);
      else if (ISOPT ("description"))
        opt.description = optarg (arg, sl);
      else if (ISOPT ("out"))
        opt.outdir = optarg (arg, sl);
      else if (ISOPT ("console"))
        opt.console = true;
      else if (ISOPT ("fullscreen"))
        opt.fullscreen = true;
      else if (ISOPT ("verbose"))
        opt.verbose = true;
      else if (ISOPT ("help"))
      {
        display_help ();
        exit (-1);
      }
      else if (ISOPT ("version"))
      {
        display_version ();
        exit (-1);
      }
      else
        addopt (argv [idx]);

      #undef ISOPT
    }
    else
      addopt (argv [idx]);
  }

  if (argc < 2)
  {
    fprintf (stderr, "%s: Nothing to do; try --help for a list of options\n", program_name);
    exit (-1);
  }

  if (!opt.modname)
  {
    fprintf (stderr, "%s: No '-o' option on command line: don't know module name\n", program_name);
    exit (-1);
  }

  new_argv [0] = opt.linker;

  char defname [CS_MAXPATHLEN + 1];
  sprintf (defname, "%s%s.def", opt.outdir, opt.modname);

  // Create the .def file
  FILE *f = fopen (defname, "w");
  if (!f)
  {
    fprintf (stderr, "%s: Cannot create temporary file %s\n", program_name, defname);
    exit (-1);
  }

  if (opt.dll)
  {
    fprintf (f, "LIBRARY %s INITINSTANCE TERMINSTANCE\n", opt.modname);
    if (opt.description)
      fprintf (f, "DESCRIPTION \"%s\"\n", opt.description);
    fprintf (f, "DATA NONSHARED\n");
    fprintf (f, "EXPORTS\n");
    fprintf (f, "	%s_scfInitialize\n", opt.modname);
  }
  else
  {
    fprintf (f, "NAME %s %s\n", opt.modname, opt.fullscreen ? "NOTWINDOWCOMPAT" :
      opt.console ? "WINDOWCOMPAT" : "WINDOWAPI");
    if (opt.description)
      fprintf (f, "DESCRIPTION \"%s\"\n", opt.description);
    fprintf (f, "STACKSIZE 1048576\n");
  }
  fclose (f);
  addopt (defname);

  // print the entire command line if required
  if (opt.verbose)
  {
	int i;
    for (i = 0; i < new_argc; i++)
      printf ("%s%s", i ? " " : "", new_argv [i]);
    printf ("\n");
  }

  // Add the final NULL argument
  new_argv = (char **)realloc (new_argv, ++new_argc * sizeof (char *));
  new_argv [new_argc - 1] = NULL;

  // Launch the linker
  int retcode = spawnv (P_WAIT, opt.linker, new_argv);

  if (retcode == -1)
    fprintf (stderr, "%s: failed to spawn %s\n", program_name, new_argv [0]);
  return retcode;
}
