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
class csGenerateTerrainImageValue;
class csGenerateTerrainImageTexture;

/** a base class which represents a value that can be computed
  for blending purposes for each pixel. */
class csGenerateTerrainImageValue
{
public:
  /// delete it
  virtual ~csGenerateTerrainImageValue() {}
  /// get the value for location
  virtual float GetValue(float x, float y) = 0;
};

/** a base class which represents a texture that can be displayed
  on the terrain. It has a colour for each pixel */
class csGenerateTerrainImageTexture
{
public:
  /// delete it
  virtual ~csGenerateTerrainImageTexture() {}
  /// get color (0..255) for location
  virtual void GetColor(csColor& col, float x, float y) = 0;
};


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
  /// the texture to show
  csGenerateTerrainImageTexture *tex;
public:
  /// create empty
  csGenerateTerrainImage();
  /// destroy
  ~csGenerateTerrainImage();

  /**
   * Set the texture to show, You can easily construct one yourself,
   * using the classes below.
   */
  void SetTexture(csGenerateTerrainImageTexture *t) {tex = t;}

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
 * This class is used to store the layers of textures per value. Used in the
 * Blend class.
 */
class csGenerateTerrainImageLayer {
public:
  /// the value where this texture should show
  float value;
  /// the texture for this layer
  csGenerateTerrainImageTexture *tex;
  /// next part (ascending in height)
  csGenerateTerrainImageLayer *next;
};

/**
 * a class for a solid coloured texture
*/
class csGenerateTerrainImageTextureSolid:public csGenerateTerrainImageTexture
{
public:
  /// the colour, range 0-255
  csColor color;
  /// delete it
  virtual ~csGenerateTerrainImageTextureSolid() {}
  /// get the color
  virtual void GetColor(csColor& col, float, float) {col = color;}
};

/**
 * A class for a single texture
*/
class csGenerateTerrainImageTextureSingle:public csGenerateTerrainImageTexture
{
public:
  /// the image - the texture image
  iImage *image;
  /// the scale of the image, (nr of times tiled on the terrain)
  csVector2 scale;
  /// offset to shift image (0..1)
  csVector2 offset;

  /// delete it
  virtual ~csGenerateTerrainImageTextureSingle();
  /// add image
  void SetImage(iImage *im);
  /// get the color
  virtual void GetColor(csColor& col, float x, float y);
  /// get a color for a pixel in image, tiles, result in res.
  void GetImagePixel(iImage *image, int x, int y, csRGBpixel& res);
  /// compute a color (0..255) for a spot on a layer
  void ComputeLayerColor(const csVector2& pos, csColor& col);
};

/**
 * a class for a texture that is made by blending together other textures
 * based on a value. It has a set of layers to blend between.
 */
class csGenerateTerrainImageTextureBlend:public csGenerateTerrainImageTexture
{
public:
  /// the list - sorted by value - of layers
  csGenerateTerrainImageLayer *layers;
  /// the value function object
  csGenerateTerrainImageValue *valuefunc;
  /// deletes the list too
  virtual ~csGenerateTerrainImageTextureBlend();
  /// get the color
  virtual void GetColor(csColor& col, float x, float y);
  /// add a layer correctly sorted into the list
  void AddLayer(float value, csGenerateTerrainImageTexture *tex);
};


/**
 * This class will generate a value using a given function. For heights
 * or slopes.
*/
class csGenerateTerrainImageValueFunc : public csGenerateTerrainImageValue
{
public:
  /// height or slope func
  float (*heightfunc)(void *, float, float);
  /// userdata for the heightfunc
  void *userdata;
  /// get the value for location
  virtual float GetValue(float x, float y){return heightfunc(userdata, x, y);}
};

/**
 * This class will generate a constant value
*/
class csGenerateTerrainImageValueFuncConst : public csGenerateTerrainImageValue
{
public:
  /// the value to return
  float constant;
  /// get the value for location
  virtual float GetValue(float, float){return constant;}
};

/**
 * This class will generate a value using a texture. The average of the
 * rgb values will be returned.
*/
class csGenerateTerrainImageValueFuncTex : public csGenerateTerrainImageValue
{
public:
  /// the texture to use
  csGenerateTerrainImageTexture *tex;
  ///
  ~csGenerateTerrainImageValueFuncTex();
  /// get the value for location
  virtual float GetValue(float x, float y);
};


#endif

