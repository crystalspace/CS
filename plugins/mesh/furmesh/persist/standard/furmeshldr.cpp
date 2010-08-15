/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/object.h"
#include "imesh/watermesh.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

#include "furmeshldr.h"

using namespace CS::Plugins::FurMeshLoader;


SCF_IMPLEMENT_FACTORY (FurMeshLoader)
SCF_IMPLEMENT_FACTORY (FurMeshSaver)

//---------------------------------------------------------------------------

FurMeshLoader::FurMeshLoader (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

FurMeshLoader::~FurMeshLoader ()
{
}

bool FurMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  FurMeshLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
    synldr->ReportError ( \
	"crystalspace.watermeshloader.parse.unknownfactory", \
	child, "Specify the factory first!"); \
    return 0; \
  }


csPtr<iBase> FurMeshLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  return 0;
}

//---------------------------------------------------------------------------

FurMeshSaver::FurMeshSaver (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

FurMeshSaver::~FurMeshSaver ()
{
}

bool FurMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  FurMeshSaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool FurMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  return true;
}
