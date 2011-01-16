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

#include "common.h"

#include "scene.h"
#include "object_terrain2.h"

namespace lighter
{
  Object_Terrain2::Object_Terrain2 (ObjectFactory_Terrain2* factory) : 
    Object (factory)
  {
    saverPluginName = "crystalspace.mesh.saver.terrain2";
  }

  bool Object_Terrain2::Initialize (Sector* sector)
  {
    if (!factory || !meshWrapper) return false;

    this->sector = sector;

    const csReversibleTransform transform = meshWrapper->GetMovable ()->
      GetFullTransform ();

    //Copy over data, transform the radprimitives..
    vertexData.Transform (transform);
    primVertexData.Transform (transform);
    if (objFlags.Check (OBJECT_FLAG_TANGENTS))
    {
      for(size_t i = 0; i < vertexData.positions.GetSize (); ++i)
      {
        csVector3& tang = 
          *((csVector3*)vertexData.GetCustomData (i, vdataTangents));
        tang = transform.This2OtherRelative (tang);
        csVector3& bitang = 
          *((csVector3*)vertexData.GetCustomData (i, vdataBitangents));
        bitang = transform.This2OtherRelative (bitang);
      }
      
    }
    ComputeBoundingSphere ();

    return true;
  }

  void Object_Terrain2::ParseMesh (iMeshWrapper *wrapper)
  {
    csRef<iTerrainSystem> terrSys = scfQueryInterface<iTerrainSystem> (
      wrapper->GetMeshObject());
    if (!terrSys.IsValid()) return;

    Object::ParseMesh (wrapper);

    /* Abuse per-vertex lighting for terrain. Instead of tesselating
       terrain into a lot of small primitives (somewhat slow) compute lighting
       at the points on the terrain that correspond to lightmap pixels.
       So set up a "vertex" for each of these points and use PVL.
       Later, the data is copied into a lightmap image. */
    lightPerVertex = true;

    if (factory->hasTangents)
    {
      objFlags.Set (OBJECT_FLAG_TANGENTS);
      vdataBitangents = factory->vdataBitangents;
      vdataTangents = factory->vdataTangents;
    }
    vertexData = factory->GetVertexData();

    size_t numCells = terrSys->GetCellCount();
    for (size_t c = 0; c < numCells; c++)
    {
      iTerrainCell* cell = terrSys->GetCell (c, true);

      const csVector3& cellSize (cell->GetSize ());
      const csVector2& cellPos (cell->GetPosition ());

      uint lmSamplesX = csMin ((uint)csFindNearestPowerOf2 (
        int (cellSize.x * factory->GetLMDensity())),
        globalConfig.GetTerrainProperties().maxLightmapU);
      uint lmSamplesY = csMin ((uint)csFindNearestPowerOf2 (
        int (cellSize.z * factory->GetLMDensity())),
        globalConfig.GetTerrainProperties().maxLightmapV);

      {
	uint primSamplesX = cell->GetGridWidth();
	uint primSamplesY = cell->GetGridHeight();
	size_t numCellVerts = primSamplesX * primSamplesY;
	uint indexOffs = (uint)primVertexData.positions.GetSize();
	primVertexData.positions.SetCapacity (indexOffs + numCellVerts);
	primVertexData.uvs.SetCapacity (primVertexData.uvs.GetSize()
	  + numCellVerts);
	// @@@ FIXME: normals needed?
	primVertexData.normals.SetCapacity (primVertexData.normals.GetSize()
	  + numCellVerts);

	float invSamplesX = 1.0f/(primSamplesX-1);
	float invSamplesY = 1.0f/(primSamplesY-1);

	float posScaleX = cellSize.x * invSamplesX;
	float posScaleY = cellSize.z * invSamplesY;

	for (uint y = 0; y < primSamplesY; y++)
	{
	  for (uint x = 0; x < primSamplesX; x++)
	  {
	    // Bit of a hack to avoid terrain self-shadowing acne
	    float height = FLT_MAX;
	    for (int dx = -1; dx < 2; dx++)
	    {
	      if (int (x) + dx < 0) continue;
	      if (x + dx >= primSamplesX) continue;
	      for (int dy = -1; dy < 2; dy++)
	      {
		if (int (y) + dy < 0) continue;
		if (y + dy >= primSamplesX) continue;
		csVector2 p (x * posScaleX, y * posScaleY);
		
		height = csMin (height, cell->GetHeight (x+dx, y+dy) - 0.01f);
	      }
	    }
	  
	    csVector2 p (x * posScaleX, y * posScaleY);

	    //float height = cell->GetHeight (x, y) - EPSILON;
	    csVector3 norm (cell->GetNormal (x, y));
	    csVector2 uv (p.x * invSamplesX, p.y * invSamplesY);

	    primVertexData.positions.Push (csVector3 (
	      p.x + cellPos.x, height, cellSize.z - p.y + cellPos.y));
	    primVertexData.uvs.Push (uv);
	    primVertexData.normals.Push (norm);
	  }
	}

	size_t numIndices = (primSamplesX - 1) * (primSamplesY - 1) * 2;
	PrimitiveArray& cellPrimitives = allPrimitives.GetExtend (
	  allPrimitives.GetSize());
	cellPrimitives.SetCapacity (numIndices);
	for (uint y = 0; y < primSamplesY-1; y++)
	{
	  for (uint x = 0; x < primSamplesX-1; x++)
	  {
	    Primitive prim1 (primVertexData, 0);
	    prim1.GetTriangle().a = indexOffs + (y*primSamplesX) + x;
	    prim1.GetTriangle().b = indexOffs + (y*primSamplesX) + x + 1;
	    prim1.GetTriangle().c = indexOffs + ((y+1)*primSamplesX) + x;
	    prim1.ComputePlane ();

	    Primitive prim2 (primVertexData, 0);
	    prim2.GetTriangle().a = indexOffs + ((y+1)*primSamplesX) + x;
	    prim2.GetTriangle().b = indexOffs + (y*primSamplesX) + x + 1;
	    prim2.GetTriangle().c = indexOffs + ((y+1)*primSamplesX) + x + 1;
	    prim2.ComputePlane ();

	    cellPrimitives.Push (prim1);
	    cellPrimitives.Push (prim2);
	  }
	}
      }

      {
	float invSamplesX = 1.0f/(lmSamplesX);
	float invSamplesY = 1.0f/(lmSamplesY);

	float pScaleX = cellSize.x * invSamplesX;
	float pScaleY = cellSize.z * invSamplesY;

	for (uint y = 0; y < lmSamplesY; y++)
	{
	  for (uint x = 0; x < lmSamplesX; x++)
	  {
	    csVector2 p ((x + 0.5f) * pScaleX, (y + 0.5f) * pScaleY);

	    float height = cell->GetHeight (p);
	    csVector3 norm (cell->GetNormal (p));

	    vertexData.positions.Push (csVector3 (
	      p.x + cellPos.x, height, 
	      cellSize.z - p.y + cellPos.y));
	    vertexData.normals.Push (norm);
	    if (factory->hasTangents)
	    {
	      csVector3 tang (cell->GetTangent (p));
	      csVector3 bitang (cell->GetBinormal (p));
	      size_t v = vertexData.positions.GetSize()-1;
	      vertexData.customData.SetSize ((v+1)*vertexData.customDataTotalComp);
	      *((csVector3*)vertexData.GetCustomData (v, vdataTangents)) = tang;
	      *((csVector3*)vertexData.GetCustomData (v, vdataBitangents)) = bitang;
	    }
	  }
	}

	LMDimensions lmDim;
	lmDim.w = lmSamplesX;
	lmDim.h = lmSamplesY;
	lmDims.Push (lmDim);
      }

      cell->SetLoadState (iTerrainCell::NotLoaded);
    }
  }

  void Object_Terrain2::SaveMesh (iDocumentNode *node)
  {
    csRef<iTerrainSystem> terrSys = scfQueryInterface<iTerrainSystem> (
      meshWrapper->GetMeshObject());
    if (!terrSys.IsValid()) return;
    csRef<iTerrainFactory> terrFact = scfQueryInterface<iTerrainFactory> (
      meshWrapper->GetFactory()->GetMeshObjectFactory());
    if (!terrFact.IsValid()) return;

    CS::ShaderVarName lightmapName[4] =
    { 
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap"),
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap dir 1"),
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap dir 2"),
      CS::ShaderVarName (globalLighter->svStrings, "tex lightmap dir 3")
    };
    int numLMs = globalConfig.GetLighterProperties().directionalLMs ? 4 : 1;

    for (size_t c = 0; c < lightmapIDs.GetSize(); c++)
    {
      iTerrainCell* cell = terrSys->GetCell (c);
      iTerrainFactoryCell* factCell = terrFact->GetCell (c);
      /* We only ensure unique cell names after the terrain object is
         loaded (and hence cell names set), so copy the names over  here */
      cell->SetName (factCell->GetName());
      iTerrainCellRenderProperties* cellRender = cell->GetRenderProperties();
      iShaderVariableContext* svc = cellRender;

      for (int l = 0; l < numLMs; l++)
      {
        uint lmID = uint (lightmapIDs[c]);
        Lightmap* lm = sector->scene->GetLightmap(lmID, l);
        csRef<csShaderVariable> svLightmap;
        svLightmap.AttachNew (new csShaderVariable (lightmapName[l]));
        svLightmap->SetValue (lm->GetTexture());
        svc->AddVariable (svLightmap);
      }
    }

    Object::SaveMesh (node);
  }

  void Object_Terrain2::PrepareLighting ()
  {
    csRef<ObjectFactory_Terrain2> factoryT2 = 
      static_cast<ObjectFactory_Terrain2*> ((ObjectFactory*)factory);

    // @@@ FIXME: lots of primitives here, progress output might be good
    for (size_t j = 0; j < this->allPrimitives.GetSize(); j++)
    {
      PrimitiveArray& allPrimitives = this->allPrimitives[j];

      uint lmID = factoryT2->uvlayout->AllocLightmap (lmDims[j].w,
	lmDims[j].h);
      lightmapIDs.Push (lmID);

      for (size_t i = 0; i < allPrimitives.GetSize(); i++)
      {
        Primitive& prim = allPrimitives[i];
        prim.SetObject (this);
	prim.SetGlobalLightmapID (lmID);
        prim.Prepare ();
      }
    }

    size_t n = globalConfig.GetLighterProperties().directionalLMs ? 4 : 1;

    litColors = new LitColorArray[n];
    for (size_t i = 0; i < n; i++)
      litColors[i].SetSize (vertexData.positions.GetSize(), 
        csColor (0.0f, 0.0f, 0.0f));
    litColorsPD = new LitColorsPDHash[n];

    factory.Invalidate();
  }

  void Object_Terrain2::SaveMeshPostLighting (Scene* scene)
  {
    int numLMs = globalConfig.GetLighterProperties().directionalLMs ? 4 : 1;

    for (int i = 0; i < numLMs; i++)
    {
      csColor* myColor = litColors[i].GetArray();
      for (size_t l = 0; l < lmDims.GetSize(); l++)
      {
	const LMDimensions& lmDim = lmDims[l];
	uint lmID = lightmapIDs[l];

	Lightmap* normalLM = scene->GetLightmap (
	  lmID, i, (Light*)0);

	ScopedSwapLock<Lightmap> lightLock (*normalLM);

	csColor* lmColor = normalLM->GetData();
	for (int y = 0; y < lmDim.h; y++)
	{
	  for (int x = 0; x < lmDim.w; x++)
	  {
	    *lmColor++ = *myColor++;
	  }
	}
      }
      LitColorsPDHash::GlobalIterator pdIter (litColorsPD[i].GetIterator ());
      while (pdIter.HasNext ())
      {
	csPtrKey<Light> light;
	LitColorArray& colors = pdIter.Next (light);
  
	csColor* myColor = colors.GetArray();
	for (size_t l = 0; l < lmDims.GetSize(); l++)
	{
	  const LMDimensions& lmDim = lmDims[l];
	  uint lmID = lightmapIDs[l];
  
	  Lightmap* normalLM = scene->GetLightmap (
	    lmID, i, light);
  
	  ScopedSwapLock<Lightmap> lightLock (*normalLM);
  
	  csColor* lmColor = normalLM->GetData();
	  for (int y = 0; y < lmDim.h; y++)
	  {
	    for (int x = 0; x < lmDim.w; x++)
	    {
	      *lmColor++ = *myColor++;
	    }
	  }
	}
      }
    }
  }

  //-------------------------------------------------------------------------

  ObjectFactory_Terrain2::ObjectFactory_Terrain2 (const Configuration& config) :
    ObjectFactory (config)
  {
    saverPluginName = "crystalspace.mesh.saver.factory.terrain2";
  }

  csPtr<Object> ObjectFactory_Terrain2::CreateObject ()
  {
    return csPtr<Object> (new Object_Terrain2 (this));
  }

  bool ObjectFactory_Terrain2::PrepareLightmapUV (LightmapUVFactoryLayouter* uvlayout)
  {
    this->uvlayout = uvlayout;
    return true;
  }
  
  void ObjectFactory_Terrain2::ParseFactory (iMeshFactoryWrapper *factory)
  {
    ObjectFactory::ParseFactory (factory);

    csRef<iTerrainFactory> terrFact = scfQueryInterface<iTerrainFactory> (
      factory->GetMeshObjectFactory());
    if (!terrFact.IsValid()) return;

    if (globalConfig.GetLighterProperties().directionalLMs)
    {
      vdataTangents = vertexData.AddCustomData (3);
      vdataBitangents = vertexData.AddCustomData (3);
      hasTangents = true;
    }
    
    // Each cell needs a unique name
    uint counter = 0;
    csSet<csString> usedCellNames;
    size_t numCells = terrFact->GetCellCount();
    for (size_t c = 0; c < numCells; c++)
    {
      iTerrainFactoryCell* cell = terrFact->GetCell (c);
      const char* name = cell->GetName();
      if ((name == 0) || (usedCellNames.Contains (name)))
      {
	csString newName;
	do
	{
	  newName.Format ("%u", counter++);
	}
	while (usedCellNames.Contains (name));
	cell->SetName (newName);
	usedCellNames.AddNoTest (newName);
      }
    }
  }

  /*void ObjectFactory_Terrain2::SaveFactory (iDocumentNode *node)
  {
  }*/
  
} // namespace lighter
