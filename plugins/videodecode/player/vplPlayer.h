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

#ifndef __VPLPLAYER_H__
#define __VPLPLAYER_H__

#include <iutil/comp.h>
#include <ivideodecode/mediaplayer.h>
#include <ivideodecode/mediacontainer.h>
#include <csutil/scf_implementation.h>

#include <iostream>
using namespace std;

struct iObjectRegistry;

#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.player"

/**
  * This is the implementation for our API and
  * also the implementation of the plugin.
  */
class vplPlayer : public scfImplementation2<vplPlayer,iMediaPlayer,iComponent>
{
private:
  iObjectRegistry* object_reg;

  csRef<iMediaContainer> _mediaFile;
  csRef<iTextureHandle> _target;

  bool _playing;
  bool _shouldLoop;

public:
  vplPlayer (iBase* parent);
  virtual ~vplPlayer ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual void InitializePlayer (csRef<iMediaContainer> media) ;
  virtual void SetActiveStream (int index) ;
  virtual void RemoveActiveStream (int index) ;
  virtual void SetTargetTexture (csRef<iTextureHandle> &target) ;
  virtual void Update ();
  virtual void Loop (bool shouldLoop) ;
  virtual void Play () ;
  virtual void Pause() ;
  virtual void Stop () ;
  virtual void Seek (float time) ;
  virtual float GetPosition () const;
  virtual bool IsPlaying () ;
  virtual float GetLength () const;

  void SwapBuffers();

  /**
   * Embedded iEventHandler interface that handles keyboard events
   */
  class FrameEventHandler : 
    public scfImplementation1<FrameEventHandler, 
    iEventHandler>
  {
  private:
    vplPlayer* parent;
  public:
    FrameEventHandler (vplPlayer* parent) :
        scfImplementationType (this), parent (parent) { }
        virtual ~FrameEventHandler () { }
        virtual bool HandleEvent (iEvent& ev)
        {
          parent->SwapBuffers (); 

          return false;
        }
        CS_EVENTHANDLER_PHASE_FRAME("crystalspace.videodecode.frame")
  };
  csRef<FrameEventHandler> frameEventHandler;

  CS_DECLARE_FRAME_EVENT_SHORTCUTS;
};

#endif // __VPLPLAYER_H__
