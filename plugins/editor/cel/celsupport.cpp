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

#include <csutil/objreg.h>
#include <csutil/plugldr.h>

#include "celsupport.h"

#include "celtool/initapp.h"
#include "celtool/persisthelper.h"
#include "physicallayer/pl.h"
#include "physicallayer/propfact.h"
#include "physicallayer/propclas.h"
#include "physicallayer/entity.h"
#include "physicallayer/persist.h"
#include "behaviourlayer/bl.h"

using namespace CS {
namespace EditorApp {;

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

SCF_IMPLEMENT_FACTORY(CELSupport)

CELSupport::CELSupport (iBase* parent)
  : scfImplementationType (this, parent)
{
}

CELSupport::~CELSupport ()
{
}


bool CELSupport::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;

  // TODO: Find out why SetupCelPluginDirs doesn't work
  // Add path to CEL plugins
  iSCF::SCF->ScanPluginsPath (getenv ("CEL"));
  
  //celInitializer::SetupCelPluginDirs (object_reg);
  pl = csQueryRegistryOrLoad<iCelPlLayer> (object_reg, "cel.physicallayer");
  bl = csQueryRegistryOrLoad<iCelBlLayer> (object_reg, "cel.behaviourlayer.test");

  pl->RegisterBehaviourLayer (bl);
  
  /*
  csPluginLoader* plugldr = new csPluginLoader (object_reg);
  plugldr->RequestPlugin ("cel.physicallayer", "iCelPlLayer");
  plugldr->RequestPlugin ("cel.behaviourlayer.test", "iCelBlLayer.Test");
  plugldr->RequestPlugin ("cel.persistence.xml", "iCelPersistence");

  if (!plugldr->LoadPlugins ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.editor.plugin.cel.support",
              "Could not load CEL plugins!");
    return false;
  }

  delete plugldr;

  pl = csQueryRegistry<iCelPlLayer> (object_reg);
  if (!pl)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.editor.plugin.cel.support",
              "CEL physical layer missing!");
    return false;
  }

  bl = csQueryRegistryTagInterface<iCelBlLayer> (
      object_reg, "iCelBlLayer.Test");
  if (!bl)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.editor.plugin.cel.support",
              "CEL behavior layer missing!");
    return false;
  }
  */

  editor = csQueryRegistry<iEditor> (object_reg);
  if (!editor)
    return false;

  engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
    return false;

  editor->AddMapListener (this);

  /// TODO: Create entity/entity template/behavior browser and generate properties

  return true;
}

void CELSupport::OnMapLoaded (const char* path, const char* filename)
{
  /// TODO: Update browsers
}

void CELSupport::OnLibraryLoaded (const char* path, const char* filename, iCollection* collection)
{
}


}
CS_PLUGIN_NAMESPACE_END(CSE)
