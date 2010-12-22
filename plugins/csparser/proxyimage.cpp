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

#include "csutil/stringquote.h"

#include "iutil/databuff.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

#include "csthreadedloader.h"
#include "loadtex.h"
#include "proxyimage.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  iImage* ProxyImage::GetProxiedImage() const
  {
    if (!proxiedImage.IsValid ())
    {
      csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D>(object_reg);
      iTextureManager *texman = g3d->GetTextureManager();

      int Format = texman->GetTextureFormat ();

      csRef<iImage> img;
      csThreadedLoader* cstldr = dynamic_cast<csThreadedLoader*>((iThreadedLoader*)loader);
      csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(object_reg);
      csRef<iThreadReturn> ret = csPtr<iThreadReturn>(new csLoaderReturn(tm));
      cstldr->LoadImageTC (ret, false, cstldr->GetVFS()->GetCwd(), filename, Format, false);
      img = scfQueryInterface<iImage>(ret->GetResultRefPtr());

      if (!img.IsValid())
      {
        csRef<iReporter> reporter = csQueryRegistry<iReporter>(object_reg);
        reporter->ReportWarning (
          "crystalspace.maploader.parse.texture",
          "Couldn't load image %s, using error texture instead!",
          CS::Quote::Single (filename.GetData()));
        img = GenerateErrorTexture (32, 32);
        CS_ASSERT(img.IsValid());
      }
      proxiedImage = img;
    }
    return proxiedImage;
  }

}
CS_PLUGIN_NAMESPACE_END(csparser)
