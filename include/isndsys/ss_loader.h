/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_SNDSYS_LOADER_H__
#define __CS_SNDSYS_LOADER_H__

#include "csutil/scf.h"
#include "csutil/ref.h"

struct iSndSysData;

SCF_VERSION (iSndSysLoader, 0, 1, 0);

/**
 * The sound loader is used to load sound files given a raw input data stream.
 */
struct iSndSysLoader : public iBase
{
  /// Create a sound object from raw input data.
  virtual csPtr<iSndSysData> LoadSound(void *Data, uint32 Size) = 0;
};

#endif // __CS_SNDSYS_LOADER_H__
