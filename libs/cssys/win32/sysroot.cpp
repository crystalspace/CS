/*
    Find System Roots
    Copyright (C) 2002 by Mark Carnes
    Copyright (C) 2002 by Eric Sunshine

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

#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/scfstrv.h"
#include "csutil/util.h"

csRef<iStrVector> csFindSystemRoots()
{
  int const lim = 199;
  char* buffer = new char[lim + 1];
  int len = GetLogicalDriveStrings(lim, buffer);
  if (len > lim)
  {
    delete [] buffer;
    buffer = new char[len + 1];
    len = GetLogicalDriveStrings(len, buffer);
  }

  scfStrVector* p = new scfStrVector;
  for (char const* s = buffer; *s != '\0'; s += strlen(s) + 1)
    p->Push(csStrNew(s));
  csRef<iStrVector> v(p);
  p->DecRef();

  delete[] buffer;
  return v;
}
