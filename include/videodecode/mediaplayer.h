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

struct iMediaContainer;
struct iTextureWrapper;

/**
 * The video player
 */
struct iMediaPlayer : public virtual iBase
{
  SCF_INTERFACE(iMediaPlayer,0,1,0);

  /// Initialize the video player
  virtual void InitializePlayer (iMediaContainer video) = 0;

  /// Sets the active video stream from inside the iMediaContainer
  virtual void SetActiveVideoStream (int index) = 0;

  /// Sets the active audio stream from inside the iMediaContainer
  virtual void SetActiveAudioStream (int index) = 0;

  /// Called continously to update the player
  virtual void Update () = 0;

  /// Returns the current video frame as an iTextureWrapper, 
  //  to be used fo texturing objects or rendered 2D
  virtual iTextureWrapper GetVideoFrame () = 0;

  /// Starts playing the video
  virtual void Play () = 0 ;

  /// Pauses the video
  virtual void Pause() = 0 ;

  /// Stops the video and seeks to the beginning
  virtual void Stop () = 0 ;

  /// Seeks the video
  virtual void Seek (long position) = 0 ;

  /// Returns if the video is playing or not
  virtual bool IsPlaying () = 0 ;
};

/** @} */

#endif // __CS_MEDIAPLAYER_H__
