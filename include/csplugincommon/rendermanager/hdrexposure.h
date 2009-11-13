/*
    Copyright (C) 2008-2009 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_H__

#include "csgfx/textureformatstrings.h"
#include "csplugincommon/rendermanager/hdrexposure_luminance.h"
#include "csplugincommon/rendermanager/hdrhelper.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csutil/ref.h"

/**\file
 * HDR exposure controllers
 */

class csShaderVariable;
struct iDataBuffer;
struct iObjectRegistry;

namespace CS
{
  namespace RenderManager
  {
    namespace HDR
    {
      namespace Exposure
      {
	/**
	 * A simple exposure controller, just scaling color values by a factor.
	 * For the rendered image the average luminance is computed. If it's higher
	 * than a given target average luminance (plus a tolerance) the image is
	 * dimmed; if it's darker, the image is brightened up.
	 */
	class CS_CRYSTALSPACE_EXPORT Linear
	{
	  csRef<csShaderVariable> svHDRScale;
	  HDRHelper* hdr;
	  
	  csTicks lastTime;
	  
	  float targetAvgLum;
	  float targetAvgLumTolerance;
	  float minExposure, maxExposure;
	  float exposureChangeRate;
	  
	  Luminance::Average luminance;
	public:
	  Linear () : hdr (0),
	    lastTime (0), targetAvgLum (0.8f), targetAvgLumTolerance (0.1f),
	    minExposure (0.1f), maxExposure (10.0f), exposureChangeRate (0.5f)
	  {}
	
	  /// Set up HDR exposure control for a post effects manager
	  void Initialize (iObjectRegistry* objReg,
	    HDRHelper& hdr);
	  
	  /// Obtain rendered image and apply exposure correction
	  void ApplyExposure (RenderTreeBase& renderTree, iView* view);
	  
	  /// Set target average luminance
	  void SetTargetAverageLuminance (float f) { targetAvgLum = f; }
	  /// Get target average luminance
	  float GetTargetAverageLuminance () const { return targetAvgLum; }
	  
	  /// Set target average luminance tolerance
	  void SetTargetAverageLuminanceTolerance (float f)
	  { targetAvgLumTolerance = f; }
	  /// Get target average luminance tolerance
	  float GetTargetAverageLuminanceTolerance () const
	  { return targetAvgLumTolerance; }
	  
	  /// Set minimum and maximum exposure
	  void SetMinMaxExposure (float min, float max)
	  { minExposure = min; maxExposure = max; }
	  /// Get minimum and maximum exposure
	  void GetMinMaxExposure (float& min, float& max) const
	  { min = minExposure; max = maxExposure; }
	
	  /// Set exposure change rate
	  void SetExposureChangeRate (float f) { exposureChangeRate = f; }
	  /// Get exposure change rate
	  float GetExposureChangeRate () const { return exposureChangeRate; }
	};
	
	class CS_CRYSTALSPACE_EXPORT Reinhard_Simple
	{
	  csRef<csShaderVariable> svHDRScale;
	  csRef<csShaderVariable> svMappingParams;
	  HDRHelper* hdr;
	  
	  csTicks lastTime;
	  
	  Luminance::LogAverage luminance;
	public:
	  Reinhard_Simple() : lastTime (0) {}
	
	  /// Set up HDR exposure control for a post effects manager
	  void Initialize (iObjectRegistry* objReg,
	    HDRHelper& hdr);
	  
	  /// Obtain rendered image and apply exposure correction
	  void ApplyExposure (RenderTreeBase& renderTree, iView* view);
	};
  
        /**
         * Exposure controller wrapping other exposure controllers, allowing
         * the choice of exposure through the configuration system.
         */
        class CS_CRYSTALSPACE_EXPORT Configurable
        {
        protected:
          struct AbstractExposure : public CS::Memory::CustomAllocated
          {
            virtual ~AbstractExposure() {}
            
            virtual void Initialize (iObjectRegistry* objReg,
	      HDRHelper& hdr) = 0;
	    virtual void ApplyExposure (RenderTreeBase& renderTree, iView* view) = 0;
          };
          
          template<typename T>
          struct WrapperExposure : public AbstractExposure
          {
            T exposure;
            
            virtual void Initialize (iObjectRegistry* objReg,
	      HDRHelper& hdr)
	    {
	      exposure.Initialize (objReg, hdr);
	    }
	    
	    virtual void ApplyExposure (RenderTreeBase& renderTree, iView* view)
	    {
	      exposure.ApplyExposure (renderTree, view);
	    }
          };
          
          virtual AbstractExposure* CreateExposure (const char* name);
          
          AbstractExposure* exposure;
        public:
          Configurable() : exposure (0) {}
          virtual ~Configurable();
        
	  void Initialize (iObjectRegistry* objReg,
	    HDRHelper& hdr, const HDRSettings& settings);
	  void ApplyExposure (RenderTreeBase& renderTree, iView* view);
        };
  
      } // namespace Exposure
    } // namespace HDR
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_H__
