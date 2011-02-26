/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg
              (C) 2004 by Marten Svanfeldt

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
#include "plugins/engine/3d/renderloop.h"

#include "csutil/sysfunc.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "igeom/clip2d.h"
#include "iengine/material.h"
#include "iengine/rendersteps/irsfact.h"
#include "iengine/rendersteps/igeneric.h"
#include "imap/reader.h"

#include "csgfx/rgbpixel.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/sector.h"
#include "csutil/xmltiny.h"
#include "cstool/rviewclipper.h"

//---------------------------------------------------------------------------


csRenderLoop::csRenderLoop (csEngine* engine)
  : scfImplementationType (this), engine (engine)
{
}

csRenderLoop::~csRenderLoop ()
{
}

void csRenderLoop::SelfDestruct ()
{
  engine->GetRenderLoopManager ()->Unregister ((iRenderLoop*)this);
}

void csRenderLoop::Draw (iRenderView *rview, iSector *s, iMeshWrapper* mesh)
{
  if (!shadermanager)
    shadermanager = csQueryRegistry<iShaderManager> (engine->objectRegistry);

  if (s)
  {
    CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());

    // Needed so halos are correctly recognized as "visible".
    csRef<iClipper2D> oldClipper = rview->GetGraphics3D()->GetClipper();
    int oldClipType = rview->GetGraphics3D()->GetClipType();    

    s->IncRecLevel ();
    s->PrepareDraw (rview);
    csSector* cs = (csSector*)s;
    cs->SetSingleMesh (mesh);

    csShaderVariableStack stack;
    stack.Setup (shadermanager->GetShaderVariableStack ().GetSize());
    size_t i;
    for (i = 0; i < steps.GetSize (); i++)
    {
      stack.Clear ();
      steps[i]->Perform (rview, s, stack);
    }
    s->DecRecLevel ();
    cs->SetSingleMesh (0);

    rview->GetGraphics3D()->SetClipper (oldClipper, oldClipType);

    // @@@ See above note about halos.
    iLightList* lights = s->GetLights();
    for (i = lights->GetCount (); i-- > 0;)
      // Tell the engine to try to add this light into the halo queue
      engine->AddHalo (rview->GetCamera(), 
        ((csLight*)lights->Get ((int)i))->GetPrivateObject ());    
  }
}

size_t csRenderLoop::AddStep (iRenderStep* step)
{
  return steps.Push (step);
}

bool csRenderLoop::DeleteStep (iRenderStep* step)
{
  return steps.Delete(step);
}

iRenderStep* csRenderLoop::GetStep (size_t n) const
{
  return steps.Get(n);
}

size_t csRenderLoop::Find (iRenderStep* step) const
{
  return steps.Find(step);
}

size_t csRenderLoop::GetStepCount () const
{
  return steps.GetSize ();
}

//---------------------------------------------------------------------------


csRenderLoopManager::csRenderLoopManager(csEngine* engine)
  : scfImplementationType (this), engine (engine)
{
}

csRenderLoopManager::~csRenderLoopManager()
{
// @@@ ???
#if 0
  csGlobalHashIteratorReversible it (&loops);
  while (it.HasNext())
  {
    iRenderLoop* loop = (iRenderLoop*)it.Next();
    loop->DecRef ();
  }
#endif
}

csPtr<iRenderLoop> csRenderLoopManager::Create ()
{
  csRenderLoop* loop = new csRenderLoop (engine);
  return csPtr<iRenderLoop> (loop);
}
  
bool csRenderLoopManager::Register (const char* name, iRenderLoop* loop,
                                    bool checkDupes)
{
  const char* myName = strings.Request (strings.Request (name));
  if (loops.In (myName))
  {
    return checkDupes;
  }
  loops.Put (myName, loop);
  return true;
}
 
iRenderLoop* csRenderLoopManager::Retrieve (const char* name)
{
  if (name && (strcmp (name, CS_DEFAULT_RENDERLOOP_NAME) == 0))
  {
    // Special case to ensure it's loaded
    engine->GetCurrentDefaultRenderloop();
  }
  return (loops.Get (name, (iRenderLoop*)0));
}

const char* csRenderLoopManager::GetName (iRenderLoop* loop)
{
  return loops.GetKey (loop, 0);
}

bool csRenderLoopManager::Unregister (iRenderLoop* loop)
{
  const char* key;
  if ((key = loops.GetKey (loop, 0)) == 0) return false;
  loops.Delete (key, loop);
  return true;
}
  
struct LoopNamePair
{
  const char* n;
  iRenderLoop* l;
    
  LoopNamePair (const char* n, iRenderLoop* l) : n (n), l (l) {}
};

void csRenderLoopManager::UnregisterAll (bool evenDefault)
{
  if (evenDefault)
  {
    loops.DeleteAll();
    return;
  }
  
  LoopsHash::ConstGlobalIterator it (
    const_cast<const LoopsHash*> (&loops)->GetIterator());
  csArray<LoopNamePair> deleteList;
  deleteList.SetCapacity (loops.GetSize());
  while (it.HasNext())
  {
    const char* name;
    iRenderLoop* loop = it.Next (name);
    if (strcmp (name, CS_DEFAULT_RENDERLOOP_NAME) != 0)
      deleteList.Push (LoopNamePair (name, loop));
  }
  for (size_t i = 0; i < deleteList.GetSize(); i++)
    loops.Delete (deleteList[i].n, deleteList[i].l);
}

csPtr<iRenderLoop> csRenderLoopManager::Load (const char* fileName)
{
  csRef<iPluginManager> plugin_mgr (
  	csQueryRegistry<iPluginManager> (engine->objectRegistry));

  csRef<iLoaderPlugin> rlLoader =
    csLoadPlugin<iLoaderPlugin> (plugin_mgr,
      "crystalspace.renderloop.loop.loader");

  if (rlLoader == 0)
  {
    engine->Error ("Error loading %s: could not retrieve render loop loader",
      CS::Quote::Single (fileName));
    return 0;
  }

  csRef<iFile> file = engine->VFS->Open (fileName, VFS_FILE_READ);
  if (file == 0)
  {
    engine->Error ("Error loading %s: could open file on VFS", CS::Quote::Single (fileName));
    return 0;
  }

  csRef<iDocumentSystem> xml/* ( 
    csQueryRegistry<iDocumentSystem> (engine->object_reg))*/;  
      /* @@@ Eeek. The iDocumentSystem may not be initialized. */
  if (!xml) xml.AttachNew (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();

  const char* error = doc->Parse (file, true);
  if (error != 0)
  {
    engine->Error ("Error parsing %s: %s", CS::Quote::Single (fileName), error);
    return 0;
  }

  csRef<iDocumentNode> rlNode = doc->GetRoot ()->GetNode ("params");
  if (rlNode == 0)
  {
    engine->Error ("Error loading %s: no <params> node", CS::Quote::Single (fileName));
    return 0;
  }

  csRef<iBase> b = rlLoader->Parse (rlNode, 0/*ssource*/, 0, 0);
  if (!b)
  {
    // Error already reported.
    return 0;
  }
  csRef<iRenderLoop> rl = scfQueryInterface<iRenderLoop> (b);
  if (rl == 0)
  {
    engine->ReportBug (
      "Error loading %s: returned object doesn't implement iRenderLoop", 
      CS::Quote::Single (fileName));
  }
  return (csPtr<iRenderLoop> (rl));
}

