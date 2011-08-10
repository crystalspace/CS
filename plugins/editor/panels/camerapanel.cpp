/*
    Copyright (C) 2011 by Jelle Hellemans

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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include <iengine/camera.h>
#include <iengine/sector.h>
#include <iutil/object.h>

#include "ieditor/space.h"
#include "ieditor/layout.h"
#include "ieditor/context.h"

#include "camerapanel.h"

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
  
SCF_IMPLEMENT_FACTORY (CameraPanel)

CameraPanel::CameraPanel (iBase* parent) : scfImplementationType (this, parent)
{
}

CameraPanel::~CameraPanel ()
{
}

bool CameraPanel::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  
  csRef<iSpaceManager> mgr = csQueryRegistry<iSpaceManager> (object_reg);
  return mgr->Register(this);
}

bool CameraPanel::Poll(iContext* ctx)
{
  return ctx && ctx->GetCamera();
}

void CameraPanel::Draw(iContext* ctx, iLayout* layout)
{
  printf("CameraPanel::Draw\n");
  
  csString ori;
  const csVector3 & origin = ctx->GetCamera()->GetTransform().GetOrigin();
  ori.Format("Origin: X:%f Y:%f Z:%f", origin[0], origin[1], origin[2]);
  layout->AppendLabel(ori.GetData());
  if (ctx->GetCamera()->GetSector())
  {
    csString sect;
    sect.Format("Sector: %s", ctx->GetCamera()->GetSector()->QueryObject()->GetName());
    layout->AppendLabel(sect.GetData());
  }
}

void CameraPanel::Prepend(iLayoutExtension*)
{
}

void CameraPanel::Append(iLayoutExtension*)
{
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
