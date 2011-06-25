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

struct csVPLvideoFormat;

/**
 * Base of media streams
 */
struct iMedia : public virtual iBase
{
  SCF_INTERFACE(iMedia,0,1,0);

  virtual const char* GetType () = 0;

  /**
   * Get size of this stream in frames.
   */
  virtual unsigned long GetFrameCount() = 0;

  /**
   * Return the length in seconds
   */
  virtual float GetLength() = 0;
  
  /**
   * Gets the position of the stream
   */
  virtual double GetPosition () = 0 ;
  
  /**
   * Clears all the decoders of the stream. Done when destroying the object by the container
   */
  virtual void CleanMedia () = 0 ;

  /**
   * Perform frame-specific updates on the stream
   */
  virtual int Update () = 0 ;
};

/**
 * Video stream
 */
struct iVideoMedia : public iMedia
{
  SCF_INTERFACE(iVideoMedia,0,1,0);

  /// Get the format of the sound data.
  virtual const csVPLvideoFormat *GetFormat() = 0;


  /**
   * Set the texture target
   */
  virtual void SetVideoTarget (csRef<iTextureHandle> texture) = 0;
  
};

/**
 * Audio stream
 */
struct iAudioMedia : public iMedia
{
  SCF_INTERFACE(iAudioMedia,0,1,0);

  /**
   * Set the audio stream target
   */
  virtual void SetAudioTarget (csRef<iSndSysStream> stream) = 0;
};


/** @} */

#endif // __CS_MEDIA_H__
