/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_SOFT_G3D_H__
#define __CS_SOFT_G3D_H__

#include "iutil/iconfig.h"
#include "sft3dcom.h"

/// Software 3D renderer
class csGraphics3DSoftware : public csGraphics3DSoftwareCommon
{
public:
  DECLARE_IBASE;
  /// Constructor
  csGraphics3DSoftware (iBase *iParent);
  /// Destructor
  virtual ~csGraphics3DSoftware ();
  /// Initialize
  virtual bool Initialize (iSystem *iSys);
  /// Open
  virtual bool Open (const char *Title);

  ///------------------- iConfig interface implementation -------------------
  struct csSoftConfig : public iConfig
  {
    DECLARE_EMBEDDED_IBASE (csGraphics3DSoftware);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csSoftConfig;
};

#endif // __CS_SOFT_G3D_H__
