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

#ifndef __CS_GENTERTEX_H__
#define __CS_GENTERTEX_H__

#include "csextern.h"

#include "csgeom/vector2.h"
#include "csutil/cscolor.h"
#include "csutil/scf.h"

struct iImage;

class csRGBpixel;

/**
 * A base class which represents a value that can be computed
 * for blending purposes for each pixel.
 */
class CS_CRYSTALSPACE_EXPORT csGenerateImageValue
{
public:
  /// delete it
  virtual ~csGenerateImageValue() {}
  /// get the value for location
  virtual float GetValue (float x, float y) = 0;
};

/**
 * A base class which represents a texture that can be displayed
 * on the terrain. It has a colour for each pixel
 */
class CS_CRYSTALSPACE_EXPORT csGenerateImageTexture
{
public:
  /// delete it
  virtual ~csGenerateImageTexture() {}
  /// get color (0..1) for location
  virtual void GetColor(csColor& col, float x, float y) = 0;
};


/**
 * This class will compute a texture for a terrain.
 * The texture is based on the heightmap for the terrain.
 * It is like the povray MaterialMap, but then indexed with
 * the height of the terrain.
 * This means, that given some base textures that should display
 * at certain heights, the whole texture is computed. At each
 * pixel the base textures are blended together, based on the
 * height.
*/
class CS_CRYSTALSPACE_EXPORT csGenerateImage
{
private:
  /// the texture to show
  csGenerateImageTexture *tex;

public:
  /// create empty
  csGenerateImage();
  /// destroy
  ~csGenerateImage();

  /**
   * Set the texture to show, You can easily construct one yourself,
   * using the classes below.
   */
  void SetTexture(csGenerateImageTexture *t) {tex = t;}

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
class CS_CRYSTALSPACE_EXPORT csGenerateImageLayer
{
public:
  /// the value where this texture should show
  float value;
  /// the texture for this layer
  csGenerateImageTexture *tex;
  /// next part (ascending in height)
  csGenerateImageLayer *next;
};

/**
 * A class for a solid coloured texture.
 */
class CS_CRYSTALSPACE_EXPORT csGenerateImageTextureSolid : 
  public csGenerateImageTexture
{
public:
  /// the colour, range 0-1
  csColor color;
  /// delete it
  virtual ~csGenerateImageTextureSolid() {}
  /// get the color
  virtual void GetColor(csColor& col, float, float) {col = color;}
};

/**
 * A class for a single texture.
 */
class CS_CRYSTALSPACE_EXPORT csGenerateImageTextureSingle : 
  public csGenerateImageTexture
{
public:
  /// the image - the texture image
  csRef<iImage> image;
  /// the scale of the image, (nr of times tiled on the terrain)
  csVector2 scale;
  /// offset to shift image (0..1)
  csVector2 offset;

  /// delete it
  virtual ~csGenerateImageTextureSingle();
  /// add image
  void SetImage(iImage *im);
  /// get the color
  virtual void GetColor(csColor& col, float x, float y);
  /// get a color for a pixel in image, tiles, result in res.
  void GetImagePixel(iImage *image, int x, int y, csRGBpixel& res);
  /// compute a color (0..1) for a spot on a layer
  void ComputeLayerColor(const csVector2& pos, csColor& col);
};

/**
 * a class for a texture that is made by blending together other textures
 * based on a value. It has a set of layers to blend between.
 */
class CS_CRYSTALSPACE_EXPORT csGenerateImageTextureBlend : 
  public csGenerateImageTexture
{
public:
  /// the list - sorted by value - of layers
  csGenerateImageLayer *layers;
  /// the value function object
  csGenerateImageValue *valuefunc;
  ///
  csGenerateImageTextureBlend();
  /// deletes the list too
  virtual ~csGenerateImageTextureBlend();
  /// get the color
  virtual void GetColor(csColor& col, float x, float y);
  /// add a layer correctly sorted into the list
  void AddLayer(float value, csGenerateImageTexture *tex);
};

SCF_VERSION (iGenerateImageFunction, 0, 0, 1);

/**
 * This class represents a function for csGenerateImageValueFunc. Expects
 * values for dx and dy between 0 and 1 and returns a height or slope.
 */
struct iGenerateImageFunction : public iBase
{
  /// Get height or slope.
  virtual float GetValue (float dx, float dy) = 0;
};


/**
 * This class will generate a value using a given function. For heights
 * or slopes.
 */
class CS_CRYSTALSPACE_EXPORT csGenerateImageValueFunc : public csGenerateImageValue
{
private:
  /// Height or slope function.
  csRef<iGenerateImageFunction> heightfunc;

public:
  csGenerateImageValueFunc () { }
  virtual ~csGenerateImageValueFunc ()
  {
  }

  /// Get the value for location.
  virtual float GetValue(float x, float y)
  {
    return heightfunc->GetValue (x, y);
  }
  /// Set the function.
  void SetFunction (iGenerateImageFunction* func)
  {
    heightfunc = func;
  }
};

/**
 * This class will generate a constant value.
 */
class CS_CRYSTALSPACE_EXPORT csGenerateImageValueFuncConst : 
  public csGenerateImageValue
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
class CS_CRYSTALSPACE_EXPORT csGenerateImageValueFuncTex : 
  public csGenerateImageValue
{
public:
  /// the texture to use
  csGenerateImageTexture *tex;
  ///
  virtual ~csGenerateImageValueFuncTex();
  /// get the value for location
  virtual float GetValue(float x, float y);
};


#endif // __CS_GENTERTEX_H__

