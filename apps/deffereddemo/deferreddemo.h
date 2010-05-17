/*
  Copyright (C) 2008 by Joe Forte

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

#ifndef __DEFERREDDEMO_H__
#define __DEFERREDDEMO_H__

#include "crystalspace.h"

struct iEngine;
struct iObjectRegistry;
struct iGraphics3D;
struct iGraphics2D;

/**
 * The main class that runs the deferred shading demo application.
 */
class DeferredDemo : public csApplicationFramework, csBaseEventHandler
{
public:

  /// Constructor.
  DeferredDemo();

  /// Destructor.
  ~DeferredDemo();

  /// Initialize the deferred shading demo application.
  virtual bool OnInitialize(int argc, char *argv[]);

  /// Main entry point to the application.
  virtual bool Application();

  /// Returns true if the application sould shutdown after the current frame.
  bool ShouldShutdown() const { return shouldShutdown; }
  
  //CS_EVENTHANDLER_PHASE_LOGIC("application.deferreddemo");

protected:

  /// The deferred shading demos run method.
  void RunDemo();

  /// Updates the cameras position and rotation.
  void UpdateCamera();

  /// Loads modules needed by the application.
  bool SetupModules();

  /// Loads the demo scene.
  bool LoadScene();

  /// Setup the demo scene.
  bool SetupScene();

  /// Handles an event not being Handled by csBaseEventHandler.
  virtual bool OnUnhandledEvent(iEvent &event);

  /// Handles a keyboard event.
  virtual bool OnKeyboard(iEvent &event);

  /// Handles a quit event.
  virtual bool OnQuit(iEvent &event);

  /// Handles the Frame event.
  virtual void Frame();

protected:

  csRef<iEventQueue> eventQueue;

  csRef<iEngine> engine;

  csRef<iGraphics2D> graphics2D;
  csRef<iGraphics3D> graphics3D;

  csRef<iRenderManager> rm;

  csRef<iLoader> loader;

  csRef<iKeyboardDriver> kbd;
  csRef<iMouseDriver> md;

  csRef<iVirtualClock> vc;

  csRef<FramePrinter> printer;

protected:

  csRef<iView> view;

  csRef<iTextureHandle> colorBuffer0;
  csRef<iTextureHandle> depthBuffer;

  float viewRotX;
  float viewRotY;

private:

  bool shouldShutdown;

};

#endif // __DEFERREDDEMO_H__
