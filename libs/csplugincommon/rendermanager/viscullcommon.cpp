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

#include "csplugincommon/rendermanager/occluvis.h"
#include "csplugincommon/rendermanager/viscullcommon.h"
#include "csutil/cfgacc.h"
#include "imap/loader.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"

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

      // Load the default occlusion shader.
      if (occluvisEnabled)
      {
        cfgkey = prefix;
        cfgkey.Append (".DefaultOcclusionShaderPath");
        const char* defaultShaderPath = cfg->GetStr (cfgkey, "/shader/early_z/z_only.xml");

        csRef<iVFS> vfs = csQueryRegistry<iVFS> (objReg);
        csRef<iThreadedLoader> loader = csQueryRegistry<iThreadedLoader> (objReg);
        loader->LoadShaderWait (vfs->GetCwd (), defaultShaderPath);

        cfgkey = prefix;
        cfgkey.Append (".DefaultOcclusionShaderName");
        defaultOccluvisShaderName = cfg->GetStr (cfgkey, "z_only");
      }
    }
  
    csPtr<iVisibilityCuller> RMViscullCommon::GetVisCuller ()
    {
      if (!occluvisEnabled) return (iVisibilityCuller*)nullptr;
      
      csRef<iVisibilityCuller> psVisCuller;
      psVisCuller.AttachNew (new csOccluvis (objReg));
      psVisCuller->Setup (defaultOccluvisShaderName);
      return csPtr<iVisibilityCuller> (psVisCuller);
    }
  } // namespace RenderManager
} // namespace CS
