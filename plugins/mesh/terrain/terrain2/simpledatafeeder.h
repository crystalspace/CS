/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#ifndef __CS_TERRAIN_SIMPLEDATAFEEDER_H__
#define __CS_TERRAIN_SIMPLEDATAFEEDER_H__

#include "csutil/scf_implementation.h"
#include "csutil/csstring.h"

#include "imesh/terrain2.h"
#include "iutil/comp.h"
#include "iengine/engine.h"
#include "imap/loader.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
struct csTerrainSimpleDataFeederProperties
  : public scfImplementation1<csTerrainSimpleDataFeederProperties,
                              iTerrainCellFeederProperties>
{
  csTerrainSimpleDataFeederProperties ();
  csTerrainSimpleDataFeederProperties (csTerrainSimpleDataFeederProperties& other);

  // ---- iTerrainCellFeederProperties ----
  virtual void SetHeightmapSource (const char* source, const char* format);
  virtual void SetNormalMapSource (const char* source);
  virtual void SetMaterialMapSource (const char* source);
  virtual void SetHeightOffset (float offset);
  virtual void AddAlphaMap (const char* material, const char* alphaMapSource);
  virtual void SetParameter (const char* param, const char* value);
  virtual size_t GetParameterCount();
  virtual const char* GetParameterName (size_t index);
  virtual const char* GetParameterValue (size_t index);
  virtual const char* GetParameterValue (const char* name);
  virtual csPtr<iTerrainCellFeederProperties> Clone ();

  virtual size_t GetAlphaMapCount() { return alphaMaps.GetSize(); }

  virtual const char* GetAlphaMapMaterial (size_t index)
  { return alphaMaps[index].material; }

  virtual const char* GetAlphaMapSource (size_t index)
  { return alphaMaps[index].alphaSource; }
  virtual const char* GetAlphaMapSource (const char* material);

  virtual void SetHeightmapSmooth (bool doSmooth);  
  virtual bool GetHeightmapSmooth () const;
  
  csString heightmapSource, heightmapFormat, normalmapSource, materialmapSource;

  struct AlphaPair
  {
    csString material;
    csString alphaSource;
  };
  csArray<AlphaPair> alphaMaps;

  float heightOffset;

  bool smoothHeightmap;
};

class csTerrainSimpleDataFeeder :
  public scfImplementation2<csTerrainSimpleDataFeeder,
                            iTerrainDataFeeder,
                            iComponent>
{
public:
  csTerrainSimpleDataFeeder (iBase* parent);

  virtual ~csTerrainSimpleDataFeeder ();

  // ------------ iTerrainDataFeeder implementation ------------
  virtual csPtr<iTerrainCellFeederProperties> CreateProperties ();

  virtual bool PreLoad (iTerrainCell* cell);

  virtual bool Load (iTerrainCell* cell);

  virtual void SetParameter (const char* param, const char* value);

  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg);

protected:
  iObjectRegistry* objectReg;
  csRef<iLoader> loader;
  csRef<iEngine> engine;
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_SIMPLEDATAFEEDER_H__
