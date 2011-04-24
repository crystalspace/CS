/*
    Copyright (C) 2007 by Seth Yastrov

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

#include <cssysdef.h>
#include "csutil/scf.h"

#include <csutil/objreg.h>
#include <iutil/object.h>

#include <iengine/sector.h>
#include <iengine/mesh.h>
#include <iengine/material.h>
#include <iengine/collection.h>

#include <wx/image.h>

#include "ieditor/editorobject.h"

#include "csobjectmaplistener.h"

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

SCF_IMPLEMENT_FACTORY(CSObjectMapListener)

#include "data/editor/images/meshIcon.xpm"
#include "data/editor/images/lightIcon.xpm"
#include "data/editor/images/defaultIcon.xpm"

CSObjectMapListener::CSObjectMapListener (iBase* parent)
  : scfImplementationType (this, parent)
{
  meshBmp = new wxBitmap (meshIcon_xpm);
  lightBmp = new wxBitmap (lightIcon_xpm);
  defaultBmp = new wxBitmap (defaultIcon_xpm);
}

CSObjectMapListener::~CSObjectMapListener ()
{
  delete meshBmp;
  delete lightBmp;
  delete defaultBmp;
}

bool CSObjectMapListener::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;

  editor = csQueryRegistry<iEditor> (object_reg);
  if (!editor)
    return false;

  engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
    return false;
  
  objects = editor->GetObjects ();
  editor->AddMapListener (this);
  
  return true;
}

void CSObjectMapListener::OnMapLoaded (const char* path, const char* filename)
{
  editor->GetSelection ()->Clear ();
  editor->GetObjects ()->Clear ();

  iSectorList* sectorList = engine->GetSectors ();
  for (int i = 0; i < sectorList->GetCount (); i++)
  {
    iSector* sector = sectorList->Get (i);
    csRef<iEditorObject> editorObject (editor->CreateEditorObject (sector, defaultBmp));
    objects->Add (editorObject);
  }

  iMeshList* meshList = engine->GetMeshes ();
  for (int i = 0; i < meshList->GetCount (); i++)
  {
    iMeshWrapper* mesh = meshList->Get (i);
    csRef<iEditorObject> editorObject (editor->CreateEditorObject (mesh, meshBmp));
    objects->Add (editorObject);
  }

  csRef<iLightIterator> lightIterator = engine->GetLightIterator ();
  while (lightIterator->HasNext ())
  {
    iLight* light = lightIterator->Next ();
    csRef<iEditorObject> editorObject (editor->CreateEditorObject (light, lightBmp));
    objects->Add (editorObject);
  }

  iMeshFactoryList* meshFactList = engine->GetMeshFactories ();
  for (int i = 0; i < meshFactList->GetCount (); i++)
  {
    csRef<iEditorObject> editorObject (editor->CreateEditorObject (meshFactList->Get (i), defaultBmp));
    objects->Add (editorObject);
  }

  iMaterialList* materialList = engine->GetMaterialList ();
  for (int i = 0; i < materialList->GetCount (); i++)
  {
    csRef<iEditorObject> editorObject (editor->CreateEditorObject (materialList->Get (i), defaultBmp));
    objects->Add (editorObject);
  }
}

 void CSObjectMapListener::OnLibraryLoaded(const char* path, const char* filename, iCollection* collection)
{
  // TODO: add methods in iCollection to iterate on its objects
  iMeshFactoryList* meshFactList = engine->GetMeshFactories ();
  for (int i = 0; i < meshFactList->GetCount (); i++)
  {
    if (!collection->FindMeshFactory (meshFactList->Get (i)->QueryObject ()->GetName ()))
      continue;
    
    csRef<iEditorObject> editorObject (editor->CreateEditorObject (meshFactList->Get (i), defaultBmp));
    objects->Add (editorObject);
  }
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
