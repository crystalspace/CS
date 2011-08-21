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

#ifndef __CS_MEDIA_H__
#define __CS_MEDIA_H__

/**\file
  * Video Player: media stream 
  */

#include "csutil/scf.h"
#include "csutil/ref.h"
#include "crystalspace.h"
#include "mediastructs.h"

/**
  * Base of media streams
  */
struct iMedia : public virtual iBase
{
  SCF_INTERFACE (iMedia,0,1,0);

  virtual const char* GetType () const = 0;

  /**
    * Get size of this stream in frames.
    */
  virtual unsigned long GetFrameCount() const = 0;

  /**
    * Return the length in seconds
    */
  virtual float GetLength () const = 0;

  /**
    * Gets the position of the stream
    */
  virtual double GetPosition () const = 0 ;

  /**
    * Clears all the decoders of the stream. Done when destroying the object by the container
    */
  virtual void CleanMedia () = 0 ;

  /**
    * Perform frame-specific updates on the stream.
    */
  virtual bool Update () = 0 ;

  /**
    * Gets data from the prefetch queue and writes it to the active buffer
    */
  virtual void WriteData () = 0 ;
  
  /**
    * Swaps the active buffer for the one that was written to last
    */
  virtual void SwapBuffers () = 0;
  
  /**
    * Set the how many frames will be cached
    */
  virtual void SetCacheSize (size_t size) = 0;

  /**
    * Returns true if there is data ready to be used
    */
  virtual bool HasDataReady () = 0;

  /**
    * Returns true if the cache is full
    */
  virtual bool IsCacheFull () = 0;

  /**
    * Triggers a frame drop
    **/
  virtual void DropFrame () = 0;
};

/**
  * Video stream
  */
struct iVideoMedia : public iMedia
{
  SCF_INTERFACE (iVideoMedia,0,1,0);

  /// Get the format of the sound data.
  virtual const csVPLvideoFormat* GetFormat () const = 0;

  // Returns the aspect ratio to use with the image
  virtual float GetAspectRatio () = 0;


  /**
    * Makes "texture" point to the internal iTextureHandle of the stream
    */
  virtual void GetVideoTarget (csRef<iTextureHandle> &texture) = 0;

  /**
    * Returns the target FPS the video stream should run at
    */
  virtual double GetTargetFPS () = 0;

};

/**
  * Audio stream
  */
struct iAudioMedia : public iMedia
{
  SCF_INTERFACE (iAudioMedia,0,1,0);

  /**
    * Get the audio stream target
    */
  virtual void GetAudioTarget (csRef<iSndSysStream> &stream) = 0;
};


/** @} */

#endif // __CS_MEDIA_H__
