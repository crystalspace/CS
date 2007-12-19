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

#include "cssysdef.h"

#include "csloader.h"
#include "proxyimage.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  iImage* ProxyImage::GetProxiedImage() const
  {
    if (!proxiedImage.IsValid ())
    {
      iTextureManager *tm = loader->G3D->GetTextureManager();

      int Format = tm->GetTextureFormat ();
      csRef<iImage> img = loader->LoadImage (filename, Format);
      if (!img)
      {
        loader->ReportWarning (
          "crystalspace.maploader.parse.texture",
          "Couldn't load image '%s', using error texture instead!",
          filename.GetData());
        img = csLoader::GenerateErrorTexture (32, 32);
        CS_ASSERT(img);
      }
      proxiedImage = img;
    }
    return proxiedImage;
  }

}
CS_PLUGIN_NAMESPACE_END(csparser)
