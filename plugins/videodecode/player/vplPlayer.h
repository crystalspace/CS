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
#include <ivideodecode/media.h>
#include <csutil/scf_implementation.h>

struct iObjectRegistry;

#ifdef WIN32
  CS_IMPLEMENT_PLUGIN
#endif

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

  virtual void RemoveActiveAudioStream (int index) ;

  virtual void SetTargetTexture (csRef<iTextureHandle> &target) ;

  virtual void Update ();

  virtual void Loop (bool shouldLoop) ;

  virtual void Play () ;

  virtual void Pause() ;

  virtual void Stop () ;

  virtual void Seek (float time) ;

  virtual float GetPosition () ;

  virtual bool IsPlaying () ;

  virtual float GetLength () ;
};

#endif // __VPLPLAYER_H__