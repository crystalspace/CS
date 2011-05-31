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

struct iTextureWrapper;
struct csVPLvideoFormat;

/**
 * Base of media streams
 */
struct iMedia : public virtual iBase
{
  SCF_INTERFACE(iMedia,0,1,0);

  virtual const char* GetType () = 0;
}

/**
 * Video stream
 */
struct iVideoMedia : public iMedia
{
  SCF_INTERFACE(iVideoMedia,0,1,0);

  /// Get the format of the sound data.
  virtual const csVPLvideoFormat *GetFormat();

  /**
   * Get size of this sound in frames.
   */
  virtual size_t GetFrameCount();

  /**
   * Return the size of the data stored in bytes
   */
  virtual size_t GetDataSize();

  /**
   * Gets the video data for the next frame
   */
  virtual iTextureWrapper GetNextFrame () = 0;
  
  /**
   * Seeks the video stream
   */
  virtual void Seek (long position) = 0 ;
  
  /**
   * Gets the position of the video stream
   */
  virtual long GetPosition () = 0 ;
}

/**
 * Audio stream
 */
struct iAudioMedia : public iMedia
{
  SCF_INTERFACE(iAudioMedia,0,1,0);

  /**
   * Get size of this sound in frames.
   */
  virtual size_t GetFrameCount();

  /**
   * Return the size of the data stored in bytes
   */
  virtual size_t GetDataSize();

  /**
   * Gets the audio data for the next frame
   */
  virtual unsigned char* GetNextFrame () = 0;

  /**
   * Seeks the audio stream
   */
  virtual void Seek (long position) = 0 ;
  
  /**
   * Gets the position of the audio stream
   */
  virtual long GetPosition () = 0 ;
}


/** @} */

#endif // __CS_MEDIA_H__
