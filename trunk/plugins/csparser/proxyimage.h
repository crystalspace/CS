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

#ifndef __PROXYIMAGE_H__
#define __PROXYIMAGE_H__

#include "iutil/databuff.h"

struct iBase;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  class ProxyImage : public scfImplementation1<ProxyImage, iImage>
  {
    csString filename;
    
    mutable csRef<iThreadedLoader> loader;
    mutable csRef<iImage> proxiedImage;
  public:
    ProxyImage (iThreadedLoader* loader, const char* fn, iObjectRegistry* object_reg) : 
      scfImplementationType (this), filename (fn),
      loader (loader), object_reg(object_reg) {}
      
    iObjectRegistry* object_reg;
    iImage* GetProxiedImage() const;
    
    const void *GetImageData ()
    { return GetProxiedImage()->GetImageData (); }
    int GetWidth() const
    { return GetProxiedImage()->GetWidth (); }
    int GetHeight() const
    { return GetProxiedImage()->GetHeight (); }
    int GetDepth() const
    { return GetProxiedImage()->GetDepth (); }  

    void SetName (const char *iName)
    { GetProxiedImage()->SetName (iName); }
    const char *GetName () const
    { return GetProxiedImage()->GetName (); }
  
    int GetFormat () const
    { return GetProxiedImage()->GetFormat (); }
    const csRGBpixel* GetPalette ()
    { return GetProxiedImage()->GetPalette (); }
    const uint8* GetAlpha ()
    { return GetProxiedImage()->GetAlpha (); }
  
    bool HasKeyColor () const
    { return GetProxiedImage()->HasKeyColor (); }
  
    void GetKeyColor (int &r, int &g, int &b) const
    { GetProxiedImage()->GetKeyColor (r, g, b); }
  
    uint HasMipmaps () const
    { return GetProxiedImage()->HasMipmaps (); }
    csRef<iImage> GetMipmap (uint num)
    { return GetProxiedImage()->GetMipmap (num); }
    
    const char* GetRawFormat() const
    { return GetProxiedImage()->GetRawFormat (); }
    csRef<iDataBuffer> GetRawData() const
    { return GetProxiedImage()->GetRawData (); }
    csImageType  GetImageType() const
    { return GetProxiedImage()->GetImageType (); }
    uint HasSubImages() const
    { return GetProxiedImage()->HasSubImages (); }
    csRef<iImage> GetSubImage (uint num)
    { return GetProxiedImage()->GetSubImage (num); }

    const char* GetCookedImageFormat()
    { return GetProxiedImage()->GetCookedImageFormat (); }
    csRef<iDataBuffer> GetCookedImageData()
    { return GetProxiedImage()->GetCookedImageData (); }
  };
}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __PROXYIMAGE_H__
