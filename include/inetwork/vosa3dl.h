/*
    Copyright (C) 2004 by Peter Amstutz <tetron@interreality.org>
    Written by Peter Amstutz <tetron@interreality.org>

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

#ifndef __CS_INETWORK_VOSA3DL_H__
#define __CS_INETWORK_VOSA3DL_H__

#include "csutil/scf.h"
#include "csutil/ref.h"

struct iString;
struct iSector;
struct iMeshWrapper;

SCF_VERSION (iVosSector, 0, 1, 1);

struct iVosSector : public iBase
{
    virtual void Load() = 0;
    virtual csRef<iSector> GetSector() = 0;
};

SCF_VERSION (iVosA3DL, 0, 1, 1);

struct iVosA3DL : public iBase
{
    virtual csRef<iVosSector> GetSector(const char* s) = 0;
};

SCF_VERSION (iVosObject3D, 0, 1, 1);

struct iVosObject3D : public iBase
{
    virtual csRef<iMeshWrapper> GetMeshWrapper() = 0;
};

#endif
