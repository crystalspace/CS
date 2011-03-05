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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_VISCULLCOMMON_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_VISCULLCOMMON_H__

/**\file
 * Common iRenderManagerVisCull implementation.
 */

#include "iengine/rendermanager.h"

namespace CS
{
  namespace RenderManager
  {
    class CS_CRYSTALSPACE_EXPORT RMViscullCommon :
      public virtual iRenderManagerVisCull
    {
    protected:
      bool occluvisEnabled;
      csString defaultOccluvisShaderName;
      iObjectRegistry* objReg;
    public:
      RMViscullCommon();
    
      /// Read configuration settings
      void Initialize (iObjectRegistry* objReg, const char* prefix);
    
      /**\name iRenderManagerVisCull implementation
      * @{ */
      virtual csPtr<iVisibilityCuller> GetVisCuller ();
      /** @} */
    };
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_VISCULLCOMMON_H__
