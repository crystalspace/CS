/*
    Copyright (C) 2007 by Jelle Hellemans aka sueastside

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

#ifndef __BASEMAPGEN_H__
#define __BASEMAPGEN_H__

#include "layers.h"

class BaseMapGen
{
private:
  iObjectRegistry* object_reg;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iCommandLineParser> cmdline;
  
  csRegExpMatcher* terraformerRE;
  csRegExpMatcher* meshRE;

  // The worldfile
  csRef<iDocumentNode> rootnode;
  
public:
  csHash<csString, csString> pluginMap;
  csHash<csString, csString> textureFiles;
  csHash<csRef<MaterialLayer>, csString> materials;
  
  csHash<csRef<AlphaLayers>, csString> terrain1Layers;
  csHash<csRef<AlphaLayers>, csString> terrain1FactoryLayers;
  
  void ScanPluginNodes ();
  const char* GetPluginSCFID (const char* pluginStr);
  void ScanTextures ();
  void ScanMaterials ();

  bool LoadMap ();
  void ScanOldMaterialMaps();
  void ScanTerrain1Factories ();
  void ScanTerrain1Meshes ();
  
  struct Terrain2Cell : public csRefCount
  {
    csString name;
    csString baseMaterial;
    csRef<AlphaLayers> alphaLayers;
    MaterialLayers alphaMaterials;
    csRef<AlphaLayers> materialMapLayers;
    
    bool Parse (iDocumentNode* node);
    void ApplyMaterialMap (const MaterialLayers& matMap);
  };
  struct Terrain2Factory : public csRefCount
  {
    csHash<csRef<Terrain2Cell>, csString> cells;
  };
  csHash<csRef<Terrain2Factory>, csString> terrain2Factories;
  void ScanTerrain2Factories ();
  void ScanTerrain2Meshes ();
  
  csString GetTextureFile (const csString& texturename);

  csRef<iDocumentNode> GetTerrainNode ();
  csRef<iDocumentNode> GetMaterialNode (const char* materialname);
  csRefArray<iDocumentNode> GetMaterialNodes ();
  
  csPtr<iImage> CreateBasemap (int basemap_w, int basemap_h, 
                               const AlphaLayers& alphaLayers,
                               MaterialLayers& txt_layers);
  void SaveImage (iImage* image, const char* filename);
public:
  BaseMapGen (iObjectRegistry* object_reg);
  ~BaseMapGen ();

  csRef<iImage> LoadImage (const csString& filename, int format);
  
  bool Initialize ();
  void Start ();
  void OnCommandLineHelp();
  void Report(const char* msg, ...);
  void DrawProgress (int percent);
};

#endif // __BASEMAPGEN_H__

