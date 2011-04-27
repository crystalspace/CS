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

#include "cssysdef.h"


#include "compressor.h"



csSoundCompressor::csSoundCompressor(int HistorySamples)
{
  m_HistorySamples=HistorySamples;
  m_pHistory=new csSoundSample[m_HistorySamples];

  m_CompressionAttackSamples=100;
  m_CompressionReleaseSamples=100;
  m_CompressionThreshold=0x7FFFFFFF;

  SetCompressionRatio(4);
  Reset();
}

csSoundCompressor::~csSoundCompressor()
{
  delete[] m_pHistory;
}

void csSoundCompressor::Reset()
{
  // Clear the history buffer to 0
  memset(m_pHistory, 0, sizeof(csSoundSample) * m_HistorySamples);
  m_HistorySum=0;
  m_HistoryPosition=0;
  m_CurrentCompressionX1024=0;
}

void csSoundCompressor::SetCompressionThreshold (csSoundSample MaxIntensity)
{
  m_CompressionThreshold = abs (MaxIntensity);
}

void csSoundCompressor::SetCompressionRatio (float CompressionRatio)
{
  m_CompressionRatioX1024 = (int)(CompressionRatio*1024.0f);
  m_CompressionAttackStepX1024 = m_CompressionRatioX1024 / (int)m_CompressionAttackSamples;
  m_CompressionReleaseStepX1024 = m_CompressionRatioX1024 / (int)m_CompressionReleaseSamples;
}

void csSoundCompressor::SetCompressionAttackSamples (size_t Samples)
{
  m_CompressionAttackSamples = Samples;
  m_CompressionAttackStepX1024 = m_CompressionRatioX1024 / (int)Samples;
}

void csSoundCompressor::SetCompressionReleaseSamples (size_t Samples)
{
  m_CompressionReleaseSamples = Samples;
  m_CompressionReleaseStepX1024 = m_CompressionRatioX1024 / (int)Samples;
}


void csSoundCompressor::ApplyCompression (csSoundSample *pSampleBuffer, size_t Samples)
{
  size_t SampleIDX;
  csSoundSample CurrentSample;
  csSoundSample MaxSample=0;

  // We also want to normalize the entire buffer to fill the 32 available bits

  for (SampleIDX=0;SampleIDX<Samples;SampleIDX++)
  {
    // Remove the oldest history entry
    m_HistorySum-=m_pHistory[m_HistoryPosition];
    // Add the new entry to the history buffer and the average
    CurrentSample=abs(pSampleBuffer[SampleIDX]);
    m_pHistory[m_HistoryPosition]=CurrentSample;
    m_HistorySum+=CurrentSample;

    // Determine whether the average is above the threshold
    if ((size_t)(ABS(m_HistorySum)) > (m_CompressionThreshold * m_HistorySamples))
    {
      // Increase the compression factor unless we're already capped
      if (m_CurrentCompressionX1024 < m_CompressionRatioX1024)
      {
        m_CurrentCompressionX1024 += m_CompressionAttackStepX1024;
        if (m_CurrentCompressionX1024 > m_CompressionRatioX1024)
          m_CurrentCompressionX1024 = m_CompressionRatioX1024;
      }
    }
    else
    {
      if (m_CurrentCompressionX1024 > 0)
      {
        m_CurrentCompressionX1024 -= m_CompressionReleaseStepX1024;
        if (m_CurrentCompressionX1024 < 0)
          m_CurrentCompressionX1024 = 0;
      }
    }
    // Apply the compression to the sample
    if (m_CurrentCompressionX1024>0 && CurrentSample>m_CompressionThreshold)
      CurrentSample= m_CompressionThreshold + ((CurrentSample-m_CompressionThreshold) * 1024 / (1024+m_CurrentCompressionX1024));

    // Record the max sample
    if (CurrentSample>MaxSample)
      MaxSample=CurrentSample;

    // Place the sample back into the original buffer
    if (pSampleBuffer[SampleIDX]<0)
      pSampleBuffer[SampleIDX]=-CurrentSample;
    else
      pSampleBuffer[SampleIDX]=CurrentSample;

    // Roll to the next history entry
    m_HistoryPosition++;
    if (m_HistoryPosition==m_HistorySamples)
      m_HistoryPosition=0;
  }

  // Fill to ~ 32 bits
  if (MaxSample>0)
  {
    int SampleMult = 0x7FFFFFFF / MaxSample;
    if (SampleMult>1)
    {
      for (SampleIDX=0;SampleIDX<Samples;SampleIDX++)
        pSampleBuffer[SampleIDX]= pSampleBuffer[SampleIDX] * SampleMult;

    }
  }

}


