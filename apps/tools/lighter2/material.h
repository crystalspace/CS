/*
  Copyright (C) 2007 by Frank Richter

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

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "common.h"
#include "swappable.h"

#include <typeinfo>

namespace lighter
{
  template<typename T>
  class MaterialImage : public csRefCount, public Swappable
  {
  public:
    MaterialImage (int width, int height) : width (width), height (height)
    {
      colorArray = (T*)SwappableHeap::Alloc (GetWidth() * GetHeight() * 
        sizeof (T));
    }

    ~MaterialImage ()
    {
      Lock();
      SwappableHeap::Free (colorArray);
    }

    // Data getters
    inline T* GetData () const { return colorArray; }

    inline int GetWidth () const { return width; }
    inline int GetHeight () const { return height; }
    
    T GetValue (int x, int y) const
    { return colorArray[y*width + x]; }
    T GetValueWrapped (int x, int y) const
    {
      int nx = x % width;
      int ny = y % height;
      return GetValue (ABS (nx), ABS (ny));
    }
    T GetInterpolated (float x, float y) const
    {
      int x1 = int (floorf (x));
      float t1 = x - x1;
      int y1 = int (floorf (y));
      float t2 = y - y1;
      return csLerp (
        csLerp (GetValueWrapped (x1,   y1  ), 
		GetValueWrapped (x1+1, y1  ), t1),
        csLerp (GetValueWrapped (x1,   y1+1), 
		GetValueWrapped (x1+1, y1+1), t1),
	t2);
    }
    T GetInterpolated (const csVector2& v) const
    { return GetInterpolated (v.x * width, v.y * height); }

    virtual void GetSwapData (void*& data, size_t& size)
    {
      data = colorArray;
      size = GetWidth() * GetHeight() * sizeof (T);
      // Set a bogus pointer so accesses to swapped data causes a segfault
      colorArray = BogusPointer ();
    }
    virtual size_t GetSwapSize()
    {
      return GetWidth() * GetHeight() * sizeof (T);
    }
    virtual void SwapIn (void* data, size_t size)
    {
      CS_ASSERT (size == GetWidth() * GetHeight() * sizeof (T));
      CS_ASSERT (colorArray == BogusPointer ());
      colorArray = (T*)data;
    }
    const char* Describe() const
    { return typeid(*this).name(); }
  protected:
    // The color data itself
    mutable T *colorArray;

    int width, height;

    inline T* BogusPointer () const 
    { return ((T*)~0) - (GetWidth() * GetHeight()); }
  };

  struct RadMaterial
  {
    
    // The original texture image
    CS::ImageAutoConvert * textureImg;
    csRef<MaterialImage<csColor> > filterImage;
    float refractiveIndex;
    bool isTexImageValid;
    bool produceCaustic;
	   
    RadMaterial() {refractiveIndex = 1; isTexImageValid=false; produceCaustic= false;}
    bool IsTransparent () const { return filterImage.IsValid(); }
    void ComputeFilterImage (iImage* img);
    void SetTextureImage (iImage * img);
    void SetRefractiveIndex(float refrIndex) {refractiveIndex = refrIndex;}
    bool IsTextureValid() const {return isTexImageValid;}
    float GetRefractiveIndex(){return refractiveIndex;}
    //Returns the color at uv coordinates in original texture
    csColor GetTextureValue(csVector2 uv) const;

  };
  typedef csHash<RadMaterial, csString> MaterialHash;
} // namespace lighter

#endif // __MATERIAL_H__
