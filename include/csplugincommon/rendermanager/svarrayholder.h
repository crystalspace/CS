/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SVARRAYHOLDER_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SVARRAYHOLDER_H__

/**\file
 * Holder for shader variable arrays.
 */
class csShaderVariable;

namespace CS
{
namespace RenderManager
{

  /**
   * Holder for SV arrays.
   *
   * Keeps a continuous array of pointers to SVs kept in three layers
   * - Layers, where each layer is a rendering layer
   * - Sets, each set corresponds normally to one rendermesh
   * - Names, one for each SV
   *
   * The 3d array is flattened into a 1d one and indexed as:
   * index = (layer*numSets + set)*numSVs + SV
   */
  class SVArrayHolder
  {
  public:
    /**
     * Construct. Calls Setup if \a numLayers, \a numSVNames and \a numSets are
     * provided.
     */
    SVArrayHolder (size_t numLayers = 1, size_t numSVNames = 0, size_t numSets = 0)
      : numLayers (numLayers), numSVNames (numSVNames), numSets (numSets), svArray (0),
        memAllocSetUp (false)
    {
      if (numSVNames && numSets && numLayers)
        Setup (numLayers, numSVNames, numSets);
    }

    SVArrayHolder (const SVArrayHolder& other)
      : svArray (0), memAllocSetUp (false)
    {
      *this = other;
    }

    ~SVArrayHolder ()
    {
      if (memAllocSetUp) GetMemAlloc().~csMemoryPool();
    }

    SVArrayHolder& operator= (const SVArrayHolder& other)
    {
      if (memAllocSetUp) GetMemAlloc().~csMemoryPool();

      numLayers = other.numLayers;
      numSVNames = other.numSVNames;
      numSets = other.numSets;

      const size_t sliceSVs = numSVNames*numSets;
      const size_t sliceSize = sizeof(csShaderVariable*)*sliceSVs;
#include "csutil/custom_new_disable.h"
      new (&memAlloc) csMemoryPool (sliceSize * 4);
#include "csutil/custom_new_enable.h"
      memAllocSetUp = true;

      csShaderVariable** superSlice = reinterpret_cast<csShaderVariable**> (
        GetMemAlloc().Alloc (numLayers * sliceSize));

      for (size_t l = 0; l < numLayers; l++)
      {
        csShaderVariable** slice = superSlice + l * sliceSVs;
        svArray.Push (slice);
        memcpy (slice, other.svArray[l], sliceSize);
      }
      svArray.ShrinkBestFit();

      return *this;
    }

    /**
     * Initialize storage for SVs, given a number of layers, sets and SV names,
     * Note that additional layers can be inserted later, sets and SVs cannot.
     */
    void Setup (size_t numLayers, size_t numSVNames, size_t numSets)
    {
      this->numLayers = numLayers;
      this->numSVNames = numSVNames;
      this->numSets = numSets;

      const size_t sliceSVs = numSVNames*numSets;
      const size_t sliceSize = sizeof(csShaderVariable*)*sliceSVs;
#ifndef DOXYGEN_RUN
#include "csutil/custom_new_disable.h"
#endif
      new (&memAlloc) csMemoryPool (sliceSize * 4);
#ifndef DOXYGEN_RUN
#include "csutil/custom_new_enable.h"
#endif
      memAllocSetUp = true;

      csShaderVariable** superSlice = reinterpret_cast<csShaderVariable**> (
        GetMemAlloc().Alloc (numLayers * sliceSize));
      memset (superSlice, 0, numLayers * sliceSize);

      for (size_t l = 0; l < numLayers; l++)
      {
        csShaderVariable** slice = superSlice + l * sliceSVs;
        svArray.Push (slice);
      }
      svArray.ShrinkBestFit();

    }

    /**
     * Setup an SV stack for direct access to given layer and set within SV
     * array
     */
    void SetupSVStack (csShaderVariableStack& stack, size_t layer, size_t set)
    {
      CS_ASSERT (layer < numLayers);
      CS_ASSERT (set < numSets);

      stack.Setup (svArray[layer] + set*numSVNames, numSVNames);
    }

    /**
     * Replicate the pointers from one set to a number or all the other sets within
     * the layer
     */
    void ReplicateSet (size_t layer, size_t from, size_t start, size_t end = (size_t)-1)
    {
      if (numSets == 1)
        return;

      if (end == (size_t)-1)
        end = numSets-1;

      CS_ASSERT (layer < numLayers);
      CS_ASSERT (from < numSets && start < numSets && end < numSets);
      CS_ASSERT (from < start || from > end);

      for (size_t i = start; i <= end; ++i)
      {
        memcpy (svArray[layer] + i*numSVNames, 
          svArray[layer] + from*numSVNames,
          sizeof(csShaderVariable*)*numSVNames);
      }
    }

    /**
     * Replicate layer zero into all other layers
     */
    void ReplicateLayerZero ()
    {
      if (numLayers == 1)
        return;

      size_t layerSize = numSets*numSVNames;

      for (size_t layer = 1; layer < numLayers; ++layer)
      {
        memcpy (svArray[layer], svArray[0], sizeof(csShaderVariable*)*layerSize);
      }
    }

    /**
     * Replicate a layer into some other layer
     */
    void ReplicateLayer (size_t from, size_t to)
    {
      size_t layerSize = numSets*numSVNames;

      memcpy (svArray[to], svArray[from], sizeof(csShaderVariable*)*layerSize);
    }

    /**
     * Insert a layer after \a after, copying values from \a replicateFrom.
     */
    void InsertLayer (size_t after, size_t replicateFrom = 0)
    {
      const size_t sliceSize = sizeof(csShaderVariable*)*numSVNames*numSets;

      csShaderVariable** slice = reinterpret_cast<csShaderVariable**> (
        GetMemAlloc().Alloc (sliceSize));
      svArray.Insert (after+1, slice);

      memcpy (slice, svArray[replicateFrom], sliceSize);

      numLayers++;
    }

    /// Get the number of shader variables stored per layer.
    size_t GetNumSVNames () const
    {
      return numSVNames;
    }

    /// Get the number of layers.
    size_t GetNumLayers () const
    {
      return numLayers;
    }

  private:
    size_t numLayers;
    size_t numSVNames;
    size_t numSets;
    csArray<csShaderVariable**> svArray;

    csMemoryPool& GetMemAlloc()
    { 
      union
      {
        uint* a;
        csMemoryPool* b;
      } pun;
      pun.a = memAlloc;
      return *(pun.b); 
    }

    uint memAlloc[(sizeof(csMemoryPool) + sizeof (uint) - 1) / sizeof (uint)];
    bool memAllocSetUp;
  };


}
}

#endif
