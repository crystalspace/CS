/*
    Copyright (C) 2001 by Norman Krämer
  
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

#ifndef _IVIDEO_H_
#define _IVIDEO_H_

/**
 * This is an interface for playing video.
 */

#include "csutil/scf.h"
#include "isystem.h"
#include "iplugin.h"

struct iMaterialHandle;
struct iFile;

enum csStreamFormatCap
{
  /**
   * The decoder is able to set the current position to a particular frame.
   */
  CS_POS_BY_FRAME = 1,
  /**
   * The decoder is able to set the current position based on a time index.
   */
  CS_POS_BY_TIME = 2,
  /**
   * The decoder is able to decode a set of consecutive frames at once.
   */
  CS_DECODE_SPAN = 4,
  /**
   * The decoder is able to dynamically change frame output size.
   */
  CS_DYNAMIC_FRAMESIZE = 8
};

struct csStreamDescription
{
  /**
   * Type of stream ala video, audio, text or whatever.
   */
  UShort type;
  /**
   * Codec id.
   */
  char codec[64];
  
};

struct csVideoStreamDescription : public csStreamDescription
{
  /**
   * Colordepth in bits or -1 if not available.
   */
  SByte colordepth;
  /**
   * Framecount or -1 if not available.
   */
  long framecount;
  /**
   * Resolution or -1 if not available.
   */
  int width, height;
  /**
   * Framerate in frames per second or -1 if not available.
   */
  float framerate;
  /**
   * duration in milliseconds or -1 if not available.
   */
  long duration;
};

struct csAudioStreamDescription : public csStreamDescription
{
  UShort formattag;
  UShort channels;
  ULong samplespersecond;
  UShort bitspersample;
  /**
   * duration in milliseconds or -1 if not available.
   */
  long duration;
};

SCF_VERSION (iStreamIterator, 0, 0, 1);
struct iStreamIterator : public iBase
{
  virtual bool HasNext () = 0;
  virtual iStream *GetNext () = 0;
};

SCF_VERSION (iStreamFormat, 0, 0, 1);
struct iStreamFormat : public iPlugIn
{
  /**
   * Retrieve the decoder capabilities.
   */
  virtual void GetCaps (csStreamFormatCap &caps) = 0;
  /**
   * Get an iterator to enumerate the streams.
   */
  virtual iStreamIterator& GetStreamIterator () = 0;
  /**
   * Choose a video and audio stream to be played when calling NextFrame.
   */
  virtual void Select (iAudioStream *pAudio, iVideoStream *pVideo);
  /** 
   * Call this in your main loop between BeginDraw and EndDraw. This will decode the next frame
   * from the video and draw it to the rectangle set in SetRect ().
   * This is an convenience function only. You achieve the same results by calling this explicitly
   * for the streams to play.
   */
  virtual void NextFrame () = 0;
  /**
   * Load the videodata from the following source.
   */
  virtual bool Load (iFile *pVideoData) = 0;
};

SCF_VERSION (iStream, 0, 0, 1);
struct iStream : public iBase
{
  /**
   * Retrieve descriptive stream information.
   */
  virtual void GetStreamDescription (csStreamDescription &desc) = 0;
  /**
   * Next frame to be examined at frameindex. Note that <frame> does not necessarily mean
   * a video frame. It is the datachunk a stream is separated into by the underlying protocol/format
   * like RIFF for instance.
   */
  virtual bool GotoFrame (ULong frameindex) = 0;
  /**
   * Next frame to be examined is the frame at timeindex.
   */
  virtual bool GotoTime (ULong timeindex) = 0;
  /**
   * When NextFrame is called either the next frame is read based on frameindex
   * or the next frame at timeindex (bTimeSynced = true).
   */
  virtual bool SetPlayMethod (bool bTimeSynced) = 0;
  /** 
   * Call this in your main loop between BeginDraw and EndDraw. 
   * This will examine the next frame (either time or index based).
   */
  virtual void NextFrame () = 0;
};

SCF_VERSION (iVideoStream, 0, 0, 1);
struct iVideoStream : public iStream
{
  /**
   * Retrieve descriptive stream information.
   */
  virtual void GetStreamDescription (csVideoStreamDescription &desc) = 0;
  /**
   * Set the output rectangle.
   */
  virtual bool SetRect (int x, int y, int w, int h) = 0;
  /**
   * Set the blendingmode that is used to combine this frame with the data in the framebuffer
   * Use the usual CS_FX_* values. The default is CS_FX_COPY.
   */
  virtual bool SetFXMode (UInt mode) = 0;
  /**
   * Call this if you want to use the frames as textures on your own polygons.
   */
  virtual iMaterialHandle* NextFrameGetMaterial () = 0;
  
};

SCF_VERSION (iAudioStream, 0, 0, 1);
struct iAudioStream : public iStream
{
  /**
   * Retrieve descriptive stream information.
   */
  virtual void GetStreamDescription (csAudioStreamDescription &desc) = 0;
};

/**
 * This is the core encoder or decoder. It will be loaded by the videodecoder.
 * For instance the AVI videodecoder is able to load various codecs based on the format the video
 * was saved in.
 */

/**
 * CHANNEL means all data of one kind in a row, like all red first then all green etc.
 * INTERLEAVED means all components of one point in a row, for instance 1st byte is red component 
 * of pixel 0, 2nd byte is green component of pixel 0 etc.
 */

/// formats for videodata
#define CS_CODECFORMAT_RGB_CHANNEL
#define CS_CODECFORMAT_RGBA_CHANNEL
#define CS_CODECFORMAT_YUV_CHANNEL
#define CS_CODECFORMAT_RGB_INTERLEAVED
#define CS_CODECFORMAT_RGBA_INTERLEAVED
#define CS_CODECFORMAT_YUV_INTERLEAVED

/// formats for audiodata
#define CS_CODECFORMAT_PCM

struct csCodecDecsription
{
  /**
   * CODEC id
   */
  char codec[64];
  /**
   * You can encode data using this codec.
   */
  bool bEncode;
  /**
   * You can decode data using this codec.
   */
  bool bDecode;
  /**
   * output data format
   */
  UShort decodeoutput;
  /**
   * input data format
   */
  UShort encodeinput;
};

SCF_VERSION (iCodec, 0, 0, 1);
struct iCodec : public iBase
{
  /**
   * Send either video or audio stream description as input. The codec will cast it.
   */
  virtual void Initialize (csStreamDescription *desc) = 0;
  virtual void GetCodecDescription (csCodecDescription &desc) = 0;
  virtual bool Decode (char *indata, long inlength, void *&outdata) = 0;
  virtual bool Encode (void *indata, char *outdata, long &outlength) = 0;
};

#endif
