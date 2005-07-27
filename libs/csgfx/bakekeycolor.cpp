/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#include "cssysdef.h"

#include "csgfx/bakekeycolor.h"
#include "csgfx/memimage.h"
#include "csutil/bitarray.h"

#include <limits.h>

//#define KEYCOLOR_DEBUG

#ifdef KEYCOLOR_DEBUG
#include "cstool/debugimagewriter.h"
#endif

/*
 * Template fun!
 */

template<int D> struct ArrayMul
{
  static inline int Mul (const int* v)
  {
    return *v * ArrayMul<D-1>::Mul (v+1);
  }
};
CS_SPECIALIZE_TEMPLATE struct ArrayMul<0>
{
  static inline int Mul (const int* v)
  {
    return *v;
  }
};

template<int D> struct CoordSplitter
{
  static inline void Split (int* destCoords, size_t coord, const int* dim)
  {
    destCoords[D] = (int)coord / dim[D];
    CoordSplitter<D-1>::Split (destCoords, coord % dim[D], dim);
  }
};
CS_SPECIALIZE_TEMPLATE struct CoordSplitter<0>
{
  static inline void Split (int* destCoords, size_t coord, const int* dim)
  {
    CS_ASSERT (dim[0] == 1);
    destCoords[0] = (int)coord;
  }
};

template<int D> struct DimPixCompute
{
  static inline void Compute (int* dimPix, const int* sizes)
  {
    DimPixCompute<D-1>::Compute (dimPix, sizes);
    dimPix[D] = dimPix[D-1] * sizes[D-1];
  }
};
CS_SPECIALIZE_TEMPLATE struct DimPixCompute<0>
{
  static inline void Compute (int* dimPix, const int* sizes)
  {
    dimPix[0] = 1;
  }
};

template<class PixelIO>
class TranspMgr
{
  PixelIO pio;
  const csRGBpixel& transp;
  csBitArray tested, isTransp;
public:
  TranspMgr (const PixelIO& pio, const csRGBpixel& transp, size_t pixNum) :
      pio(pio), transp(transp), tested(pixNum), isTransp(pixNum) {}

  bool IsTransp (size_t px)
  {
    if (tested.IsBitSet (px))
      return isTransp.IsBitSet (px);
    else
    {
      bool t = pio[px] == transp;
      tested.SetBit (px);
      isTransp.Set (px, t);
      return t;
    }
  }
};

template<int Dimensions, class PixelIO>
class MaKeyColorBaker
{
public:
  static void RenderToAlpha (PixelIO& dst, PixelIO& src,
    const csRGBpixel& transpColor, const int sizes[Dimensions])
  {
    size_t pixNum = ArrayMul<Dimensions-1>::Mul (sizes); //sizes[0];
    TranspMgr<PixelIO> transpMgr (src, transpColor, pixNum);
    int dimPixels[Dimensions];
    DimPixCompute<Dimensions-1>::Compute (dimPixels, sizes);
    const int neighCount = Dimensions * 2;
    const PixelIO srcStart = src;
    uint* distances = new uint[pixNum];
    memset (distances, 0xff, sizeof(uint) * pixNum);

    for (size_t p = 0; p < pixNum; p++) // Position
    {
      int coord[Dimensions];
      CoordSplitter<Dimensions-1>::Split (coord, p, dimPixels);
      int d, n;
      if (transpMgr.IsTransp (p))
      {
	int mr = 0, mg = 0, mb = 0;
	int neighPix = 0;
	uint neighMask = 0;
	for (n = 0; n < neighCount; n++)
	{
	  uint newpx = 0;
	  for (int d = 0; d < Dimensions; d++)
	  {
	    int delta = ((n >> 1) != d) ? 0 : (n & 1) * 2 - 1;
	    int nc = coord[d] + delta; // New Coordintate
	    if (nc < 0)
	      nc = sizes[d] - 1;
	    else if (nc >= sizes[d])
	      nc = 0;
	    newpx += nc * dimPixels[d];
	  }
	  if (!transpMgr.IsTransp (newpx))
	  {
	    const csRGBpixel px (srcStart[newpx]);
	    mr += px.red;
	    mg += px.green;
	    mb += px.blue;
	    neighPix++;
	    neighMask |= 1 << n;
	  }
	}
        int dimDelta[Dimensions];
	csRGBpixel avg;
	int ec = 0;
	if (neighPix != 0)
	{
	  /* There are direct neighbours. Use their averaged color to fill 
	   * KCed areas. */

	  /*
	    Drop the alpha of the transparent pixels to 0, but leave the alpha
	    of non-keycolored ones as is. Keycolor only makes transparent, but
	    doesn't mean the rest of the image isn't.
	  */
	  avg.Set (mr / neighPix, mg / neighPix, mb / neighPix, 0);

	  if (neighMask != 0)
	  {
	    for (d = 0; d < Dimensions; d++)
	    {
	      uint f = (neighMask >> (d * 2)) & 3;
	      int delta;
	      switch (f)
	      {
		case 1: delta = 1;  break;
		case 2: delta = -1; break;
		default: delta = 0; break;
	      }
	      dimDelta[d] = delta;
	    }
	  }
	}
	else
	{
	  /* Maybe some lonely edge is here. Use its color for KC filling. */
	  for (n = 0; n < (1 << Dimensions); n++)
	  {
	    int ofs = 0;
	    int nd[Dimensions]; // New Delta
	    for (d = 0; d < Dimensions; d++)
	    {
	      nd[d] = (((n >> d) & 1) * 2 - 1);
	      int nc = coord[d] + nd[d]; // New Coordintate
	      if (nc < 0)
		nc = sizes[d] - 1;
	      else if (nc >= sizes[d])
		nc = 0;
	      ofs += nc * dimPixels[d];
	    }
	    if (!transpMgr.IsTransp (ofs))
	    {
	      ec++;
	      for (d = 0; d < Dimensions; d++)
		dimDelta[d] = -nd[d];
	      avg = srcStart[ofs];
	      avg.alpha = 0;
	    }
	  }
	}

	if ((neighPix != 0) || (ec == 1))
	{
	  int startCoord[Dimensions];
	  memcpy (startCoord, coord, sizeof(startCoord));
	  int cDist[Dimensions];
	  memset (cDist, 0, sizeof (cDist));
	  int maxDist[Dimensions];
	  for (d = 0; d < Dimensions; d++) maxDist[d] = INT_MAX;

	  dst.Set (avg);
	  distances[p] = 0;
	  d = 0;
	  while (d < Dimensions)
	  {
	    bool coordOkay = ((coord[d] >= 0) && (coord[d] < sizes[d])
	      && (coord[d] < maxDist[d]) && (dimDelta[d] != 0));
	    if (!coordOkay)
	    {
	      coord[d] = startCoord[d];
	      cDist[d] = 0;
	      d++;
	      continue;
	    }
	    int pOfs = 0;
	    uint newdist = 0;
	    int d2;
	    for (d2 = 0; d2 < Dimensions; d2++)
	    {
	      pOfs += cDist[d2] * dimPixels[d2];
	      newdist += cDist[d2] * cDist[d2];
	    }
	    if (!transpMgr.IsTransp (pOfs + p))
	    {
	      maxDist[d] = cDist[d];
	      coord[d] = startCoord[d];
	      cDist[d] = 0;
	      d++;
	      continue;
	    }
	    uint& dist = distances[p + pOfs];
	    if (dist < newdist) 
	    {
	      coord[d] = startCoord[d];
	      cDist[d] = 0;
	      d++;
	      continue;
	    }
	    dist = newdist;
	    dst.Set (pOfs, avg);
	    coord[d] += dimDelta[d];
	    cDist[d] += dimDelta[d];
	    if ((coord[d] >= 0) && (coord[d] < sizes[d])&& (dimDelta[d] != 0))
	    {
	      d = 0;
	    }
	    else
	    {
	      coord[d] = startCoord[d];
	      cDist[d] = 0;
	      d++;
	    }
	  }
     	  memcpy (coord, startCoord, sizeof(startCoord));
	}
	else
	{
	  if (distances[p] == (uint)~0) 
	    dst.Set (transpColor.red, transpColor.green, transpColor.blue, 0);
	}

	++dst;
	++src;
      }
      else
      {
	dst.Set (*src);
	++dst; ++src;
	distances[p] = 0;
      }
    }
    delete[] distances;
  }
};

class PixelIORGBpixel
{
  csRGBpixel* ptr;
public:
  PixelIORGBpixel (csRGBpixel* ptr) : ptr(ptr) {}

  const csRGBpixel& operator* () const
  { return *ptr; }
  const csRGBpixel& operator[] (size_t index) const
  { return *(ptr + index); }
  PixelIORGBpixel& operator++ ()
  { ptr++; return *this; }
  void Set (int r, int g, int b, int a)
  { ptr->Set (r, g, b, a); }
  void Set (const csRGBpixel& col)
  { *ptr = col; }
  void Set (int ofs, const csRGBpixel& col)
  { *(ptr+ofs) = col; }
};

csRef<iImage> csBakeKeyColor::Image (iImage* source, 
				     const csRGBpixel& transpColor)
{
  csRef<iImage> image;
  if (source->GetFormat() != (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA))
  {
    image.AttachNew (new csImageMemory (source, 
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
  }
  else
    image = source;

  csRef<csImageMemory> result;
  result.AttachNew (new csImageMemory (source->GetWidth(), source->GetHeight(), 
    source->GetDepth()));
  result->SetImageType (source->GetImageType());
  PixelIORGBpixel ioSrc ((csRGBpixel*)image->GetImageData());
  PixelIORGBpixel ioDst ((csRGBpixel*)result->GetImagePtr());
  if ((source->GetImageType() == csimg3D) && (source->GetDepth() > 1))
  {
    const int sizes[3] = {image->GetWidth(), image->GetHeight(), image->GetDepth()};
    MaKeyColorBaker<3, PixelIORGBpixel>::RenderToAlpha (ioDst, ioSrc,
      transpColor, sizes);
  }
  else
  {
    const int sizes[2] = {image->GetWidth(), image->GetHeight()};
    MaKeyColorBaker<2, PixelIORGBpixel>::RenderToAlpha (ioDst, ioSrc,
      transpColor, sizes);
  }

  if ((source->GetFormat() & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
    result->SetFormat ((source->GetFormat() & CS_IMGFMT_MASK) | CS_IMGFMT_ALPHA);

#ifdef KEYCOLOR_DEBUG
  csDebugImageWriter::DebugImageWrite (result, "bakekeycolor/%p.png", source);
#endif
  return result;
}

class PixelIORGBA
{
  uint8* ptr;
public:
  PixelIORGBA (uint8* ptr) : ptr(ptr) {}

  csRGBpixel operator* () const
  { return csRGBpixel (*(ptr+0), *(ptr+1), *(ptr+2), *(ptr+3)); }
  csRGBpixel operator[] (size_t index) const
  { 
    const uint8* np = ptr + index*4;
    return csRGBpixel (*(np+0), *(np+1), *(np+2), *(np+3)); 
  }
  PixelIORGBA& operator++ ()
  { ptr += 4; return *this; }
  void Set (int r, int g, int b, int a)
  { 
    *(ptr+0) = r;
    *(ptr+1) = g;
    *(ptr+2) = b;
    *(ptr+3) = a;
  }
  void Set (const csRGBpixel& col)
  { Set (col.red, col.green, col.blue, col.alpha); }
  void Set (int ofs, const csRGBpixel& col)
  { 
    uint8* np = ptr + ofs*4;
    *(np+0) = col.red;
    *(np+1) = col.green;
    *(np+2) = col.blue;
    *(np+3) = col.alpha;
  }
};

void csBakeKeyColor::RGBA2D (uint8* dest, const uint8* source, int w, int h, 
			     const csRGBpixel& transpColor)
{
  PixelIORGBA ioSrc ((uint8*)source);
  PixelIORGBA ioDst (dest);
  const int sizes[2] = {w, h};
  MaKeyColorBaker<2, PixelIORGBA>::RenderToAlpha (ioDst, ioSrc,
    transpColor, sizes);
#ifdef KEYCOLOR_DEBUG
  csRef<iImage> result;
  result.AttachNew (new csImageMemory (w, h, dest, false));
  static int counter = 0;
  csDebugImageWriter::DebugImageWrite (result, "bakekeycolor/%d.png", counter++);
#endif
}
