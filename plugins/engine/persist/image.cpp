/*
  Copyright (C) 2011 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#include "igraphic/image.h"
#include "ivaria/reporter.h"

#include "persist.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  csPtr<iResource> Persist::LoadImage (iDocumentNode* node)
  {
    return 0;
  }

  csPtr<iResource> Persist::LoadImage (iDataBuffer* buf)
  {
    if (!buf || !buf->GetSize ())
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.engine.persist", "Invalid data buffer!");
      return 0;
    }

    // we don't use csRef because we need to return an Increfed object later
    csRef<iImage> image (imageLoader->Load (buf, CS_IMGFMT_ANY));
    if (!image)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.engine.persist", "Unknown image format!");
      return 0;
    }

    return csPtr<iResource> (image);
  }

  bool Persist::SaveImage (iResource* resource, iDocumentNode* node)
  {
    csRef<iImage> image = scfQueryInterface<iImage> (resource);
    if (image.IsValid ())
    {
      // Only do something if there's raw data to be written.
      if (image->GetRawData ())
      {
        // TODO: Write out the raw data.
      }

      return true;
    }

    return false;
  }

  csPtr<iDataBuffer> Persist::SaveImage (iResource* resource)
  {
    csRef<iImage> image = scfQueryInterface<iImage> (resource);
    if (image.IsValid ())
    {
      return imageLoader->Save (image);
    }

    return 0;
  }
}
CS_PLUGIN_NAMESPACE_END (Engine)
