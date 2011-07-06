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
#include <videodecode/mediaplayer.h>
#include <videodecode/mediacontainer.h>
#include <videodecode/media.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>

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

  /// Initialize the video player
  virtual void InitializePlayer (csRef<iMediaContainer> media) ;

  /// Activates a stream from inside the iMediaContainer
  virtual void SetActiveStream (int index) ;

  /// Deactivates a stream from inside the iMediaContainer
  virtual void RemoveActiveAudioStream (int index) ;

  /// Set the target texture
  virtual void SetTargetTexture (csRef<iTextureHandle> &target) ;

  /// Called continuously to update the player
  virtual void Update ();

  /// Enable/disable looping
  virtual void Loop (bool shouldLoop) ;

  /// Starts playing the media
  virtual void Play () ;

  /// Pauses the media
  virtual void Pause() ;

  /// Stops the media and seeks to the beginning
  virtual void Stop () ;

  /// Seeks the media
  virtual void Seek (float time) ;

  /// Get the position of the media
  virtual csTicks GetPosition () ;

  /// Returns if the media is playing or not
  virtual bool IsPlaying () ;
};

#endif // __VPLPLAYER_H__