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
#include "csqint.h"
#include "csutil/refarr.h"
#include "csgfx/memimage.h"

#include "csgfx/imagemanipulate.h"


csRef<iImage> csImageManipulate::Rescale2D (iImage* source, int newwidth, 
    int newheight)
{
  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  if (newwidth == Width && newheight == Height)
    return source;

  // This is a quick and dirty algorithm and it doesn't do funny things
  // such as blending multiple pixels together or bilinear filtering,
  // just a rough scale. It could be improved by someone in the future.

  unsigned int x, y;
  unsigned int dx = csQfixed16 (float (Width) / float (newwidth));
  unsigned int dy = csQfixed16 (float (Height) / float (newheight));

#define RESIZE(pt, Source, Dest)			\
  {							\
    const pt* field = (pt*)Source;			\
    pt* dst = (pt*)Dest;				\
    y = 0;						\
    int ny, nx;						\
    for (ny = newheight; ny; ny--)			\
    {							\
      const pt* src = field + (y >> 16) * Width;	\
      y += dy; x = 0;					\
      for (nx = newwidth; nx; nx--)			\
      {							\
        *dst++ = src [x >> 16];				\
        x += dx;					\
      }							\
    }							\
  }

  csImageMemory* newImg = new csImageMemory (newwidth, newheight,
    source->GetFormat());
  newImg->SetImageType (source->GetImageType());

  switch (source->GetFormat() & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_TRUECOLOR:
      RESIZE (csRGBpixel, source->GetImageData(), newImg->GetImagePtr())
      break;
    case CS_IMGFMT_PALETTED8:
      RESIZE (uint8, source->GetPalette(), newImg->GetPalettePtr())
      break;
  }
  if (source->GetAlpha())
    RESIZE (uint8, source->GetAlpha(), newImg->GetAlphaPtr())

  csRef<iImage> imageRef;
  imageRef.AttachNew (newImg);
  return imageRef;
}

csRef<iImage> csImageManipulate::Rescale (iImage* source, int newwidth, 
    int newheight, int newdepth)
{
  if (source->GetImageType() != csimg3D) 
    return Rescale2D (source, newwidth, newheight);

  const int Width = source->GetWidth();
  const int Height = source->GetHeight();
  const int Depth = source->GetDepth();

  if (newwidth == Width && newheight == Height && newdepth == Depth)
    return source;

  csRef<csImageMemory> newImage;
  newImage.AttachNew (new csImageMemory (newwidth, newheight, newdepth, 
    source->GetFormat()));
  if (source->GetPalette() != 0)
    memcpy (newImage->GetPalettePtr(), source->GetPalette(), 
      sizeof (csRGBpixel) * 256);
  uint dz = csQfixed16 (float (Depth) / float (newdepth));
  if (newdepth < Depth)
  {
    csRef<csImageMemory> resizeScrap;
    resizeScrap.AttachNew (new csImageMemory (Width, Height, 
      source->GetFormat()));
    const size_t srcSliceSize = csImageTools::ComputeDataSize (source) / Depth;
    const size_t srcSlicePix = Width * Height;
    const uint8* srcData = (uint8*)source->GetImageData();
    const uint8* srcAlpha = source->GetAlpha();
    const size_t dstSliceSize = csImageTools::ComputeDataSize (newImage) / newdepth;
    const size_t dstSlicePix = newwidth * newheight;
    uint8* dstData = (uint8*)newImage->GetImagePtr();
    uint8* dstAlpha = newImage->GetAlphaPtr();

    for (int d = 0; d < newdepth; d++)
    {
      uint srcSlice = (dz * d) >> 16;
      memcpy (resizeScrap->GetImagePtr(), srcData + (srcSlice * srcSliceSize), 
	srcSliceSize);
      if (srcAlpha != 0)
	memcpy (resizeScrap->GetAlphaPtr(), srcAlpha + (srcSlice * srcSlicePix), 
	  srcSlicePix);
      csRef<iImage> resizedSlice = Rescale2D (resizeScrap, 
	newwidth, newheight);
      memcpy (dstData + (d * dstSliceSize), resizedSlice->GetImageData(), 
	dstSliceSize);
      if (dstAlpha != 0)
	memcpy (dstAlpha + (d * dstSlicePix), resizedSlice->GetAlpha(), 
	  dstSlicePix);
    }
  }
  else
  {
    int d;
    csRefArray<iImage> resizedSlices;
    {
      csRef<csImageMemory> resizeScrap;
      resizeScrap.AttachNew (new csImageMemory (Width, Height, 
	source->GetFormat()));
      const size_t srcSliceSize = csImageTools::ComputeDataSize (source) / Depth;
      const size_t srcSlicePix = Width * Height;
      const uint8* srcData = (uint8*)source->GetImageData();
      const uint8* srcAlpha = source->GetAlpha();

      for (d = 0; d < Depth; d++)
      {
	memcpy (resizeScrap->GetImagePtr(), srcData + (d * srcSliceSize), 
	  srcSliceSize);
	if (srcAlpha != 0)
	  memcpy (resizeScrap->GetAlphaPtr(), srcAlpha + (d * srcSlicePix), 
	    srcSlicePix);
	if ((newwidth != Width) || (newheight != Height))
	  resizedSlices.Push (Rescale2D (resizeScrap, newwidth, newheight));
	else
	{
	  csRef<csImageMemory> newImage;
	  newImage.AttachNew (new csImageMemory (resizeScrap));
	  resizedSlices.Push (newImage);
	}
      }
    }

    const size_t dstSliceSize = csImageTools::ComputeDataSize (newImage) / newdepth;
    const size_t dstSlicePix = newwidth * newheight;
    uint8* dstData = (uint8*)newImage->GetImagePtr();
    uint8* dstAlpha = newImage->GetAlphaPtr();

    for (d = 0; d < newdepth; d++)
    {
      uint srcSlice = (dz * d) >> 16;
      memcpy (dstData + (d * dstSliceSize), resizedSlices[srcSlice]->GetImageData(), 
	dstSliceSize);
      if (dstAlpha != 0)
	memcpy (dstAlpha + (d * dstSlicePix), resizedSlices[srcSlice]->GetAlpha(), 
	  dstSlicePix);
    }
  }
  return newImage;
}

//---------------------- Helper functions ---------------------------

#define MIPMAP_NAME	mipmap_0
#define MIPMAP_LEVEL	0
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_t
#define MIPMAP_LEVEL	0
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_p
#define MIPMAP_LEVEL	0
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_pt
#define MIPMAP_LEVEL	0
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_a
#define MIPMAP_LEVEL	0
#define MIPMAP_ALPHA
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1
#define MIPMAP_LEVEL	1
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_t
#define MIPMAP_LEVEL	1
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_p
#define MIPMAP_LEVEL	1
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_pt
#define MIPMAP_LEVEL	1
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_a
#define MIPMAP_LEVEL	1
#define MIPMAP_ALPHA
#include "mipmap.inc"

//-----------------------------------------------------------------------------

csRef<iImage> csImageManipulate::Mipmap2D (iImage* source, int steps, 
    csRGBpixel* transp)
{
  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  if ((Width == 1) && (Height == 1)) return source;

  csRef<csImageMemory> nimg;
  csRef<iImage> simg = source;

  int cur_w = Width;
  int cur_h = Height;

  while (steps && !((cur_w == 1) && (cur_h == 1)) )
  {
    const int newW = MAX(1, cur_w >> 1);
    const int newH = MAX(1, cur_h >> 1);
    
    nimg.AttachNew (new csImageMemory (newW, newH, simg->GetFormat()));

    csRGBpixel *mipmap = new csRGBpixel [newW * newH];
    uint8* Alpha = nimg->GetAlphaPtr();

    int transpidx = -1;
    if (transp && simg->GetPalette ())
      transpidx = csImageTools::ClosestPaletteIndex (simg->GetPalette(), 
      *transp);

    switch (simg->GetFormat() & CS_IMGFMT_MASK)
    {
      case CS_IMGFMT_NONE:
      case CS_IMGFMT_PALETTED8:
	if (simg->GetImageData())
	  if (transpidx < 0)
	    mipmap_1_p (cur_w, cur_h, 
	      (uint8 *)simg->GetImageData(), mipmap, simg->GetPalette());
	  else
	    mipmap_1_pt (cur_w, cur_h, (uint8*)simg->GetImageData(), mipmap,
	      simg->GetPalette(), transpidx);
	nimg->ConvertFromRGBA (mipmap);
	if (simg->GetAlpha ())
	{
	  mipmap_1_a (cur_w, cur_h, (uint8 *)simg->GetAlpha (), Alpha);
	}
	break;
      case CS_IMGFMT_TRUECOLOR:
	if (!transp)
	  mipmap_1 (cur_w, cur_h, (csRGBpixel *)simg->GetImageData (), mipmap);
	else
	  mipmap_1_t (cur_w, cur_h, (csRGBpixel *)simg->GetImageData (), mipmap, *transp);
	nimg->ConvertFromRGBA (mipmap);
	break;
    }

    simg = nimg;
    steps--;
    cur_w = nimg->GetWidth();
    cur_h = nimg->GetHeight();
  }

  return nimg;
}

csRef<iImage> csImageManipulate::Mipmap3D (iImage* source, int step, 
    csRGBpixel* transp)
{
  const int nw = source->GetWidth() >> step;
  const int nh = source->GetHeight() >> step;
  const int nd = source->GetDepth() >> step;
  // @@@ Will look ugly...
  return Rescale (source, MAX (nw, 1), MAX (nh, 1), MAX (nd, 1));
}

csRef<iImage> csImageManipulate::Mipmap (iImage* source, int steps, 
    csRGBpixel* transp)
{
  if (steps == 0)
    return source;

  if (source->GetImageType() == csimg3D)
    return Mipmap3D (source, steps, transp);
  else
    return Mipmap2D (source, steps, transp);
}

csRef<iImage> csImageManipulate::Blur (iImage* source, csRGBpixel* transp)
{
  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  csRef<csImageMemory> nimg;
  nimg.AttachNew (new csImageMemory (source->GetWidth(), 
    source->GetHeight(), source->GetFormat()));

  csRGBpixel *mipmap = new csRGBpixel [Width * Height];
  uint8* Alpha = nimg->GetAlphaPtr();

  int transpidx = -1;
  if (transp && source->GetPalette ())
      transpidx = csImageTools::ClosestPaletteIndex (source->GetPalette(), 
      *transp);

  switch (source->GetFormat() & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
    case CS_IMGFMT_PALETTED8:
      if (source->GetImageData())
	if (transpidx < 0)
	  mipmap_0_p (source->GetWidth(), source->GetHeight(), 
	    (uint8 *)source->GetImageData(), mipmap, source->GetPalette());
	else
	  mipmap_0_pt(source->GetWidth(), source->GetHeight(), 
	    (uint8*)source->GetImageData(), mipmap, source->GetPalette(),
	    transpidx);
      nimg->ConvertFromRGBA (mipmap);
      if (source->GetAlpha())
      {
	mipmap_0_a (source->GetWidth(), source->GetHeight(), 
	  (uint8 *)source->GetAlpha(), Alpha);
      }
      break;
    case CS_IMGFMT_TRUECOLOR:
      if (!transp)
	mipmap_0 (source->GetWidth(), source->GetHeight(), 
	  (csRGBpixel *)source->GetImageData(), mipmap);
      else
	mipmap_0_t (source->GetWidth(), source->GetHeight(), 
	  (csRGBpixel *)source->GetImageData(), mipmap, *transp);
      nimg->ConvertFromRGBA (mipmap);
      break;
  }

  return nimg;
}

csRef<iImage> csImageManipulate::Crop (iImage* source, int x, int y, 
    int width, int height)
{
  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  if (x+width > Width || y+height > Height) return 0;
  csRef<csImageMemory> nimg;
  nimg.AttachNew (new csImageMemory (width, height, source->GetFormat()));

  int i;
  if (source->GetAlpha())
  {
    for ( i=0; i<height; i++ )
      memcpy (nimg->GetAlphaPtr() + i*width, 
	source->GetAlpha() + (i+y)*Width + x, width);
  }

  if (source->GetPalette())
  {
    memcpy (nimg->GetPalettePtr(), source->GetPalette(), 
      256 * sizeof (csRGBpixel));
  }

  if (source->GetImageData())
  {      
    switch (source->GetFormat() & CS_IMGFMT_MASK)
    {
      case CS_IMGFMT_NONE:
        break;
      case CS_IMGFMT_PALETTED8:
        for ( i=0; i<height; i++ )
          memcpy ( (uint8*)nimg->GetImagePtr() + i*width, 
	    (uint8*)source->GetImageData() + (i+y)*Width + x, width);
        break;
      case CS_IMGFMT_TRUECOLOR:
        for ( i=0; i<height; i++ )
          memcpy ((csRGBpixel*)nimg->GetImagePtr() + i*width, 
	    (csRGBpixel*)source->GetImageData() + (i+y)*Width + x, 
	    width * sizeof (csRGBpixel));
        break;
    }
  }

  return nimg;
}

csRef<iImage> csImageManipulate::Sharpen (iImage* source, int strength, 
    csRGBpixel* transp)
{
/*

  How it works:

  The algorithm is known as 'Unsharp Mask'. 
  Expressed as a formula:

  sharpened image = original image + 
    strength * (original image - smoothed image)

  You may try
    http://www.dai.ed.ac.uk/HIPR2/unsharp.htm
  for some more information.

*/

  if (strength <= 0) 
    return source;

  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  // we need an RGB version of ourselves
  csRef<iImage> original; 
  if ((source->GetFormat() & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
  {
    csImageMemory* nimg = new csImageMemory (source, CS_IMGFMT_TRUECOLOR);
    nimg->SetFormat (CS_IMGFMT_TRUECOLOR | 
      (source->GetAlpha() ? CS_IMGFMT_ALPHA : 0));
    original.AttachNew (nimg);
  }
  else
    original = source;
  csRef<iImage> blurry = Blur (original, transp);
  
  csRGBpixel *result = new csRGBpixel [Width * Height];
  csRGBpixel *src_o = (csRGBpixel*)original->GetImageData ();
  csRGBpixel *src_b = (csRGBpixel*)blurry->GetImageData ();
  csRGBpixel *dest = result;
 
  for (int n = Width * Height; n > 0; n--)
  {
    int v;
    #define DO(comp)  \
      v = src_o->comp + ((strength * (src_o->comp - src_b->comp)) >> 8);  \
      dest->comp = (v>255)?255:((v<0)?0:v)

    DO(red);
    DO(green);
    DO(blue);
    DO(alpha);

    #undef DO

    dest++;
    src_o++;
    src_b++;
  }

  csRef<csImageMemory> resimg;
  resimg.AttachNew (new csImageMemory (source->GetWidth(), source->GetHeight(),
    result, true));

  return resimg;
}

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
    destCoords[D] = coord / dim[D];
    CoordSplitter<D-1>::Split (destCoords, coord % dim[D], dim);
  }
};
CS_SPECIALIZE_TEMPLATE struct CoordSplitter<0>
{
  static inline void Split (int* destCoords, size_t coord, const int* dim)
  {
    CS_ASSERT (dim[0] == 1);
    destCoords[0] = coord;
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

template<int D> struct NeighDivCompute
{
  static inline int Po3() { return NeighDivCompute<D-1>::Po3() * 3; }
  static inline void Compute (int* neighDiv)
  {
    neighDiv[D] = Po3();
    NeighDivCompute<D-1>::Compute (neighDiv);
  }
};
CS_SPECIALIZE_TEMPLATE struct NeighDivCompute<0>
{
  static inline int Po3() { return 1; }
  static inline void Compute (int* neighDiv)
  {
    neighDiv[0] = 1;
  }
};

template<int Dimensions>
class KeycolorRenderer
{
public:
  static void RenderToAlpha (csRGBpixel* dst, const csRGBpixel* src,
    const csRGBpixel& transpColor, const csRGBpixel& fillColor,
    const int sizes[Dimensions])
  {
    size_t pixNum = ArrayMul<Dimensions-1>::Mul (sizes); //sizes[0];
    int dimPixels[Dimensions];
    DimPixCompute<Dimensions-1>::Compute (dimPixels, sizes);
    int neighDiv[Dimensions];
    NeighDivCompute<Dimensions-1>::Compute (neighDiv);
    /*dimPixels[0] = 1;
    neighDiv[0] = 1;
    uint i;
    for (i = 1; i < Dimensions; i++) 
    {
      pixNum *= sizes[i];
      dimPixels[i] = dimPixels[i-1] * sizes[i-1];
      neighDiv[i] = neighDiv[i-1] * 3;
    }*/
    const int neighCount = neighDiv[Dimensions-1] * 3;
    const int neighMiddle = neighCount / 2;
    const csRGBpixel* srcStart = src;

    for (size_t p = 0; p < pixNum; p++)
    {
      if (*src == transpColor)
      {
	//size_t c = p;
	int coord[Dimensions];
	/*for (i = Dimensions; i-- > 0;) 
	{
	  coord[i] = c / dimPixels[i];
	  c %= dimPixels[i];
	}*/
	CoordSplitter<Dimensions-1>::Split (coord, p, dimPixels);
	int mr = 0, mg = 0, mb = 0;
	int neighPix = 0;
	for (int n = 0; n < neighCount; n++)
	{
	  if (n == neighMiddle) continue;
	  uint newpx = 0;
	  for (int d = 0; d < Dimensions; d++)
	  {
	    int delta = ((n / neighDiv[d]) % 3) - 1;
	    int nc = coord[d] + delta;
	    if (nc < 0)
	      nc = sizes[d] - 1;
	    else if (nc >= sizes[d])
	      nc = 0;
	    newpx += nc * dimPixels[d];
	  }
	  if (srcStart[newpx] != transpColor)
	  {
	    mr += srcStart[newpx].red;
	    mg += srcStart[newpx].green;
	    mb += srcStart[newpx].blue;
	    neighPix++;
	  }
	}
	if (neighPix != 0)
	{
	  (dst++)->Set (mr / neighPix, mg / neighPix, mb / neighPix, 0);
	}
	else
	{
	  *dst = fillColor;
	  /*
	    Drop the alpha of the transparent pixels to 0, but leave the alpha
	    of non-keycolored ones as is. Keycolor only makes transparent, but
	    doesn't mean the rest of the image isn't.
	  */
	  dst->alpha = 0;
	  dst++;
	}
	src++;
      }
      else
	*dst++ = *src++;
    }
  }
};

csRef<iImage> csImageManipulate::RenderKeycolorToAlpha (iImage* source, 
							const csRGBpixel& transpColor, 
							const csRGBpixel& fillColor)
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
  if ((source->GetImageType() == csimg3D) && (source->GetDepth() > 1))
  {
    const int sizes[3] = {image->GetWidth(), image->GetHeight(), image->GetDepth()};
    KeycolorRenderer<3>::RenderToAlpha ((csRGBpixel*)result->GetImagePtr(), 
      (csRGBpixel*)image->GetImageData(), transpColor, fillColor, 
      sizes);
  }
  else
  {
    const int sizes[2] = {image->GetWidth(), image->GetHeight()};
    KeycolorRenderer<2>::RenderToAlpha ((csRGBpixel*)result->GetImagePtr(), 
      (csRGBpixel*)image->GetImageData(), transpColor, fillColor, 
      sizes);
  }

  if ((source->GetFormat() & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
    result->SetFormat ((source->GetFormat() & CS_IMGFMT_MASK) | CS_IMGFMT_ALPHA);

#ifdef KEYCOLOR_DEBUG
  csDebugImageWriter::DebugImageWrite (result, "renderkeycolor_%p.png", source);
#endif
  return result;
}
