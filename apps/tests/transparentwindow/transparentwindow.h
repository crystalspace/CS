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

#ifndef __TRANSPARENTWINDOW_H__
#define __TRANSPARENTWINDOW_H__

#include <crystalspace.h>

class TransparentWindow : public csApplicationFramework, public csBaseEventHandler
{
private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  csRef<iRenderManager> rm;
  csRef<iNativeWindow> natwin;

  iSector* room;
  float rotX, rotY;
  csRef<iTextureHandle> logoTex;
  csRef<iFont> font;
  csRef<FramePrinter> printer;
  
  bool transpRequested;
  bool lastTranspState;
  bool transpOutsideChange;

  bool captionGotChanged;
  bool clientFrameGotChanged;
public:
  bool SetupModules ();

  bool OnKeyboard (iEvent&);
  
  void Frame ();
  void DrawLogo ();
  void DrawOutlineText (iFont* font, int x, int y, const char* text);
  
  void CreateRoom ();
  void CreateTeapot ();
    
  TransparentWindow ();
  ~TransparentWindow ();

  void OnExit ();

  bool OnInitialize (int argc, char* argv[]);
  bool Application ();

  CS_EVENTHANDLER_PHASE_LOGIC("application.transparentwindow")
};

#endif // __TRANSPARENTWINDOW_H__
