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

/**\file
 * Common code useful for all OpenGL-based shader program plugins.
 */

#include "csextern_gl.h"

#include "csgeom/plane3.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/scf_implementation.h"

struct csGLExtensionManager;
class csGLStateCache;

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
      /// Known vendors of graphics hardware/drivers.
      enum HardwareVendor
      {
        /// No vendor identified yet
        Invalid = -1,
        /// Other vendor
        Other = 0,
        /// ATI/AMD
        ATI = 1,
        /// NVidia
        NVIDIA = 2
      };
      /// Give a string representation from a vendor value
      static const char* VendorToString (HardwareVendor vendor);
      /// Convert a to a vendor value
      static HardwareVendor VendorFromString (const char* vendorStr);
    protected:
      /**
       * Vendor of the current graphics hardware/driver.
       * Set after Open().
       */
      HardwareVendor vendor;
      
      /// Construct.
      ShaderProgramPluginGL (iBase* parent);
      
      /// Initialize program plugin common stuff.
      bool Initialize (iObjectRegistry* objectReg);
      /// Open the common part. Returns whether opened successful.
      bool Open();
      /// Close the common part.
      void Close ();
      
    public:
      /**
       * Parse a string into a set of vendors.
       * \param mask Mask string. It can be a single vendor string 
       *   (<tt>ati</tt>, <tt>nvidia</tt>, <tt>other</tt>) causing this
       *   vendor to be included in the mask, the special string <tt>*</tt>
       *   including all vendors in the mask, a vendor string prefixed
       *   by <tt>!</tt> excluding that vendor from the mask or a combination
       *   of these, separated by <tt>,</tt>. If the first sub-string is an
       *   exclusion mask string an <tt>*</tt> is implicitly inserted before
       *   the first sub-string.
       * \return A set of vendors encoded as a combination of bits.. 
       *   If a bit <tt>1 << (vendor)</tt> is set, then that vendor is
       *   included in the set, otherwise not.
       */
      uint ParseVendorMask (const char* mask);
      
    protected:
      /// Whether the program plugin was openend
      bool isOpen;
      /// Object registry
      iObjectRegistry* object_reg;
      /// GL extension manager
      csGLExtensionManager* ext;
      /// GL state cache
      csGLStateCache* statecache;
      /// Whether verbose reporting was enabled.
      bool doVerbose;
      /**
       * Whether "precache" verbose reporting was enabled.
       * This should report progress on precaching, but not necessarily
       * everything during shader precaching (e.g. compile errors).
       */
      bool doVerbosePrecache;
      
    public:
      /// Helper class for user defined OpenGL clip planes.
      class CS_CSPLUGINCOMMON_GL_EXPORT ClipPlanes
      {
        size_t maxPlanes;
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
        
        /// Initialize, should be called by plugin Initialize().
        void Initialize (iObjectRegistry* objectReg);
      
        /// Set shader vars required for transforming clip planes.
        void SetShaderVars (const csShaderVariableStack& stack);
        /// Possible clip plane spaces
        enum ClipSpace
        {
          /// Object space
          Object,
          /// World space
          World,
          /// Eye space (pre-projection camera space)
          Eye
        };
        //@{
        /// Add a clip plane.
        bool AddClipPlane (const csPlane3& plane, ClipSpace space);
        bool AddClipPlane (const csVector4& plane, ClipSpace space)
        { return AddClipPlane (csPlane3 (plane.x, plane.y, plane.z, plane.w), space); }
        //@}
        /**
         * Enable the \a n th OpenGL clip plane without setting it. It is up 
         * to the caller to call glClipPlane().
         */
        bool EnableClipPlane (size_t n);
        /**
         * Enable the next available OpenGL clip plane without setting it. 
         * It is up  to the caller to call glClipPlane().
         */
        bool EnableNextClipPlane ();
        /// Disable all clip planes previously enabled.
        void DisableClipPlanes ();
      };
      /// Helper for user-defined clip planes
      ClipPlanes clipPlanes;
    };
  } // namespace PluginCommon
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_OPENGL_SHADERPLUGIN_H__
