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

#ifndef __ISNDLDR_H__
#define __ISNDLDR_H__

#include "isys/iplugin.h"

struct iSoundData;
struct csSoundFormat;

SCF_VERSION (iSoundLoader, 1, 0, 0);

/**
 * The sound loader plugin is used to load sound files from the VFS and
 * create sound data objects or sound streams from it.
 */
struct iSoundLoader : public iPlugIn
{
  /// Initialize the Sound Loader.
  virtual bool Initialize (iSystem *sys) = 0;

  /// Load a sound file from the VFS.
  virtual iSoundData *LoadSound(void *Data, unsigned long Size,
    const csSoundFormat *Format) = 0;
};

#endif
