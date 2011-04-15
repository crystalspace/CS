/*
  Copyright (C) 2010-11 Christian Van Brussel, Communications and Remote
  Sensing Laboratory of the School of Engineering at the 
  Universite catholique de Louvain, Belgium
  http://www.tele.ucl.ac.be

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
#ifndef __CS_HUDMANAGER_TEXT_H__
#define __CS_HUDMANAGER_TEXT_H__

#include "cssysdef.h"
#include "cstool/cspixmap.h"
#include "csutil/scf_implementation.h"
#include "csutil/stringarray.h"
#include "csutil/eventhandlers.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivaria/hudmanager.h"

CS_PLUGIN_NAMESPACE_BEGIN(TextHUDManager)
{

/**
 * Text-based HUD manager
 */

class TextHUDManager
  : public scfImplementation3<TextHUDManager,
  CS::Utility::iHUDManager,
  iEventHandler,
  iComponent>
{
public:
  TextHUDManager (iBase* parent);
  ~TextHUDManager ();

  //-- iComponent
  bool Initialize (iObjectRegistry* registry);

  //-- iEventHandler
  virtual bool HandleEvent (iEvent& event);

  //-- CS::Utility::iHUDManager
  virtual void SwitchKeysPage ();

  virtual void WriteShadow (int x, int y, int color, const char *str,...) const;
  virtual void Write (int x, int y, int fg, int color, const char *str,...) const;

  virtual void SetEnabled (bool enabled);
  virtual bool GetEnabled () const;

  virtual iStringArray* GetKeyDescriptions ();
  virtual iStringArray* GetStateDescriptions ();

private:
  // Reference to the 3D graphics
  csRef<iGraphics3D> g3d;
  // Reference to the 2D graphics
  csRef<iGraphics2D> g2d;
  // Reference to the virtual clock
  csRef<iVirtualClock> vc;
  // Reference to the event queue
  csRef<iEventQueue> eventQueue;

  csRef<iStringArray> keyDescriptions;
  csRef<iStringArray> stateDescriptions;

  // Reference to the font used to display information
  csRef<iFont> font;
  // Crystal Space logo
  csPixmap* cslogo;

  // Whether or not the HUD is displayed
  bool enabled;

  // Computing of frames per second
  uint frameCount;
  int frameTime;
  float currentFPS;

  // Current page of available keys to be displayed
  uint currentKeyPage;
  uint maxKeys;

  // Declare this event handler as listening to the '2D' frame phase
  CS_EVENTHANDLER_PHASE_2D ("crystalspace.utilities.texthud");
};

}
CS_PLUGIN_NAMESPACE_END(TextHUDManager)

#endif // __CS_HUDMANAGER_TEXT_H__
