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

#ifndef __CS_THOGGVIDEOMEDIA_H__
#define __CS_THOGGVIDEOMEDIA_H__

/**\file
 * Video Player: media stream 
 */

#include <iutil/comp.h>
#include <videodecode/medialoader.h>
#include <videodecode/mediacontainer.h>
#include <videodecode/media.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>

// theora headers
#include "theora/theoradec.h"
#include "theora/theora.h"

#include <iostream>
using namespace std;

struct iTextureWrapper; 
struct csVPLvideoFormat;

/**
 * Video stream
 */
class TheoraVideoMedia : public scfImplementation2< TheoraVideoMedia, iVideoMedia, scfFakeInterface<iMedia> >
{
private:
  iObjectRegistry*			object_reg;
	csRef<iTextureHandle> _texture;
	float									length;
	unsigned long					frameCount;

	ogg_int64_t						videobuf_granulepos;
	bool									skipToKeyframe;

// these will be private and have getters and setters, but for now, it's faster like this
public:
  ogg_stream_state	to;
  th_info						ti;
  th_comment				tc;
  th_dec_ctx				*td;
  ogg_packet				op;
  th_setup_info			*ts;
  int								theora_p;
  FILE							*out,
										*infile;
	bool							decodersStarted;
	bool							videobuf_ready;
	double						videobuf_time;

public:
  TheoraVideoMedia (iBase* parent);
  virtual ~TheoraVideoMedia ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  
  virtual const char* GetType ();

  /// Get the format of the sound data.
  virtual const csVPLvideoFormat *GetFormat();

  /**
   * Get size of this sound in frames.
   */
  virtual unsigned long GetFrameCount();

  /**
   * Return the length in seconds
   */
  virtual float GetLength();

  /**
   * Set the texture target
   */
  virtual void SetVideoTarget (csRef<iTextureHandle> texture);
  
  /**
   * Gets the position of the video stream
   */
  virtual double GetPosition ();
  
  /**
   * Clears all the decoders of the stream. Done when destroying the object by the container
   */
  virtual void CleanMedia () ;

  /**
   * Perform frame-specific updates on the stream
   */
  virtual int Update () ;


	void SetFrameCount (unsigned long count)
	{
		frameCount=count;
	}

	void SetLength (float length)
	{
		this->length=length;
	}

	long SeekPage(long targetFrame,bool return_keyframe, ogg_sync_state *oy,unsigned  long fileSize);
};


#endif // __CS_THOGGVIDEOMEDIA_H__
