/*
Copyright (C) 2010 by Alin Baciu

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

#ifndef __VIDEOPLAY_H__
#define __VIDEOPLAY_H__

#include "crystalspace.h"

#include "ivaria/icegui.h"

#include <iostream>

#include <ivideodecode/medialoader.h>
#include <ivideodecode/media.h>
#include <ivideodecode/mediacontainer.h>
#include <ivideodecode/mediaplayer.h>

struct iEngine;
struct iObjectRegistry;
struct iGraphics3D;
struct iGraphics2D;

/**
  * The main class that runs the video player demo application.
  */
class VideoTest : public CS::Utility::DemoApplication
{
private:
  bool CreateScene ();

  bool inWater;
  csRef<iTextureHandle> logoTex;
  csRef<iMediaPlayer> mediaPlayer;

  csRef<iVFS> vfs;
  csRef<iCEGUI> cegui;

  // The sound renderer.
  csRef<iSndSysRenderer> sndrenderer;

  // The sound source.
  csRef<iSndSysSource> sndsource;

  // The audio stream.
  csRef<iSndSysStream> audioStream;

  // We need this to make the seeking slider work properly
  bool updateSeeker;

private:
  void InitializeCEGUI ();

  // Handle exit button clicked event
  bool OnExitButtonClicked (const CEGUI::EventArgs& e);
  // Handle play button clicked event
  bool OnPlayButtonClicked (const CEGUI::EventArgs& e);
  // Handle pause button clicked event
  bool OnPauseButtonClicked (const CEGUI::EventArgs& e);
  // Handle stop button clicked event
  bool OnStopButtonClicked (const CEGUI::EventArgs& e);
  // Handle the loop radio button
  bool OnLoopToggle (const CEGUI::EventArgs& e);
  // Handle seeking
  bool OnSeekingStart (const CEGUI::EventArgs& e);
  bool OnSeekingEnd (const CEGUI::EventArgs& e);

public:
  VideoTest ();

  //-- CS::Utility::DemoApplication
  void OnExit ();
  void PrintHelp ();
  void Frame ();

  const char* GetApplicationConfigFile ()
  { return "/config/csisland.cfg"; }

  //-- csApplicationFramework
  bool Application ();
};

#endif // __VIDEOPLAY_H__
