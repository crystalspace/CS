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

#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#include "persist.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  SCF_IMPLEMENT_FACTORY(Persist)

  Persist::Persist (iBase* pParent)
  : scfImplementationType (this, pParent)
  {
  }

  Persist::~Persist ()
  {
  }

  bool Persist::Initialize (iObjectRegistry* objectReg_)
  {
    objectReg = objectReg_;
    syntaxService = csQueryRegistry<iSyntaxService> (objectReg);

    imageLoader = csQueryRegistry<iImageIO> (objectReg);
    if (!imageLoader)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.engine.persist", "Failed to find an image loader!");
      return false;
    }

    InitTokenTable (xmltokens);
    return true;
  }

  csPtr<iResource> Persist::Load (iDocumentNode* node)
  {
    csRef<iResource> resource;

    if (resource = LoadImage (node)) return resource;

    return 0;
  }

  csPtr<iResource> Persist::Load (iDataBuffer* buf)
  {
    csRef<iResource> resource;

    if (resource = LoadImage (buf)) return resource;

    return 0;
  }

  bool Persist::Save (iResource* resource, iDocumentNode* node)
  {
    if (SaveImage (resource, node)) return true;
    return false;
  }

  csPtr<iDataBuffer> Persist::Save (iResource* resource)
  {
    csRef<iDataBuffer> buf;

    if (buf = SaveImage (resource)) return buf;

    return 0;
  }
}
CS_PLUGIN_NAMESPACE_END (Engine)
