/*
    PicoGUI Client/Server Test Application
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

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
#include "csutil/ref.h"
#include "igraphic/imageio.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/event.h"
#include "iutil/evdefs.h"
#include "cstool/initapp.h"
#include "iutil/objreg.h"
#include "csutil/sysfunc.h"
//#include "ivaria/gclient.h"
#include "ivaria/gserver.h"

CS_IMPLEMENT_APPLICATION

iObjectRegistry *objreg;
csRef<iGraphics3D> G3D;

bool HandleEvent (iEvent &Event)
{
  if (Event.Type == csevBroadcast)
  {
    if (Event.Command.Code == cscmdPreProcess)
    {
      G3D->BeginDraw (CSDRAW_2DGRAPHICS);
    }
    else if (Event.Command.Code == cscmdPostProcess)
    {
      G3D->FinishDraw ();
      G3D->Print (0);
    }
    else return false;
  }
  else if ((Event.Type == csevKeyboard) &&
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == 'q'))
  {
    csInitializer::CloseApplication (objreg);
  }
  else return false;

  csSleep(5);

  return true;
}

int main (int argc, char *argv[])
{
  objreg = csInitializer::CreateEnvironment (argc, argv);
  if (! objreg)
  {
    fprintf (stderr, "Failed to create environment.\n");
    return 1;
  }

  bool ok = csInitializer::RequestPlugins (objreg,
    //CS_REQUEST_SOFTWARE3D,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,					  
//  CS_REQUEST_PLUGIN ("crystalspace.gui.pico.client", iGUIClientHelper),
    CS_REQUEST_PLUGIN ("crystalspace.gui.pico.server", iGUIServer),
    CS_REQUEST_END);
  if (! ok)
  {
    fprintf (stderr, "Failed to load plugins.\n");
    return 2;
  }

  G3D = CS_QUERY_REGISTRY (objreg, iGraphics3D);
  if (! G3D)
  {
    fprintf (stderr, "Failed to find 3d graphics driver.\n");
    return 3;
  }

  ok = G3D->Open ();
  if (! ok)
  {
    fprintf (stderr, "Failed to open 3d graphics display.\n");
    return 3;
  }

  ok = csInitializer::SetupEventHandler (objreg, HandleEvent,
    CSMASK_Nothing | CSMASK_Keyboard);
  if (! ok)
  {
    fprintf (stderr, "Failed to setup event handler.\n");
    return 5;
  }

  ok = csInitializer::OpenApplication (objreg);
  if (! ok)
  {
    fprintf (stderr, "Failed to open application.\n");
    return 6;
  }

  G3D->GetDriver2D ()->SetMouseCursor (csmcNone);
  csDefaultRunLoop (objreg);

  // G3D->Close ();
  G3D = 0;
  csInitializer::DestroyApplication (objreg);
  return 0;
}
