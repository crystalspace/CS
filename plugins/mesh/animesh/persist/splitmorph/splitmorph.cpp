/*
  Copyright (C) 2011 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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

#include "imap/services.h"
#include "imesh/animesh.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "cstool/animeshtools.h"
#include "csutil/scf.h"
#include "csutil/stringquote.h"

#include "splitmorph.h"

CS_PLUGIN_NAMESPACE_BEGIN (SplitMorph)
{
  SCF_IMPLEMENT_FACTORY (SplitMorphLoader);

  SplitMorphLoader::SplitMorphLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  static const char* msgidFactory = "crystalspace.mesh.loader.factory.animesh.splitmorph";

  bool SplitMorphLoader::Initialize (iObjectRegistry* registry)
  {
    this->registry = registry;
    synldr = csQueryRegistry<iSyntaxService> (registry);
    InitTokenTable (xmltokens);

    return true;
  }

  csPtr<iBase> SplitMorphLoader::Parse (iDocumentNode* node,
					iStreamSource* ssource,
					iLoaderContext* ldr_context,
					iBase* context)
  {
    csString path, realPath, file, name, mask;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_VFSPATH:
	path = child->GetContentsValue ();
	break;

      case XMLTOKEN_REALPATH:
	realPath = child->GetContentsValue ();
	break;

      case XMLTOKEN_BASEFILE:
	file = child->GetContentsValue ();
	break;

      case XMLTOKEN_MESHNAME:
	name = child->GetContentsValue ();
	break;

      case XMLTOKEN_MASK:
	mask = child->GetContentsValue ();
	break;

      default:
        synldr->ReportBadToken (child);
        return csPtr<iBase> (nullptr);
      }
    }

    // Check that the parameters are invalid
    if (file.IsEmpty ())
    {
      synldr->ReportError (msgidFactory, node, "No base file provided!");
      return csPtr<iBase> (nullptr);
    }

    // If no name is provided then use the one from the mesh wrapper
    if (name.IsEmpty ())
    {
      if (node->GetParent ())
	name = node->GetParent ()->GetAttributeValue ("name");
      else name = "splitmorph";
    }

    // Check for a temporary path to be mounted
    if (!realPath.IsEmpty ())
    {
      csRef<iVFS> vfs =  csQueryRegistry<iVFS> (registry);
      if (!vfs->Mount ("/tmp/splitmorph", realPath))
      {
	synldr->ReportError (msgidFactory, node, "Could not mount real path %s!",
			     CS::Quote::Single (realPath));
	return csPtr<iBase> (nullptr);
      }

      path = "/tmp/splitmorph/";
    }

    // Import the animesh
    csRef<CS::Mesh::iAnimatedMeshFactory> meshFactory =
      CS::Mesh::AnimatedMeshTools::ImportSplitMorphMesh (registry, path, file, name, mask);
    if (!meshFactory)
      return csPtr<iBase> (nullptr);

    // Unmount the temporary path
    if (!realPath.IsEmpty ())
    {
      csRef<iVFS> vfs =  csQueryRegistry<iVFS> (registry);
      vfs->Unmount ("/tmp/splitmorph", nullptr);
    }

    return csPtr<iBase> (scfQueryInterface<iBase> (meshFactory));
  }

}
CS_PLUGIN_NAMESPACE_END (SplitMorph)
