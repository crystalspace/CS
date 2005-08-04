/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef __CS_INETWORK_VOSAPI_H__
#define __CS_INETWORK_VOSAPI_H__

#include "csutil/scf.h"

#include <vos/vos/vos.hh>

SCF_VERSION (iVosApi, 0, 1, 1);
struct iVosApi : public virtual iBase
{
    virtual VUtil::vRef<VOS::Vobject> GetVobject() = 0;
};

#endif
