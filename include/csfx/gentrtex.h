/*
    Copyright (C) 2001 by W.C.A. Wijngaards
  
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

#ifndef __CSGENTERTEX_H__
#define __CSGENTERTEX_H__

#include "csutil/cscolor.h"
#include "csgeom/vector2.h"
#include "csgfx/rgbpixel.h"
struct iImage;
class csGenerateTerrainImagePart;

/** This class will compute a texture for a terrain.
 * The texture is based on the heightmap for the terrain.
 * It is like the povray MaterialMap, but then indexed with
 * the height of the terrain.
 * This means, that given some base textures that should display
 * at certain heights, the whole texture is computed. At each
 * pixel the base textures are blended together, based on the
 * height.
*/
class csGenerateTerrainImage 
{
  /// the list of base textures, sorted from low to high height
  csGenerateTerrainImagePart *baselist;
  /// the height function to use float func(userdata, x, y);
  float (*heightfunc)(void*, float, float);
  /// user data for height function
  void *userdata;
public:
  /// create empty
  csGenerateTerrainImage();
  /// destroy
  ~csGenerateTerrainImage();

  /// get a color for a pixel in image, tiles, result in res.
  void GetImagePixel(iImage *image, int x, int y, csRGBpixel& res);

  /// compute a color (0..255) for a spot on a layer
  csColor ComputeLayerColor(csGenerateTerrainImagePart *layer,
    const csVector2& pos);

  /// compute color for a spot (0..1) on the terrain
  csRGBpixel ComputeColor(const csVector2& pos);

  /**
   * Add a base texture. It is displayed at a certain layer of height.
   * You can give the height where this texture should show up.
   * (0 == low, 1 == high)
   * give the iImage of the texture, as well as the scale and offset.
   * offset is from 0..1, scale is the size respective to total image size.
   * A scale of 1 means the texture will fit exactly onto the entire terrain.
   * A scale of 0.1 means the texture will be tiled 10x10 times.
   */
  void AddLayer(float height, iImage *image, const csVector2& scale,
    const csVector2& offset);

  /**
   * Set the height function to use for the terrain.
   * The function type is:
   * float heightfunc(void *userdata, float x, float y);
   * userdata is passed unchanged. x and y range from 0..1;
   * return a height from 0 (low) to 1(high)
   */
  void SetHeightFunction( float (*func)(void*,float,float), void *data)
  { heightfunc = func; userdata = data;}

  /**
   * Generate part of a terrain image.
   * Give total terrain image size,
   * give the startx,y texel in the total image, and the size
   * of the part you want generated
   * creates a new iImage.
   */
  iImage *Generate(int totalw, int totalh, int startx, int starty, 
    int partw, int parth);
};

/** 
 * This class is used to store the base texture information
 * inside the csGenerateTerrainImage class 
 */
class csGenerateTerrainImagePart
{
public:
  /// the height where this texture should show (0..1)
  float height;
  /// the image - the texture image
  iImage *image;
  /// the scale of the image, 1/(nr of times tiled on the terrain)
  csVector2 scale;
  /// offset to shift image (0..1)
  csVector2 offset;
  /// next part (ascending in height)
  csGenerateTerrainImagePart *next;
};
#endif

