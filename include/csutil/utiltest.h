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

#ifndef __CS_CSUTIL_UNITTEST_H__
#define __CS_CSUTIL_UNITTEST_H__

#include "csextern.h"
#include "iutil/dbghelp.h"

/**
 * This is a class that does unit testing (and other debug stuff) for some
 * of csutil classes.
 */
class CS_CRYSTALSPACE_EXPORT csUtilDebugHelper : public iDebugHelper
{
public:
  csUtilDebugHelper ();
  virtual ~csUtilDebugHelper ();

  SCF_DECLARE_IBASE;
  virtual int GetSupportedTests () const
  {
    return CS_DBGHELP_UNITTEST;
  }
  virtual csPtr<iString> UnitTest ();
  virtual csPtr<iString> StateTest ()
  {
    return 0;
  }
  virtual csTicks Benchmark (int /*num_iterations*/)
  {
    return 0;
  }
  virtual csPtr<iString> Dump ()
  {
    return 0;
  }
  virtual void Dump (iGraphics3D* /*g3d*/)
  {
  }
  virtual bool DebugCommand (const char*)
  {
    return false;
  }
};

#endif // __CS_CSUTIL_UNITTEST_H__

