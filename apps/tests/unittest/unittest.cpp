/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#include <stdarg.h>

#include "cssysdef.h"
#include "unittest.h"
#include "csengine/engine.h"
#include "csengine/xorbuf.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

// need to register the engine explicit here when not building static
#if !defined(CS_STATIC_LINKED)
SCF_REGISTER_STATIC_LIBRARY (engine)
#endif

//-----------------------------------------------------------------------------

UnitTest::UnitTest ()
{
}

UnitTest::~UnitTest ()
{
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (NULL));

  bool rc = csXORBuffer::Debug_UnitTest (10000);
  printf ("csXORBuffer unit testing %s\n", rc ? "succeeded." : "failed!");

  return 0;
}

