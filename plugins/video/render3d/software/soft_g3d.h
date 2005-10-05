/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
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

#ifndef __CS_SOFT_RENDER3D_H__
#define __CS_SOFT_RENDER3D_H__

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/pluginconfig.h"
#include "sft3dcom.h"

namespace cspluginSoft3d
{

/// Software 3D renderer
class csSoftwareGraphics3D : 
  public scfImplementationExt1<csSoftwareGraphics3D,
			       csSoftwareGraphics3DCommon,
			       iPluginConfig>
{
public:
  /// Constructor
  csSoftwareGraphics3D (iBase*);
  /// Destructor
  virtual ~csSoftwareGraphics3D ();
  /// Initialize iComponent.
  virtual bool Initialize (iObjectRegistry*);
  /// Open a canvas.
  virtual bool Open ();

  // --------------------------- iPluginConfig ------------------------------
  virtual bool GetOptionDescription (int idx, csOptionDescription*);
  virtual bool SetOption (int id, csVariant* value);
  virtual bool GetOption (int id, csVariant* value);
};

} // namespace cspluginSoft3d

#endif // __CS_SOFT_RENDER3D_H__
