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
#include "csengine/renderloop.h"

#include "cssys/sysfunc.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "iengine/material.h"
#include "iengine/rendersteps/irsfact.h"
#include "iengine/rendersteps/igeneric.h"
#include "imap/reader.h"

#include "csgfx/rgbpixel.h"
#include "csengine/engine.h"
#include "csutil/xmltiny.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csRenderLoop)
  SCF_IMPLEMENTS_INTERFACE(iRenderLoop)
  SCF_IMPLEMENTS_INTERFACE(iRenderStepContainer)
SCF_IMPLEMENT_IBASE_END

csRenderLoop::csRenderLoop (csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (engine);
  csRenderLoop::engine = engine;
}

csRenderLoop::~csRenderLoop ()
{
  SCF_DESTRUCT_IBASE ();
}

void csRenderLoop::StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview)
{
  rview.SetEngine (engine);
  rview.SetOriginalCamera (c);

/*  iEngineSequenceManager* eseqmgr = GetEngineSequenceManager ();
  if (eseqmgr)
  {
    eseqmgr->SetCamera (c);
  }*/

  // This flag is set in HandleEvent on a cscmdContextResize event
/*  if (resize)
  {
    resize = false;
    Resize ();
  }*/

  rview.GetClipPlane ().Set (0, 0, 1, -1);      //@@@CHECK!!!

  // Calculate frustum for screen dimensions (at z=1).
  float leftx = -c->GetShiftX () * c->GetInvFOV ();
  float rightx = (engine->frame_width - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = -c->GetShiftY () * c->GetInvFOV ();
  float boty = (engine->frame_height - c->GetShiftY ()) * c->GetInvFOV ();
  rview.SetFrustum (leftx, rightx, topy, boty);
}

void csRenderLoop::Draw (iCamera *c, iClipper2D *view)
{
  engine->ControlMeshes ();

  csRenderView rview (c, view, engine->G3D, engine->G2D);
  StartDraw (c, view, rview);

  // First initialize G3D with the right clipper.
  engine->G3D->SetClipper (view, CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  engine->G3D->ResetNearPlane ();
  engine->G3D->SetPerspectiveAspect (c->GetFOV ());
  
  Draw (&rview, c->GetSector());

  // draw all halos on the screen
/*  if (halos.Length () > 0)
  {
    csTicks elapsed = virtual_clock->GetElapsedTicks ();
    for (int halo = halos.Length () - 1; halo >= 0; halo--)
      if (!halos[halo]->Process (elapsed, *this)) halos.Delete (halo);
  }*/

  engine->G3D->SetClipper (0, CS_CLIPPER_NONE);
}

void csRenderLoop::Draw (iRenderView *rview, iSector *s)
{
  //if (s) s->Draw (&rview);
  if (s)
  {
    s->IncRecLevel ();
    s->PrepareDraw (rview);

    int i;
    for (i = 0; i < steps.Length(); i++)
    {
      steps[i]->Perform (rview, s);
    }
    s->DecRecLevel ();
  }
}

int csRenderLoop::AddStep (iRenderStep* step)
{
  return steps.Push (step);
}

int csRenderLoop::GetStepCount ()
{
  return steps.Length();
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csRenderLoopManager)
  SCF_IMPLEMENTS_INTERFACE(iRenderLoopManager)
SCF_IMPLEMENT_IBASE_END

csRenderLoopManager::csRenderLoopManager(csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (0);

  csRenderLoopManager::engine = engine;
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
  SCF_DESTRUCT_IBASE ();
}

csPtr<iRenderLoop> csRenderLoopManager::Create ()
{
  csRenderLoop* loop = new csRenderLoop (engine);
  return csPtr<iRenderLoop> (loop);
}
  
bool csRenderLoopManager::Register (const char* name, iRenderLoop* loop)
{
  const char* myName = strings.Request (strings.Request (name));
  if (loops.Get (myName) != 0) return false;
  //loop->IncRef();
  loops.Put (myName, loop);
  return true;
}
 
iRenderLoop* csRenderLoopManager::Retrieve (const char* name)
{
  return (loops.Get (name));
}

const char* csRenderLoopManager::GetName (iRenderLoop* loop)
{
  return loops.GetKey (loop);
}

bool csRenderLoopManager::Unregister (iRenderLoop* loop)
{
  const char* key;
  if ((key = loops.GetKey (loop)) == 0) return false;
  //loop->DecRef();
  loops.Delete (key, loop);
  return true;
}

csPtr<iRenderLoop> csRenderLoopManager::Load (const char* fileName)
{
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (engine->object_reg, iPluginManager));

  csRef<iLoaderPlugin> rlLoader =
    CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.renderloop.loop.loader",
      iLoaderPlugin);

  if (rlLoader == 0)
  {
    engine->Error ("Error loading '%s': could not retrieve render loop loader",
      fileName);
    return 0;
  }

  csRef<iFile> file = engine->VFS->Open (fileName, VFS_FILE_READ);
  if (file == 0)
  {
    engine->Error ("Error loading '%s': could open file on VFS", fileName);
    return 0;
  }

  csRef<iDocumentSystem> xml (CS_QUERY_REGISTRY (engine->object_reg, 
    iDocumentSystem));
  if (!xml) xml.AttachNew (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();

  const char* error = doc->Parse (file);
  if (error != 0)
  {
    engine->Error ("Error parsing '%s': %s", fileName, error);
    return 0;
  }

  csRef<iDocumentNode> rlNode = doc->GetRoot ()->GetNode ("params");
  if (rlNode == 0)
  {
    engine->Error ("Error loading '%s': no <params> node", fileName);
    return 0;
  }

  csRef<iBase> b = rlLoader->Parse (rlNode, 0, 0);
  if (!b)
  {
    // Error already reported.
    return 0;
  }
  csRef<iRenderLoop> rl = SCF_QUERY_INTERFACE (b, iRenderLoop);
  if (rl == 0)
  {
    engine->ReportBug (
      "Error loading '%s': returned object doesn't implement iRenderLoop", 
      fileName);
  }
  return (csPtr<iRenderLoop> (rl));
}

