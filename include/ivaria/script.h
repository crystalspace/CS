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

#ifndef __IVARIA_SCRIPT_H__
#define __IVARIA_SCRIPT_H__

#include "csutil/scf.h"

struct iObjectRegistry;

SCF_VERSION (iScript, 0, 0, 1);

/**
 * @@@ Please document iScript with Doxygen comments.
 */
struct iScript : public iBase
{
  /// Initialize
  virtual bool Initialize (iObjectRegistry *object_reg) = 0;
  /// RunText
  virtual bool RunText (const char *iStr) = 0;
  /// LoadModule
  virtual bool LoadModule (const char *iStr) = 0;
  /// Store (Data in Name, Tag is specific to script language)
  virtual bool Store(const char* name, void* data, void* tag) = 0;
};

#endif // __IVARIA_SCRIPT_H__











