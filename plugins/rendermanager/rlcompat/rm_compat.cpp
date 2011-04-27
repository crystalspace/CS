/*
    Copyright (C) 2008 by Frank Richter

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

#include "iengine/engine.h"
#include "ivaria/reporter.h"
#include "csplugincommon/rendermanager/render.h"

#include "rm_compat.h"



CS_PLUGIN_NAMESPACE_BEGIN(RM_RLCompat)
{
  SCF_IMPLEMENT_FACTORY(RMCompat)

  bool RMCompat::Initialize (iObjectRegistry* objReg)
  {
    dbgDebugClearScreen = debugPersist.RegisterDebugFlag ("debugclear");
    return true;
  }
  
  bool RMCompat::RenderView (iView* view)
  {
    iEngine* engine = view->GetEngine();
    view->UpdateClipper();
    
    if (debugPersist.IsDebugFlagEnabled (dbgDebugClearScreen))
    {
      iGraphics2D* G2D = view->GetContext()->GetDriver2D();
      view->GetContext()->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARZBUFFER);
      int bgcolor_clear = G2D->FindRGB (0, 255, 255);
      G2D->Clear (bgcolor_clear);
    }
      
    CS::RenderManager::BeginFinishDrawScope drawScope (
      view->GetContext(), CSDRAW_CLEARSCREEN | CSDRAW_3DGRAPHICS);
    engine->Draw (view->GetCamera(), view->GetClipper());
    return true;
  }

  bool RMCompat::PrecacheView (iView* view)
  {
    return RenderView (view);
  }
  
  bool RMCompat::DebugCommand (const char* _cmd)
  {
    csString cmd (_cmd);
    csString args;
    size_t space = cmd.FindFirst (' ');
    if (space != (size_t)-1)
    {
      cmd.SubString (args, space+1);
      cmd.Truncate (space);
    }

    if (strcmp (cmd, "toggle_debug_flag") == 0)
    {
      uint flag = debugPersist.QueryDebugFlag (args);
      if (flag != (uint)-1)
      {
	debugPersist.EnableDebugFlag (flag,
	  !debugPersist.IsDebugFlagEnabled (flag));
      }
      return true;
    }
    return false;
  }

}
CS_PLUGIN_NAMESPACE_END(RM_RLCompat)
