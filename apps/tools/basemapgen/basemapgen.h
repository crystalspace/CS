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

#include "crystalspace.h"
#include "stdarg.h"

class BaseMapGen
{
private:
  iObjectRegistry* object_reg;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iCommandLineParser> cmdline;

  // The worldfile
  csRef<iDocumentNode> rootnode;

  struct MaterialLayer
  {
    csVector2 texture_scale;
    csString texture_name;
    csString texture_file;
    csRef<iImage> image;
  };

  struct ImageMap
  {
    csString texture_file;
    csRef<iImage> image;
  };

  bool LoadMap ();

  void AddMaterialLayer (csArray<MaterialLayer>& txt_layers, iDocumentNode* materialnode);

  csRef<iDocumentNode> GetTerrainNode ();

  csString GetTextureFile (const csString& texturename);

  csRef<iDocumentNode> GetMaterialNode (const csString& materialname);

  csRefArray<iDocumentNode> GetMaterialNodes ();

  ImageMap GetMaterialMap ();

  ImageMap GetBaseMap ();

  csRef<iImage> LoadImage (const csString& filename, int format);

  void SaveImage (ImageMap image);

  void CreateBasemap (int basemap_res, 
                      csRGBpixel* basemap_dst, 
                      int matmap_res,
                      uint8* matmap_dst, 
                      const csArray<MaterialLayer>& txt_layers);

public:
  BaseMapGen (iObjectRegistry* object_reg);
  ~BaseMapGen ();

  bool Initialize ();
  void Start ();
  void OnCommandLineHelp();
  void Report(const char* msg, ...);
};

#endif // __BASEMAPGEN_H__

