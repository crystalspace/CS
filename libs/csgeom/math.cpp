/*
  Copyright (C) 2011 by Frank Richter
  
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
#include "csgeom/math.h"

namespace CS
{
#ifdef __STRICT_ANSI__
  #error Building CS with -ansi -pedantic? - come on, be realistic!
#endif
  
  bool IsNaN_ext (double d)
  {
    /* No recursion, as CS is not (well, not supposed to be) built with
       -ansi -pedantic */
    return IsNaN (d);
  }
  
  bool IsFinite_ext (double d)
  {
    /* No recursion either, for the same reason. */
    return IsFinite (d);
  }
} // namespace CS
