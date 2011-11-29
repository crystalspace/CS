/*
    Copyright (C) 2011 by Michael Gist

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
#include "fileloader.h"

using namespace CS::Resource;

CS_PLUGIN_NAMESPACE_BEGIN(csloader)
{

SCF_IMPLEMENT_FACTORY (FileLoader)

static int GeneratedNameVal = 0;

FileLoader::FileLoader (iBase* parent)
: scfImplementationType (this, parent)
{
}

FileLoader::~FileLoader ()
{
}

bool FileLoader::Initialize (iObjectRegistry* obj_reg)
{
  return ResourceManager::Initialize (obj_reg);
}

csRef<iLoadingResource> FileLoader::Get (TypeID type, const char* name)
{
  // We only handle file types in this loader.
  if (type == GetTypeID ("file"))
  {
    // Load the file as a document.
    csRef<iDocument> doc = LoadDocument (name);
    if (!doc.IsValid ()) return 0;

    // Iterate over all nodes and load each one.
    csRef<iDocumentNodeIterator> nodes = doc->GetRoot ()->GetNodes ();
    while (nodes->HasNext ())
    {
      csRef<iDocumentNode> node = nodes->Next ();

      // Get or generate the name of the node.
      csString name = node->GetAttributeValue ("name");
      if (name.IsEmpty ())
      {
        name = "FileLoaderGenName";
        name.Append (GeneratedNameVal++);
      }

      // Get the type of the node.
      TypeID nodeType = GetTypeID (node->GetValue ());

      // Load the node.
      ResourceManager::Load (nodeType, name, node);
    }
  }

  return ResourceManager::Get (type, name);
}

}
CS_PLUGIN_NAMESPACE_END(loaders)
