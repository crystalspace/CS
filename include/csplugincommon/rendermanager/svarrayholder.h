/*
    Copyright (C) 2007 by Marten Svanfeldt

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

class csShaderVariable;

namespace CS
{
namespace RenderManager
{

  class SVArrayHolder
  {
  public:
    SVArrayHolder (size_t numLayers = 1, size_t numSVNames = 0, size_t numSets = 0)
      : numLayers (numLayers), numSVNames (numSVNames), numSets (numSets), svArray (0)
    {
      if (numSVNames && numSets && numLayers)
        Setup (numLayers, numSVNames, numSets);
    }

    SVArrayHolder (const SVArrayHolder& other)
      : svArray (0)
    {
      *this = other;
    }

    ~SVArrayHolder ()
    {
      cs_free (svArray);
    }

    SVArrayHolder& operator= (const SVArrayHolder& other)
    {
      cs_free (svArray);

      numLayers = other.numLayers;
      numSVNames = other.numSVNames;
      numSets = other.numSets;

      const size_t arraySize = sizeof(csShaderVariable*)*numLayers*numSVNames*numSets;
      svArray = static_cast<csShaderVariable**> (cs_malloc (arraySize));

      memcpy (svArray, other.svArray, arraySize);

      return *this;
    }

    void Setup (size_t numLayers, size_t numSVNames, size_t numSets)
    {
      this->numLayers = numLayers;
      this->numSVNames = numSVNames;
      this->numSets = numSets;

      const size_t arraySize = sizeof(csShaderVariable*)*numLayers*numSVNames*numSets;
      svArray = static_cast<csShaderVariable**> (cs_malloc (arraySize));
      memset (svArray, 0, arraySize);
    }

    void SetupSVStack (csShaderVariableStack& stack, size_t layer, size_t set)
    {
      CS_ASSERT (layer < numLayers);
      CS_ASSERT (set < numSets);

      stack.Setup (svArray + (layer*numSets + set)*numSVNames, numSVNames);
    }

    void ReplicateSet (size_t layer, size_t from, size_t start, size_t end = (size_t)-1)
    {
      if (numSets == 1)
        return;

      if (end == (size_t)-1)
        end = numSets-1;

      CS_ASSERT (layer < numLayers);
      CS_ASSERT (from < numSets && start < numSets && end < numSets);
      CS_ASSERT (from < start || from > end);

      size_t layerStart = layer*numSets*numSVNames;

      for (size_t i = start; i <= end; i++)
        memcpy (svArray + layerStart + i*numSVNames, svArray + layerStart + from*numSVNames,
          sizeof(csShaderVariable*)*numSVNames);
    }

    void ReplicateLayerZero ()
    {
      if (numLayers == 1)
        return;

      size_t layerSize = numSets*numSVNames;

      for (size_t layer = 1; layer < numLayers; ++layer)
      {
        memcpy (svArray + layer*layerSize, svArray, sizeof(csShaderVariable*)*layerSize);
      }
    }

    size_t GetNumSVNames () const
    {
      return numSVNames;
    }

    size_t GetNumLayers () const
    {
      return numLayers;
    }

  private:
    size_t numLayers;
    size_t numSVNames;
    size_t numSets;    
    csShaderVariable** svArray;
  };


}
}

#endif
