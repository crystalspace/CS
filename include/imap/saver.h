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

#ifndef __CS_IMAP_SAVER_H__
#define __CS_IMAP_SAVER_H__

#include "csutil/scf.h"

struct iDocumentNode;
struct iString;

SCF_VERSION (iSaver, 0, 0, 3);

/**
* This interface is used to serialize the engine
* contents.
*/ 
struct iSaver : public iBase
{
  /// Save the current engine contents to the filename.
  virtual bool SaveMapFile(const char *filename) = 0;
  /// Return the current engine contents as a string.
  virtual csRef<iString> SaveMapFile() = 0;
  /// Save map to DocumentNode
  virtual bool SaveMapFile(csRef<iDocumentNode> &root) = 0;
};

#endif // __CS_IMAP_SAVER_H__
