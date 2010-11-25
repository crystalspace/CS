/*
    Copyright (C) 2010 by Frank Richter
	      (C) 2010 by Mike Gist

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
#include "csplugincommon/rendermanager/viscullcommon.h"

#include "csplugincommon/rendermanager/occluvis.h"
#include "csutil/cfgacc.h"

namespace CS
{
  namespace RenderManager
  {
    RMViscullCommon::RMViscullCommon() : occluvisEnabled (false), objReg (nullptr)
    {
    }
  
    void RMViscullCommon::Initialize (iObjectRegistry* objReg, const char* prefix)
    {
      this->objReg = objReg;
      
      csConfigAccess cfg (objReg);
      csString cfgkey (prefix);
      cfgkey.Append (".OcclusionCulling");
      occluvisEnabled = cfg->GetBool (cfgkey, true);
    }
  
    csPtr<iVisibilityCuller> RMViscullCommon::GetVisCuller ()
    {
      if (!occluvisEnabled) return (iVisibilityCuller*)nullptr;
      
      csRef<iVisibilityCuller> psVisCuller;
      psVisCuller.AttachNew (new csOccluvis (objReg));
      return csPtr<iVisibilityCuller> (psVisCuller);
    }
  } // namespace RenderManager
} // namespace CS
