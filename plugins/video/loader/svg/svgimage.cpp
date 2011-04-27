/*
    SVG image loader
    Copyright (C) 2007 Frank Richter

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
//#include <math.h>
//#include "cstypes.h"
//#include "csgeom/math3d.h"
//#include "csutil/csendian.h"
//#include "csgfx/rgbpixel.h"
//#include "csutil/databuf.h"

#include "csutil/csstring.h"
#include "csutil/scopeddelete.h"

#include "vgvm/vgvm.h"
#include "vgvm/contexts/cairo.h"
#include "vgvm/compilers/svg.h"

#include "svgimage.h"



CS_PLUGIN_NAMESPACE_BEGIN(SVGImage)
{

SCF_IMPLEMENT_FACTORY (SVGImageIO)

#define SVG_MIME "image/svg+xml"
#define SVG_EXT	 "svg"

static const iImageIO::FileFormatDescription formatlist[] =
{
  { SVG_MIME, "32 bit, RGBA", CS_IMAGEIO_LOAD}
};

//---------------------------------------------------------------------------

SVGImageIO::SVGImageIO (iBase *pParent) : 
  scfImplementationType (this, pParent)
{
  formats.Push (&formatlist[0]);
}

const csImageIOFileFormatDescriptions& SVGImageIO::GetDescription ()
{
  return formats;
}

csPtr<iImage> SVGImageIO::Load (iDataBuffer* buf, int iFormat)
{
  SVGImage* i = new SVGImage (iFormat);
  if (i && !i->Load (buf->GetUint8(), buf->GetSize()))
  {
    delete i;
    return 0;
  }
  return csPtr<iImage> (i);
}

//---------------------------------------------------------------------------

struct AllocatorCS : public vgvm::MemoryAllocator
{
  void* Malloc (size_t size) { return cs_malloc (size); }
  void Free (void* p) { cs_free (p); }
  void* Realloc (void* p, size_t size) { return cs_realloc (p, size); }
};

bool SVGImage::Load (uint8* iBuffer, size_t iSize)
{
  AllocatorCS allocator;
  CS::Utility::ScopedDelete<vgvm::cairo::CairoContext> context ( 
    vgvm::cairo::createCairoContext (&allocator));
    
  CS::Utility::ScopedDelete<vgvm::Program> prog (context->createProgram ());
  {
    csString bufferStr ((char*)iBuffer, iSize);
    if (!vgvm::svg::compileSVG (bufferStr, prog))
      return false;
  }
    
  uint width = 256;
  uint height = 256;
  uint8* buf_bgra = (uint8*)cs_malloc (width * height * 4);
  context->setOutput (buf_bgra, width, height, true);
  context->executeProgram (prog);
  
  csRGBpixel* buf = new csRGBpixel[width * height];
  size_t numPix = width * height;
  const uint8* sp = buf_bgra;
  csRGBpixel* dp = buf;
  while (numPix-- > 0)
  {
    dp->blue  = *sp++;
    dp->green = *sp++;
    dp->red   = *sp++;
    dp->alpha = *sp++;
    dp++;
  }
  cs_free (buf_bgra);
  
  Format |= ~CS_IMGFMT_ALPHA;
  SetDimensions (width, height);
  ConvertFromRGBA (buf);

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(SVGImage)
