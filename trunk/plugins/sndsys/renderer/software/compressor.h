/*
  Copyright (C) 2006 by Andrew Mann

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


#ifndef SNDSYS_RENDERER_SOFTWARE_COMPRESSOR_H
#define SNDSYS_RENDERER_SOFTWARE_COMPRESSOR_H

#include "csutil/cfgacc.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csgeom/vector3.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_renderer.h"
#include "isndsys/ss_eventrecorder.h"


class csSoundCompressor
{
public:
  csSoundCompressor(int HistorySamples=1000);
  ~csSoundCompressor();

  /// Reset the Average sample intensity 
  void Reset ();
  void ApplyCompression (csSoundSample *pSampleBuffer, size_t Samples);

  void SetCompressionThreshold (csSoundSample MaxIntensity);
  void SetCompressionRatio (float CompressionRatio);
  void SetCompressionAttackSamples (size_t Samples);
  void SetCompressionReleaseSamples (size_t Samples);


protected:
  csSoundSample m_CompressionThreshold;
  int m_CompressionRatioX1024;
  int m_CurrentCompressionX1024;
  int m_CompressionAttackStepX1024;
  int m_CompressionReleaseStepX1024;
  size_t m_CompressionAttackSamples;
  size_t m_CompressionReleaseSamples;
  csSoundSample m_HistorySum;
  csSoundSample *m_pHistory;
  size_t m_HistorySamples;
  size_t m_HistoryPosition;
};







#endif // #ifndef SNDSYS_RENDERER_SOFTWARE_COMPRESSOR_H


