/*
    Copyright (C) 2003-2006 by Jorrit Tyberghein
	      (C) 2003-2006 by Frank Richter

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

#ifndef __CS_PTPDLIGHT_H__
#define __CS_PTPDLIGHT_H__

#include "csgeom/csrect.h"
#include "csgfx/imageautoconvert.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"
#include "cstool/proctex.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "imap/reader.h"
#include "imesh/lighting.h"
#include "igraphic/image.h"

class csProcTexture;

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

class ProctexPDLightLoader :
  public scfImplementation2<ProctexPDLightLoader,
                            iLoaderPlugin, 
                            iComponent>
{
protected:
  iObjectRegistry* object_reg;

  void Report (int severity, iDocumentNode* node, const char* msg, ...);

public:
  ProctexPDLightLoader (iBase *p);
  virtual ~ProctexPDLightLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource*, iLoaderContext* ldr_context,
  	iBase* context);
};  

class ProctexPDLight : 
  public scfImplementationExt1<ProctexPDLight, 
                               csProcTexture,
                               iLightingInfo>
{
public:
  class PDMap
  {
    friend class ProctexPDLight;
    void ComputeValueBounds ();
    void ComputeValueBounds (const csRect& area);
  public:
    csRef<iImage> image;
    csRGBcolor maxValue;
    csRect nonNullArea;

    PDMap () { ComputeValueBounds (); }
    PDMap (iImage* img) : image (img) { ComputeValueBounds (); }

    iImage* operator->() const { return image; }
    operator iImage*() const { return image; }
    PDMap& operator=(iImage* image)
    {
      this->image = CS::ImageAutoConvert (image, CS_IMGFMT_TRUECOLOR);
      ComputeValueBounds();
      return *this;
    }
  };
  struct MappedLight
  {
    PDMap map;
    csString* lightId;
    csWeakRef<iLight> light;
  };
private:
  typedef csDirtyAccessArray<csRGBpixel> LightmapScratch;
  CS_DECLARE_STATIC_CLASSVAR_REF(lightmapScratch, GetScratch, LightmapScratch);
  size_t lightmapSize;

  PDMap baseMap;
  csArray<MappedLight> lights;
  csRect totalAffectedAreas;
  enum
  {
    stateAffectedAreaDirty = 1 << 0,
    stateDirty = 1 << 1,
    statePrepared = 1 << 2,
  };
  csFlags state;

  void Report (int severity, const char* msg, ...);
  bool HexToLightID (char* lightID, const csString& lightIDHex);
  void UpdateAffectedArea ();
public:
  const char* AddLight (const MappedLight& light);

  ProctexPDLight (iImage* img);
  virtual ~ProctexPDLight ();

  virtual bool PrepareAnim ();

  virtual void Animate (csTicks /*current_time*/);

  /**\name iLightingInfo implementation
   * @{ */
  void DisconnectAllLights ()
  { 
    lights.DeleteAll(); 
    state.Set (stateAffectedAreaDirty);
  }
  void InitializeDefault (bool /*clear*/) {}
  void LightChanged (iLight* /*light*/) { state.Set (stateDirty); }
  void LightDisconnect (iLight* light);
  void PrepareLighting () {}
  bool ReadFromCache (iCacheManager* /*cache_mgr*/) { return true; }
  bool WriteToCache (iCacheManager* /*cache_mgr*/) { return true; }
  /** @} */
};

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)

#endif // __CS_PTPDLIGHT_H__
