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

#ifndef __CS_CSPLUGINCOMMON_OPENGL_SHADERPLUGIN_H__
#define __CS_CSPLUGINCOMMON_OPENGL_SHADERPLUGIN_H__

#include "csextern_gl.h"

#include "csgeom/plane3.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/scf_implementation.h"

class csGLExtensionManager;

namespace CS
{
  namespace PluginCommon
  {
    /**
     * Common code useful for all OpenGL-based shader program plugins.
     */
    class CS_CSPLUGINCOMMON_GL_EXPORT ShaderProgramPluginGL :
      public scfImplementation1<ShaderProgramPluginGL, iShaderProgramPlugin>
    {
    public:
      enum HardwareVendor
      {
        ATI = 0, NVIDIA = 1, Other = 2
      };
    protected:
      /**
       * Vendor of the current graphics hardware/driver.
       * Set after Open().
       */
      HardwareVendor vendor;
      
      ShaderProgramPluginGL (iBase* parent);
      
      /// Initialize program plugin common stuff.
      bool Initialize (iObjectRegistry* objectReg);
      bool Open();
      void Close ();
      
    public:
      uint ParseVendorMask (const char* mask);
      
    protected:
      /// Whether the program plugin was openend
      bool isOpen;
      /// Object registry
      iObjectRegistry* object_reg;
      /// GL extension manager
      csGLExtensionManager* ext;
      /// Whether verbose reporting was enabled.
      bool doVerbose;
      
    public:
      class CS_CSPLUGINCOMMON_GL_EXPORT ClipPlanes
      {
        int maxPlanes;
        uint32 currentPlanes;
        
        CS::ShaderVarStringID svObjectToWorldInv;
        CS::ShaderVarStringID svWorldToCamera;
        
        csReversibleTransform worldToObject;
        csReversibleTransform worldToCam;
        bool eyeToObjectDirty;
        csTransform eyeToObject;
      public:
        ClipPlanes ();
        ~ClipPlanes ();
        
        void Initialize (iObjectRegistry* objectReg);
      
        void SetShaderVars (const csShaderVariableStack& stack);
        enum ClipSpace
        {
          Object, World, Eye
        };
        bool AddClipPlane (const csPlane3& plane, ClipSpace space);
        bool AddClipPlane (const csVector4& plane, ClipSpace space)
        { return AddClipPlane (csPlane3 (plane.x, plane.y, plane.z, plane.w), space); }
        bool EnableClipPlane (int n);
        bool EnableNextClipPlane ();
        void DisableClipPlanes ();
      };
      /// Helper for user-defined clip planes
      ClipPlanes clipPlanes;
    };
  } // namespace PluginCommon
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_OPENGL_SHADERPLUGIN_H__
