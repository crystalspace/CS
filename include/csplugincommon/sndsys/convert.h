/*
    Copyright (C) 2004 by Andrew Mann

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

#ifndef SNDSYS_CONVERT_H
#define SNDSYS_CONVERT_H

// Some helper functions for sound elements

// This is the maximum number of channels we'll mix.
// Mono sound is 1 channel
// Stereo is 2 channels
// 5.1 sound is 6 channels
#define SOUND_ELEMENT_MAX_CHANNELS 8


/** This setting controls how many steps are used internally between source samples.
 *
 *  Time is steped along based on the ratio of the input frequency to the output frequency.  If the output frequency is
 *   twice as fast as the input frequency, then two times as many sample points are needed in the output as in the input.
 *  In order to generate samples that sound appropriate when the time is between two actual source samples, 
 *    the amplitude of the sound wave at the given time is approximated by interpolating between two adjacent source samples.
 *  The position in the source data is internally represented by a signed integer which can be thought of as the sample number
 *    multiplied by this divisor value.  The first sample is at INTERNAL_FREQUENCY_DIVISOR, the second is at 2*INTERNAL_FREQUENCY_DIVISOR.
 *    A position mid way between the two is at 1.5 * INTERNAL_FREQUENCY_DIVISOR.
 *
 *  In order to keep things quick, use a power of 2 here (power of 2 division and multiplication will be optimized into a bit shift).
 *   This value is multiplied with frequency values and stored in a signed integer.  Beware of making it too big!
 */

#define INTERNAL_FREQUENCY_DIVISOR 1024

/** A PCMSampleConverter object converts PCM data from one format to another.
 *
 *  The converter can convert between different number of channels, bits per sample and sampling frequency.
 *
 *  UNCOMPRESSED PCM SAMPLES ONLY!
 *
 *  8 and 16 bit samples are supported.
 *  
 *  When converting between streams with different numbers of channels, mono source streams will have audio data copied to the second channel.
 *   If the destination is a stereo (2 channel) output, this will duplicate the mono/left stream onto the right stream.  Additional channels
 *   will be left in silence.  Converting a mono sample to a 5.1 sample will result in identical samples in streams 0 and 1 and silence in
 *   all other streams.
 *
 *  Frequency conversion is performed using linear interpolation between adjacent samples.  If a direct-ratio conversion is performed 
 *   (for example, if a 44100 khz stream is converted into a 11025 stream) then only direct data points will be considered and the rest
 *   will be thrown out.
 *
 *  Frequency conversion can be used to alter the playback speed of stream.  If the output format should be 22050 khz and you specify
 *   11025 khz, the sound will play twice as fast (no, this isn't a typo - the slower output data is read at a faster rate, causing
 *   the data to be traversed faster).  Internally, speed can be stepped in 1/INTERNAL_FREQUENCY_DIVISOR increments from the source frequency.
 *   This means if you specify more accurate than this increment, truncation to the next lowest value will occur. If you have a source at 
 *   44100 khz and specify an output at 44101 khz, the rate will most likely be reduced to 44100 khz - so no
 *   frequency conversion will take place.
 *
 *  Beware of very high frequency conversion rates!  If your source audio is of poor sampling rate (11025 khz), increasing the speed
 *   of playback will make it sound worse!  At some point it will degenerate into indistinguishable noise.  If you intend to speed
 *   up playback it's best to start with a higher sampling rate (44100 khz).
 */
class PCMSampleConverter
{
public:
  /// Create a PCMSapmeConverter with initial source properties provided.
  PCMSampleConverter(int source_channels, int source_bitspersample, int source_frequency, bool swap16=false);
  ~PCMSampleConverter();

  /** Return a multiplier that relates input buffer samples/size to output buffer samples/size.  Note that in many cases one extra source sample should be presumed to be present.
   *  To be safe, use ((source_samples+1)*source_sample_size*GetRequiredOutputBufferMultiple()) for the size.   The value returned will be
   *  constant for a given source format and destination format.
   * 
   * @param dest_channels
   * @param dest_bitspersample
   * @param dest_frequency
   */
  int GetRequiredOutputBufferMultiple(int dest_channels, int dest_bitspersample, int dest_frequency);


  /** Convert a buffer of PCM data from the currently set source format to the desired dest format.
   *
   * @param source
   * @param source_len
   * @param dest
   * @param dest_channels
   * @param dest_bitspersample
   * @param dest_frequency
   */
  int ConvertBuffer(const void *source, int source_len, void *dest, int dest_channels, int dest_bitspersample, int dest_frequency);

  /** Adjust the source properties for data being presented in the next call to ConvertBuffer.
   *
   *  This is primarily intended for variable-rate audio formats such as some Ogg and MP3 files.
   *  NOTE: At this time this function does not properly consider the transition between formats at the proper location
   *        and instead presumes the transition is at the last "step" from the previous buffer.
   *
   * @param source_channels
   * @param source_bitspersample
   * @param source_frequency
   */
  void SetSourceProperties(int source_channels, int source_bitspersample, int source_frequency);



protected:
  bool ReadFullSample8(const void **source, int *source_len, int *sample_buffer);
  bool ReadFullSample16(const void **source, int *source_len, int *sample_buffer);
  bool ReadFullSample(const void **source, int *source_len, int *sample_buffer);
  int WriteSample8(int *sample_buffer, void **dest, int dest_channels);
  int WriteSample16(int *sample_buffer, void **dest, int dest_channels);
  int WriteSample(int *sample_buffer, void **dest, int dest_channels, int dest_bitspersample);
  int AdvanceSourceSamples(const void **source, int *source_len, int samples_to_advance, int *sample_buffer);


protected:
  int last_sample[SOUND_ELEMENT_MAX_CHANNELS];
  int position_offset;
  int src_channels,src_bytes,src_frequency;
  bool swap_16;

};


#endif // #ifndef SNDSYS_CONVERT_H



