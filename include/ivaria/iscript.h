/*
    Copyright (C) 1999 by Brandon Ehle <azverkan@yahoo.com>

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

#ifndef __ISCRIPT_H__
#define __ISCRIPT_H__

#include "csutil/scf.h"
#include "isys/iplugin.h"

enum ResultType
{
  rtDouble=0,
  rtPointer
};

struct iSystem;

SCF_VERSION (iScript, 0, 0, 1);

/**
 * @@@ Please document me using Doc++!
 */
struct iScript : public iPlugIn
{
  ///
  virtual bool Initialize (iSystem *iSys) = 0;
  ///
  virtual bool RunText (const char *iStr) = 0;
  ///
  virtual bool LoadModule (const char *iStr) = 0;
  ///
  virtual bool Store (const char* type, const char* name, void* data) = 0;
};

#endif // __ISCRIPT_H__











