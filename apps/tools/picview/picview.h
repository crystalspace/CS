/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __PICVIEW_H__
#define __PICVIEW_H__

#include <crystalspace.h>

class PicView : public csApplicationFramework, public csBaseEventHandler
{
 private:

  csRef<iEngine> engine;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVFS> vfs;
  csRef<iImageIO> imgloader;
  csRef<iAws2> aws;

  csRef<iStringArray> files;
  csRef<iTextureHandle> txt;
  csSimplePixmap* pic;
  
  size_t cur_idx;
  bool scale;
  float x,y;

  bool OnKeyboard (iEvent&);
  bool HandleEvent (iEvent &);

  void ProcessFrame ();
  void FinishFrame ();

  void CreateGui ();


  iAws2ScriptObject *picview_events;

 public:

  PicView ();
  ~PicView ();

  /** Allows us to load the next image in progression. */
  void LoadNextImage (bool rewind, int step);
  
  /** Turns scaling on and off. */
  void FlipScale() { scale^=true; }
  
  /** Posts a quit message. */
  void Quit()
  {
	csRef<iEventQueue> q = CS_QUERY_REGISTRY(GetObjectRegistry(), iEventQueue);
  	if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));	  
  }

  void OnExit ();
  bool OnInitialize (int argc, char* argv[]);

  bool Application ();

  CS_EVENTHANDLER_NAMES ("crystalspace.picview")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __PICVIEW_H__
