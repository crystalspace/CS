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
    This program uses a feature of OS/2 that allows to launch PM programs
    as fullscreen sessions. If a full-screen VIO program executes a child
    process using exec() instead of DosStartSession() it will be executed
    in same process. This is used to launch Crystal Space in full screen
    so that MGL 2D driver can work.
*/

#include <process.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf (stderr, "Usage: startfs <executable>\n");
    return -1;
  }

  // Wait for session switch to finish...
  sleep (1);

  execv (argv [1], &argv [1]);

  fprintf (stderr, "%s: %s\n", argv [1], strerror (errno));
  return -1;
}
