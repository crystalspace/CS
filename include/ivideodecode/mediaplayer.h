/*
Copyright (C) 2011 by Alin Baciu

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

#ifndef __CS_MEDIAPLAYER_H__
#define __CS_MEDIAPLAYER_H__

/**\file
  * Video Player: media container 
  */

#include "csutil/scf.h"
#include "csutil/ref.h"
#include "crystalspace.h"

struct iMediaContainer;
struct iTextureHandle;

/**
  * The video player
  */
struct iMediaPlayer : public virtual iBase
{
  SCF_INTERFACE (iMediaPlayer,0,1,0);

  /// Initialize the video player
  virtual void InitializePlayer (csRef<iMediaContainer> media) = 0;

  /// Activates a stream from inside the iMediaContainer
  virtual void SetActiveStream (int index) = 0;

  /// Deactivates a stream from inside the iMediaContainer
  virtual void RemoveActiveAudioStream (int index) = 0;

  /// Set the target texture
  virtual void SetTargetTexture (csRef<iTextureHandle> &target) = 0;

  /// Called continuously to update the player
  virtual void Update () = 0;

  /// Enable/disable looping
  virtual void Loop (bool shouldLoop) = 0 ;

  /// Starts playing the media
  virtual void Play () = 0 ;

  /// Pauses the media
  virtual void Pause() = 0 ;

  /// Stops the media and seeks to the beginning
  virtual void Stop () = 0 ;

  /// Seeks the media
  virtual void Seek (float time) = 0 ;

  /// Get the position of the media
  virtual float GetPosition () = 0 ;

  /// Get the length of the media
  virtual float GetLength () = 0 ;

  /// Returns if the media is playing or not
  virtual bool IsPlaying () = 0 ;
};

/** @} */

#endif // __CS_MEDIAPLAYER_H__
