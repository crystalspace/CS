/*
    Copyright (C) 2001 by Norman Kraemer

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

#ifndef __CS_IVIDEO_CODEC_H__
#define __CS_IVIDEO_CODEC_H__

/**\file
 * Interface for playing video.
 */

#include "csutil/scf.h"

struct iFile;
struct iStream;
struct iVideoStream;
struct iAudioStream;
struct iCodec;
struct iTextureHandle;

/**
 * Stream format capability enumeration.
 */
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

#define CS_STREAMTYPE_AUDIO 1
#define CS_STREAMTYPE_VIDEO 2
#define CS_STREAMTYPE_MIDI 3
#define CS_STREAMTYPE_TEXT 4

/**
 * Stream description structure.
 */
struct csStreamDescription
{
  /**
   * Type of stream ala video, audio, text or whatever.
   */
  uint16 type;
  /**
   * Codec id.
   */
  char codec[64];
  /**
   * stream name
   */
  const char *name;
};

/**
 * Video stream description structure.
 */
struct csVideoStreamDescription : public csStreamDescription
{
  /**
   * Colordepth in bits or -1 if not available.
   */
  int8 colordepth;
  /**
   * Framecount or -1 if not available.
   */
  int32 framecount;
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
  int32 duration;
};

/**
 * Audio stream description structure.
 */
struct csAudioStreamDescription : public csStreamDescription
{
  uint16 formattag;
  uint16 channels;
  uint32 samplespersecond;
  uint16 bitspersample;
  /**
   * duration in milliseconds or -1 if not available.
   */
  uint32 duration;
};

SCF_VERSION (iStreamIterator, 0, 0, 1);

/// @@@ Document me.
struct iStreamIterator : public iBase
{
  virtual bool HasNext () = 0;
  virtual iStream *Next () = 0;
};

SCF_VERSION (iStreamFormat, 0, 0, 1);

/**
 * Stream format.
 */
struct iStreamFormat : public iBase
{
  /**
   * Retrieve the decoder capabilities.
   */
  virtual void GetCaps (csStreamFormatCap &caps) = 0;
  /**
   * Get an iterator to enumerate the streams.
   */
  virtual iStreamIterator* GetStreamIterator () = 0;
  /**
   * Choose a video and audio stream to be played when calling NextFrame.
   */
  virtual void Select (iAudioStream *pAudio, iVideoStream *pVideo) = 0;
  /**
   * Call this in your main loop between BeginDraw and EndDraw.  This will
   * decode the next frame from the video and draw it to the rectangle set in
   * SetRect ().  This is an convenience function only.  You achieve the same
   * results by calling this explicitly for the streams to play.
   */
  virtual void NextFrame () = 0;
  /**
   * Load the videodata from the following source.
   */
  virtual bool Load (iFile *pVideoData) = 0;
  /**
   * Unload this video.  All streams become invalid.  This is automatically
   * called by Load ().  Prior to the final DecRef of this plugin you have to
   * call this yourself.
   */
  virtual void Unload () = 0;
};

SCF_VERSION (iStream, 0, 0, 1);

/**
 * A stream.
 */
struct iStream : public iBase
{
  /**
   * Retrieve descriptive stream information.
   */
  virtual void GetStreamDescription (csStreamDescription &desc) = 0;
  /**
   * Next frame to be examined at frameindex.  Note that `frame' does not
   * necessarily mean a video frame.  It is the datachunk a stream is separated
   * into by the underlying protocol/format like RIFF for instance.
   */
  virtual bool GotoFrame (uint32 frameindex) = 0;
  /**
   * Next frame to be examined is the frame at timeindex.
   */
  virtual bool GotoTime (uint32 timeindex) = 0;
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

/**
 * A video stream.
 */
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
   * Set the blendingmode that is used to combine this frame with the data in
   * the framebuffer Use the usual CS_FX_* values.  The default is CS_FX_COPY.
   */
  virtual bool SetFXMode (uint mode) = 0;
  /**
   * Call this if you want to use the frames as textures on your own polygons.
   */
  virtual iTextureHandle* NextFrameGetTexture () = 0;

};

SCF_VERSION (iAudioStream, 0, 0, 1);

/**
 * An audio stream.
 */
struct iAudioStream : public iStream
{
  /**
   * Retrieve descriptive stream information.
   */
  virtual void GetStreamDescription (csAudioStreamDescription &desc) = 0;
};

/**
 * Formats for videodata.
 * CHANNEL means all data of one kind in a row, like all red first then all
 * green etc.  INTERLEAVED means all components of one point in a row, for
 * instance 1st byte is red component of pixel 0, 2nd byte is green component
 * of pixel 0 etc.
 */

#define CS_CODECFORMAT_RGB_CHANNEL       1
#define CS_CODECFORMAT_RGBA_CHANNEL      2
#define CS_CODECFORMAT_YUV_CHANNEL       3
#define CS_CODECFORMAT_RGB_INTERLEAVED   4
#define CS_CODECFORMAT_RGBA_INTERLEAVED  5
#define CS_CODECFORMAT_YUV_INTERLEAVED   6

/// Formats for audiodata
#define CS_CODECFORMAT_PCM

/**
 * Codec description structure.
 */
struct csCodecDescription
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
  uint16 decodeoutput;
  /**
   * input data format
   */
  uint16 encodeinput;
};

#endif // __CS_IVIDEO_CODEC_H__
