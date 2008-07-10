/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __SAMPLER_H__
#define __SAMPLER_H__

// Define different forms of sampling methods

namespace lighter
{

  /**
   * Radical inverse in given base.
   * 
   * A given number n in base b it can be written as
   * n = Sum n_i * b^(i-1)
   * The radical inverse is a number between 0 and 1 defined by
   * ri = Sum n_i * b^-i
   *
   * This is used to generate a number of low discrepancy sequences
   */
  template<typename T>
  inline T RadicalInverse (uint n, int base)
  {
    T result = 0;
    T invBase = (T)1.0 / base, invBi = invBase;
    while (n > 0)
    {
      // Accumulate result
      int n_i = n % base;
      result += n_i * invBi;

      // Next digit
      n /= base;
      invBi *= invBase;
    }
    return result;
  }

  /**
   * Compute an N dimensional Halton sequence (N <= 10)
   */
  template<typename T, int N>
  inline void HaltonSequence (uint seqIdx, T (&result)[N])
  {
    static const int PrimeNumbers[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    
    for (size_t i = 0; i < N; ++i)
    {
      result[i] = RadicalInverse<T> (seqIdx, PrimeNumbers[i]);
    }
  }

  /**
   * Compute an one dimensional van der Corput sequence
   */
  template<typename T>
  inline T VanDerCorputSequence (uint index, int scramble = 0)
  {
    index = (index << 16) | (index >> 16);
    index = ((index & 0x00ff00ff) << 8) | ((index & 0xff00ff00) >> 8);
    index = ((index & 0x0f0f0f0f) << 4) | ((index & 0xf0f0f0f0) >> 4);
    index = ((index & 0x33333333) << 2) | ((index & 0xcccccccc) >> 2);
    index = ((index & 0x55555555) << 1) | ((index & 0xaaaaaaaa) >> 1);
    index ^= scramble;

    return (T)index / (T)CONST_INT64(0x100000000);
  }

  /**
   * Helper for below
   */
  class SampleSequenceIndex : public csRefCount
  {
  public:
    SampleSequenceIndex ()
      : sequenceIndex (1)
    {
    }

    uint GetNext ()
    {
      return sequenceIndex++;
    }

  private:
    uint sequenceIndex;
  };

  /**
   * Simple class that provides low discrepancy numbers of given dimension
   */
  template<int N>
  class SamplerSequence
  {
  public:
    SamplerSequence ()
    {
      seqIndexHolder.AttachNew (new SampleSequenceIndex);
    }

    SamplerSequence (const SamplerSequence& other)
    {
      seqIndexHolder = other.GetIndexHolder ();
    }

    template<int M>
    SamplerSequence (const SamplerSequence<M>& other)
    {
      seqIndexHolder = other.GetIndexHolder ();
    }

    SamplerSequence& operator= (const SamplerSequence& other)
    {
      seqIndexHolder = other.GetIndexHolder ();
      return *this;
    }

    template<int M>
    SamplerSequence<N>& operator= (const SamplerSequence<M>& other)
    {
      seqIndexHolder = other.GetIndexHolder ();
      return *this;
    }

    /// Get next number in sequence
    void GetNext (float (&result)[N])
    {
      HaltonSequence (seqIndexHolder->GetNext (), result);
    }

    csRef<SampleSequenceIndex> GetIndexHolder() const
    {
      return seqIndexHolder;
    }
  private:
    csRef<SampleSequenceIndex> seqIndexHolder;
  };
}

#endif
