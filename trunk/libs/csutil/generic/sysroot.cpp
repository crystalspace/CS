/*
    Find System Roots
    Copyright (C) 2002 by Eric Sunshine.

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
#include "csutil/syspath.h"
#include "csutil/scfstringarray.h"
#include "csutil/util.h"

csRef<iStringArray> csInstallationPathsHelper::FindSystemRoots()
{
  scfStringArray* p = new scfStringArray(0);
  p->Push("/");
  csRef<iStringArray> v(p);
  p->DecRef();
  return v;
}
