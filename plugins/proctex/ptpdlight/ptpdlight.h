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
  bool HexToLightID (char* lightID, const char* lightIDHex);
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
  struct Lumel
  {
    uint8 blue, green, red, alpha;

    void UnsafeAdd (int R, int G, int B)
    {
      red   = (unsigned char)(red   + R);
      green = (unsigned char)(green + G);
      blue  = (unsigned char)(blue  + B);
    }
    void SafeAdd (int R, int G, int B)
    {
      int color = red + R;
      red   = (unsigned char)(color > 255 ? 255 : color);
      color = green + G;
      green = (unsigned char)(color > 255 ? 255 : color);
      color = blue + B;
      blue  = (unsigned char)(color > 255 ? 255 : color);
    }
  };
  struct LumelBuffer : public csRefCount
  {
    static CS_FORCEINLINE size_t LumelAlign (size_t n)
    {
      static const size_t align = sizeof (Lumel);
      return ((n + align - 1) / align) * align;
    }
  public:
    CS_FORCEINLINE Lumel* GetData () 
    { 
      return reinterpret_cast<Lumel*> (
        (reinterpret_cast<uint8*> (this)) + LumelAlign (sizeof (*this))); 
    }
    
    inline void* operator new (size_t n, size_t lumels)
    { 
      CS_ASSERT (n == sizeof (LumelBuffer));
      size_t allocSize = 
        LumelAlign (sizeof (LumelBuffer)) + lumels * sizeof (Lumel);
      return cs_malloc (allocSize);
    }
    inline void operator delete (void* p, size_t lumels) 
    {
      cs_free (p);
    }
    inline void operator delete (void* p) 
    {
      cs_free (p);
    }

  };
  class PDMap
  {
    friend class ProctexPDLight;
    void ComputeValueBounds ();
    void ComputeValueBounds (const csRect& area);

    void SetImage (iImage* img);
  public:
    csRGBcolor maxValue;
    csRect nonNullArea;
    int imageW, imageH;
    csRef<LumelBuffer> imageData;

    PDMap () : imageW (0), imageH (0), imageData (0) { ComputeValueBounds (); }
    PDMap (iImage* img) : imageData (0)
    { SetImage (img); }

    PDMap& operator=(iImage* image)
    {
      SetImage (image);
      return *this;
    }
  };
  struct MappedLight
  {
    PDMap map;
    char* lightId;
    csWeakRef<iLight> light;

    MappedLight() : lightId (0) {}
    MappedLight (const MappedLight& other)
    {
      map = other.map;
      if (other.lightId != 0)
      {
        lightId = new char[16];
        memcpy (lightId, other.lightId, 16);
      }
      else
        lightId = 0;
      light = other.light;
    }
    ~MappedLight() { delete[] lightId; }
  };
private:
  typedef csDirtyAccessArray<Lumel> LightmapScratch;
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
  void UpdateAffectedArea ();
public:
  const char* AddLight (const MappedLight& light);
  void FinishLoad()
  {
    lights.ShrinkBestFit();
  }

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

  virtual void UseTexture (iTextureWrapper*)
  { 
    if (!PrepareAnim ()) return;
    Animate (0);
  }
};

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)

#endif // __CS_PTPDLIGHT_H__
