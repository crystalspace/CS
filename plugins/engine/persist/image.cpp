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
    /**
     * <image>/path/to/image</image>
     */

    // Sanity check.
    if (strcmp (node->GetValue (), "image") != 0)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.engine.persist",
        "Invalid document node passed to 'LoadImage' - missing 'image' root node!");
      return 0;
    }

    // Load the file.
    const char* filePath = node->GetContentsValue ();
    csRef<iDataBuffer> dataBuffer = vfs->ReadFile (filePath, false);
    if (!dataBuffer.IsValid())
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.engine.persist",
        "Invalid image path '%s'!", filePath);
      return 0;
    }

    csRef<iImage> image (imageLoader->Load (dataBuffer, CS_IMGFMT_ANY));
    if (!image)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.engine.persist", "Unknown image format!");
      return 0;
    }

    // Set the image name from the file path.
    csRef<iDataBuffer> xname = vfs->ExpandPath (filePath);
    image->SetName (**xname);

    return csPtr<iResource> (image); 
  }

  csPtr<iResource> Persist::LoadImage (iDataBuffer* buf)
  {
    if (!buf || !buf->GetSize ())
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.engine.persist", "Invalid data buffer!");
      return 0;
    }

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
      // Get a data buffer of the image.
      csRef<iDataBuffer> buf = SaveImage (resource);
      
      // If the buffer is valid, then write out the buffer.
      if (buf.IsValid ())
      {
        const char* imageName = image->GetName ();

        csRef<iFile> file = vfs->Open (imageName, VFS_FILE_WRITE);
        if (file)
        {
          file->Write (buf->GetData (), buf->GetSize ());
          file->Flush ();

          // Write the the image info to the document.
          csRef<iDocumentNode> iNode = node->CreateNodeBefore (CS_NODE_ELEMENT);
          iNode->SetValue ("image");
          iNode = iNode->CreateNodeBefore (CS_NODE_TEXT);
          iNode->SetValue (imageName);
        }
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
      return imageLoader->Save (image, "image/png");
    }

    return 0;
  }
}
CS_PLUGIN_NAMESPACE_END (Engine)
