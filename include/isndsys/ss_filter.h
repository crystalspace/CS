/*
    Copyright (C) 2005 by Andrew Mann

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

#ifndef __CS_SNDSYS_FILTER_H__
#define __CS_SNDSYS_FILTER_H__

/**\file
 * Sound system: software filters
 */

#include "csutil/scf.h"
#include "isndsys/ss_structs.h"

/**\addtogroup sndsys
 * @{ */

class csSourceParameters3D;
class csListenerProperties;
struct iReporter;

struct iSndSysSoftwareFilter3DProperties
{
  csSoundSample *clean_buffer;
  csSoundSample *work_buffer;
  size_t buffer_samples;
  csSourceParameters3D *source_parameters;
  csListenerProperties *listener_parameters;
  csSndSysSoundFormat *sound_format;

  float closest_speaker_distance;
  float *speaker_distance;
  float *speaker_direction_cos;
  size_t channel;
};

SCF_VERSION (iSndSysSoftwareFilter3D, 0, 1, 0);

/**
 * A sound filter is an interface to an object that modifies sequences of
 * sound samples.
 */
struct iSndSysSoftwareFilter3D : public iBase
{
  /**
   * Apply this filter to the mutable buffer passed.  The unmutable main
   * buffer is also passed, although this is not likely to be
   * very useful, since the main buffer has an unknown number of sources
   * previously mixed in (possibly none).
   * The sample_count is the number of samples available in both the mutable
   * buffer and the main buffer.
   * The format is the format of the audio.
   */
  virtual void Apply(iSndSysSoftwareFilter3DProperties &properties) = 0;

  virtual bool AddSubFilter(iSndSysSoftwareFilter3D *filter,
  	int chain_idx=0) = 0;

  virtual iSndSysSoftwareFilter3D *GetSubFilter(int chain_idx=0) = 0;

  /**
   * Retrieve the base pointer for this filter.  Used internally by the
   * sound system.
   */
  virtual iSndSysSoftwareFilter3D *GetPtr() = 0;
};

SCF_VERSION (iSndSysSoftwareFilterOutput, 0, 1, 0);

/**
 * An output sound filter receives 
 */
struct iSndSysSoftwareFilterOutput : public iBase
{
  /**
   * Apply this filter to the mutable buffer passed.  The unmutable main
   * buffer is also passed, although this is not likely to be
   * very useful, since the main buffer has an unknown number of sources
   * previously mixed in (possibly none).
   * The sample_count is the number of samples available in both the mutable
   * buffer and the main buffer.
   * The format is the format of the audio.
   */
  virtual void Apply(iSndSysSoftwareFilter3DProperties &properties) = 0;

  virtual bool AddSubFilter(iSndSysSoftwareFilter3D *filter,
  	int chain_idx=0) = 0;

  virtual iSndSysSoftwareFilter3D *GetSubFilter(int chain_idx=0) = 0;

  /**
   * Retrieve the base pointer for this filter.  Used internally by the
   * sound system.
   */
  virtual iSndSysSoftwareFilter3D *GetPtr() = 0;
};

/** @} */

#endif // __CS_SNDSYS_FILTER_H__
