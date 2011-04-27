/*
    Copyright (C) 2007 by Jelle Hellemans aka sueastside
              (C) 2008 by Frank Richter

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

#ifndef __LAYERS_H__
#define __LAYERS_H__

class AlphaLayers : public csRefCount
{
  int w, h;
  csArray<uint8*> alphaMaps;
public:
  AlphaLayers() : w (0), h (0) {}
  ~AlphaLayers ();
  
  bool BuildAlphaMapsFromMatMap (iImage* matMap);
  bool AddAlphaMap (iImage* alphaMap);
  void AddRemainderAlpha();
  
  size_t GetSize() const { return alphaMaps.GetSize(); }
  
  int GetWidth() const { return w; }
  int GetHeight() const { return h; }
  float GetAlpha (size_t layer, float coord_x, float coord_y) const;
};

class TextureInfo;

struct MaterialLayer : public csRefCount
{
  csString name;
  csVector2 texture_scale;
  csRef<TextureInfo> texture;
};
typedef csRefArray<MaterialLayer> MaterialLayers;

#endif // __LAYERS_H__

