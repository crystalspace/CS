/*
    Copyright (C) 2003 by Anders Stenberg

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

#ifndef __CS_SCFSTRSET_H__
#define __CS_SCFSTRSET_H__

#include "iutil/strset.h"
#include "csutil/strset.h"

/// This is a thin SCF wrapper around csStringSet
class csScfStringSet : public iStringSet
{
  csStringSet set;

public:
  SCF_DECLARE_IBASE;

  /// Create an empty scfStringSet object
  csScfStringSet ()
  { SCF_CONSTRUCT_IBASE (0); }

  /// Create an scfStringSet object and set the size of the hash
  csScfStringSet (uint32 size) : set(size)
  { SCF_CONSTRUCT_IBASE (0); }

  /// Destructor.
  virtual ~csScfStringSet() {}

  virtual csStringID Request (const char *s);

  virtual const char* Request (csStringID id);

  virtual void Clear ();
};

#endif // __CS_SCFSTRSET_H__
