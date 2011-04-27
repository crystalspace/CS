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

#ifndef __CS_SVGIMAGE_H__
#define __CS_SVGIMAGE_H__

#include "igraphic/imageio.h"
#include "iutil/comp.h"
#include "csgfx/imagememory.h"

CS_PLUGIN_NAMESPACE_BEGIN(SVGImage)
{

class SVGImageIO : public scfImplementation2<SVGImageIO,
                                             iImageIO,
                                             iComponent>
{
 protected:
  csImageIOFileFormatDescriptions formats;

 public:
  SVGImageIO (iBase *pParent);
  virtual ~SVGImageIO () {}

  virtual const csImageIOFileFormatDescriptions& GetDescription ();
  virtual csPtr<iImage> Load (iDataBuffer* buf, int iFormat);
  virtual csPtr<iDataBuffer> Save (iImage *image, const char *mime = 0,
    const char* extraoptions = 0)
  { return 0; }
  virtual csPtr<iDataBuffer> Save (iImage *image,
    iImageIO::FileFormatDescription *format = 0,
    const char* extraoptions = 0)
  { return 0; }

  virtual bool Initialize (iObjectRegistry*) { return true; }
};

class SVGImage : public csImageMemory
{
public:
  SVGImage (int iFormat) : csImageMemory (iFormat) { };
  bool Load (uint8* iBuffer, size_t iSize);
};

}
CS_PLUGIN_NAMESPACE_END(SVGImage)

#endif // __CS_SVGIMAGE_H__
