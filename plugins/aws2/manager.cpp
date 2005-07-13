/*
    Copyright (C) 2005 by Christopher Nelson

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
#include "manager.h"
#include "frame.h"

#include "csutil/csevent.h"
#include "iengine/engine.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivideo/txtmgr.h"

SCF_IMPLEMENT_FACTORY(awsManager)

SCF_IMPLEMENT_IBASE(awsManager)
  SCF_IMPLEMENTS_INTERFACE(iAws)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(awsManager::eiComponent)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

awsManager::awsManager(iBase *the_base)
{
  SCF_CONSTRUCT_IBASE (the_base);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = 0;
}

awsManager::~awsManager()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);

    scfiEventHandler->DecRef ();
  }

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool 
awsManager::Initialize (iObjectRegistry *_object_reg)
{
  object_reg = _object_reg;
  
  return true;
}

void 
awsManager::SetDrawTarget(iGraphics2D *_g2d, iGraphics3D *_g3d)
{
  g2d = _g2d;
  g3d = _g3d;

  default_font = g2d->GetFontServer()->LoadFont (CSFONT_LARGE);
}

/*********************************************************************
 ***************** Event Handling ************************************
 ********************************************************************/

bool awsManager::HandleEvent (iEvent &Event)
{  
  // Find out what kind of event it is
  switch (Event.Type)
  {
  case csevMouseMove:
  case csevMouseUp:
  case csevMouseClick:
  case csevMouseDown:
  case csevKeyboard:
  case csevBroadcast:
    break;
  }

  return false;
}

/*********************************************************************
 ***************** Redrawing *****************************************
 ********************************************************************/


void awsManager::Redraw()
{
  csPen pen(g2d, g3d);
  csReversibleTransform rt;

  /*rt.Identity();
  g3d->SetWorldToCamera(rt);  */

  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);

  g2d->Write(default_font, 90, 90, g2d->FindRGB(128,128,128,128), -1, "AWS Redrawing");

  pen.SetColor(1,1,1,1);

  pen.WriteBoxed(default_font, 100,100,500,500, CS_PEN_TA_CENTER, CS_PEN_TA_CENTER, "Test Boxed Text - Centered");
  pen.WriteBoxed(default_font, 100,100,500,500, CS_PEN_TA_RIGHT, CS_PEN_TA_TOP, "Test Boxed Text - Right, Top");
  pen.WriteBoxed(default_font, 100,100,500,500, CS_PEN_TA_LEFT, CS_PEN_TA_BOT, "Test Boxed Text - Left, Bot");
  
  pen.DrawPoint(100,100);
  pen.DrawLine(100,100,500,500);
  pen.DrawRect(100,100,500,500, true);
  pen.DrawRoundedRect(0,0,500,500,0.5,false);

  {
    csSimpleRenderMesh mesh;
    csVector3 verts[4];
    csVector2 texels[4];
    csVector4 colors[4];
    float x=100, y=100;
    float w=400, h=400;
    float fr=0.5, fg=0.75, fb=1, fa=1;

    mesh.meshtype = CS_MESHTYPE_QUADS;
    mesh.vertexCount = 4;
    mesh.vertices = verts;    
    mesh.colors = colors;
        
    verts[0].Set (x, y, 0);
    colors[0].Set (fr, fg, fb, fa);
    
    verts[1].Set (x + w, y, 0);    
    colors[1].Set (fr, fg, fb, fa);

    verts[2].Set (x + w, y + h, 0);    
    colors[2].Set (fr, fg, fb, fa);

    verts[3].Set (x, y + h, 0);
    colors[3].Set (fr, fg, fb, fa);

    g3d->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
  }
  

}

/*********************************************************************
 ***************** Definition Files **********************************
 ********************************************************************/

bool awsManager::Load(const scfString &_filename)
{
  return prefs.load(object_reg, _filename);
}
