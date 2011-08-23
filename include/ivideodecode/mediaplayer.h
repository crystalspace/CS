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
  * Video Player: player 
  */
/**
 * \addtogroup videoplay
 * @{ */


#include "csutil/scf.h"
#include "csutil/ref.h"
#include "crystalspace.h" 	

#include "iutil/threadmanager.h"

struct iMediaContainer;
struct iTextureHandle;

/**
  * The video player
  */
struct iMediaPlayer : public virtual iBase
{
  SCF_INTERFACE (iMediaPlayer,0,1,0);

  /**
    * Initialize the video player
    * \param[in] media The media container to be used by the player
    * \param[in] cacheSize The number of frames to cache.If caching is not needed, use 1
    */
  virtual void InitializePlayer (csRef<iMediaContainer> media, size_t cacheSize = 1) = 0;
  /**
    * Activate a stream from the media container
    * \param[in] index Object index
    */
  virtual void SetActiveStream (int index) = 0;

  /**
   * Deactivate a stream from the media container
    * \param[in] index Object index
    */
  virtual void RemoveActiveStream (int index) = 0;

  /**
    * Get a reference to the internal texture buffer
    * \param[out] target Target texture
    */
  virtual void GetTargetTexture (csRef<iTextureHandle> &target) = 0;

  /**
    *  Get a reference to the internal audio stream
    * \param[out] target Target audio stream
    */
  virtual void GetTargetAudio (csRef<iSndSysStream> &target) = 0;

  /**
    * Called continuously to update the player. The user shouldn't call this method directly
    * To start and stop the player, use StartPlayer() and StopPlayer()
    */
  THREADED_INTERFACE ( Update );

  /**
   * Start the update thread for the media player In order to close the player
    * properly, this must be called when starting the player.
    */
  virtual void StartPlayer () = 0;

  /**
    * Stop the update thread for the media player. In order to close the player
    * properly, this must be called when shutting down the player.
    */
  virtual void StopPlayer () = 0;

  /**
    * Enable/disable looping
    */
  virtual void Loop (bool shouldLoop) = 0 ;

  /**
    * Start playing the media
    */
  virtual void Play () = 0 ;

  /**
    * Pause the media
    */
  virtual void Pause () = 0 ;

  /**
    * Stop the media and seek to the beginning
    */
  virtual void Stop () = 0 ;

  /**
    * Seek the media
    */
  virtual void Seek (float time) = 0 ;

  /**
    * Get the position of the media in seconds
    */
  virtual float GetPosition () const = 0 ;

  /**
    * Get the length of the media in seconds
    */
  virtual float GetLength () const = 0 ;

  /**
    * Check if the media is playing or not
    */
  virtual bool IsPlaying () = 0 ;

  /**
    * Get the aspect ratio of the active video stream
    */  
  virtual float GetAspectRatio () = 0;

  /**
    * Select a language from the available ones
    */
  virtual void SelectLanguage (const char* identifier) = 0;
};

/** @} */

#endif // __CS_MEDIAPLAYER_H__
