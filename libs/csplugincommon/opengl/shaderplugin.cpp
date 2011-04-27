/*
    Copyright (C) 2008 by Frank Richter

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

#include "csplugincommon/opengl/shaderplugin.h"
#include "csutil/objreg.h"
#include "csutil/stringarray.h"

#include "iutil/verbositymanager.h"
#include "ivideo/graph2d.h"

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

namespace CS
{
  namespace PluginCommon
  {
  
    ShaderProgramPluginGL::ShaderProgramPluginGL (iBase* parent)
     : scfImplementationType (this, parent), vendor (Invalid), isOpen (false),
       object_reg (0), ext (0), doVerbose (false)
    {
    }
  
    bool ShaderProgramPluginGL::Initialize (iObjectRegistry* objectReg)
    {
      object_reg = objectReg;
    
      csRef<iVerbosityManager> verbosemgr (
	csQueryRegistry<iVerbosityManager> (object_reg));
      if (verbosemgr) 
      {
	doVerbose = verbosemgr->Enabled ("renderer.shader");
	doVerbosePrecache = verbosemgr->Enabled ("renderer.shader.precache");
      }
      else
	doVerbose = doVerbosePrecache = false;
	
      return true;
    }
    
    bool ShaderProgramPluginGL::Open()
    {
      if (isOpen) return true;
      isOpen = true;
      
      csRef<iGraphics3D> r = csQueryRegistry<iGraphics3D> (object_reg);
    
      // Sanity check
      csRef<iFactory> f = scfQueryInterfaceSafe<iFactory> (r);
      if (f == 0 || strcmp ("crystalspace.graphics3d.opengl", 
	    f->QueryClassID ()) != 0)
	return false;
    
      ext = 0;
      statecache = 0;
      if (r)
      {
	r->GetDriver2D()->PerformExtension ("getstatecache", &statecache);
	r->GetDriver2D()->PerformExtension ("getextmanager", &ext);
      }
      if ((ext == 0) || (statecache == 0))
	return false;
	
      csString vendorStr ((const char*)glGetString (GL_VENDOR));
      vendorStr.Downcase();
      if (vendorStr.FindStr ("nvidia") != (size_t)-1)
      {
        vendor = NVIDIA;
      }
      else if ((vendorStr.FindStr ("ati") != (size_t)-1)
          || (vendorStr.FindStr ("amd") != (size_t)-1))
      {
        vendor = ATI;
      }
      else
      {
        vendor = Other;
      }
      
      clipPlanes.Initialize (object_reg);
      
      return true;
    }
    
    void ShaderProgramPluginGL::Close ()
    {
      isOpen = false;
    }
    
    const char* ShaderProgramPluginGL::VendorToString (HardwareVendor vendor)
    {
      switch (vendor)
      {
        case Invalid: return 0;
        case Other: return "other";
        case ATI: return "ati";
        case NVIDIA: return "nv";
      }
      CS_ASSERT_MSG("Forgot to add vendor string when adding vendor enum?",
        false);
      return 0;
    }
    
    ShaderProgramPluginGL::HardwareVendor ShaderProgramPluginGL::VendorFromString (
      const char* vendorStr)
    {
      if (vendorStr == 0) return Invalid;
    
      csString str (vendorStr);
      str.Downcase();
      
      if (str == "ati")
      {
	return ATI;
      }
      else if ((str == "nvidia") || (str == "nv"))
      {
	return NVIDIA;
      }
      else if (str == "other")
      {
	return Other;
      }
      return Invalid;
    }
      
    uint ShaderProgramPluginGL::ParseVendorMask (const char* maskStr)
    {
      uint mask = 0;
      csStringArray maskSplit;
      maskSplit.SplitString (maskStr, ",");
      
      for (size_t i = 0; i < maskSplit.GetSize(); i++)
      {
        csString str = maskSplit[i];
        if (str.IsEmpty()) continue;
        
        bool doNegate = false;
        if (str.GetAt(0) == '!')
        {
          doNegate = true;
          str.DeleteAt (0, 1);
          if ((i == 0) && (mask == 0)) mask = ~0;
        }
        if (str.IsEmpty()) continue;
        
        uint thisMask = 0;
        if (str == "*")
          thisMask = ~0;
        else
        {
          HardwareVendor thisVendor = VendorFromString (str);
          if (thisVendor != Invalid)
            thisMask = 1 << thisVendor;
        }
        if (doNegate)
          mask &= ~thisMask;
        else
          mask |= thisMask;
      }
      return mask;
    }
    
    //-----------------------------------------------------------------------
  
    ShaderProgramPluginGL::ClipPlanes::ClipPlanes () : currentPlanes (0)
    {
    }
    
    ShaderProgramPluginGL::ClipPlanes::~ClipPlanes ()
    {
    }
  
    void ShaderProgramPluginGL::ClipPlanes::Initialize (iObjectRegistry* objectReg)
    {
      GLint _maxClipPlanes;
      glGetIntegerv (GL_MAX_CLIP_PLANES, &_maxClipPlanes);
      maxPlanes = csMin ((int)_maxClipPlanes, 6);
      // @@@ Lots of places assume max 6 planes
      
      csRef<iShaderVarStringSet> strings = 
        csQueryRegistryTagInterface<iShaderVarStringSet> (
	  objectReg, "crystalspace.shader.variablenameset");
      
      svObjectToWorldInv = strings->Request ("object2world transform inverse");
      svWorldToCamera = strings->Request ("world2camera transform");
    }
    
    void ShaderProgramPluginGL::ClipPlanes::SetShaderVars (const csShaderVariableStack& stack)
    {
      if (stack.GetSize() > svObjectToWorldInv)
      {
        csShaderVariable* sv = stack[svObjectToWorldInv];
        if (sv) sv->GetValue (worldToObject);
      }
      if (stack.GetSize() > svWorldToCamera)
      {
        csShaderVariable* sv = stack[svWorldToCamera];
        if (sv) sv->GetValue (worldToCam);
      }
      eyeToObjectDirty = true;
    }
    
    bool ShaderProgramPluginGL::ClipPlanes::AddClipPlane (const csPlane3& plane, 
                                                          ClipSpace space)
    {
      unsigned long nextPlane;
      if (!CS::Utility::BitOps::ScanBitForward (~currentPlanes, nextPlane))
        return false;
      if (nextPlane >= maxPlanes) return false;
      
      csPlane3 planeTF;
      switch (space)
      {
        case Object:
          planeTF = plane;
          break;
        case Eye:
          {
            if (eyeToObjectDirty)
            {
	      eyeToObject = worldToObject.GetInverse() * worldToCam;
              eyeToObjectDirty = false;
            }
            planeTF = eyeToObject.Other2This (plane);
          }
          break;
        case World:
          planeTF = worldToObject.Other2This (plane);
          break;
      }
      
      glEnable (GL_CLIP_PLANE0 + nextPlane);
      GLdouble glPlane[4] = { planeTF.A(), planeTF.B(), planeTF.C(), planeTF.D() };
      glClipPlane (GL_CLIP_PLANE0 + nextPlane, glPlane);
      
      currentPlanes |= 1 << nextPlane;
      return true;
    }
    
    bool ShaderProgramPluginGL::ClipPlanes::EnableClipPlane (size_t n)
    {
      if (n >= maxPlanes) return false;
      
      glEnable (GL_CLIP_PLANE0 + (GLenum)n);
      currentPlanes |= 1 << n;
      return true;
    }
    
    bool ShaderProgramPluginGL::ClipPlanes::EnableNextClipPlane ()
    {
      unsigned long nextPlane;
      if (!CS::Utility::BitOps::ScanBitForward (~currentPlanes, nextPlane))
        return false;
      if (nextPlane >= maxPlanes) return false;
      
      glEnable (GL_CLIP_PLANE0 + nextPlane);
      
      currentPlanes |= 1 << nextPlane;
      return true;
    }
    
    void ShaderProgramPluginGL::ClipPlanes::DisableClipPlanes ()
    {
      for (size_t i = 0; i < maxPlanes; i++)
      {
        if (currentPlanes & (1 << i)) glDisable (GL_CLIP_PLANE0 + (GLenum)i);
      }
      currentPlanes = 0;
    }
    
  } // namespace PluginCommon
} // namespace CS
