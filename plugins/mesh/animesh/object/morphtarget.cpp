/*
  Copyright (C) 2008 by Frank Richter

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

#include "morphtarget.h"

#include "ivideo/rndbuf.h"

#include "animesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{
  MorphTarget::MorphTarget (AnimeshObjectFactory* parent, const char* name)
  : scfImplementationType (this), parent (parent), name (name)
  {}

  bool MorphTarget::SetVertexOffsets (iRenderBuffer* renderBuffer)
  {
    offsets = renderBuffer;
    return true;
  }

  void MorphTarget::Invalidate ()
  {
    CS_ASSERT (parent.IsValid ());
    if (parent->GetSubsetCount ())
    {
      CS_ASSERT (offsets->GetElementCount() <= parent->GetVertexCountP());   
    }
    else
    {
      CS_ASSERT (offsets->GetElementCount() == parent->GetVertexCountP());
    }
  }

  void MorphTarget::AddSubset (const size_t subset)
  {
    CS_ASSERT (parent.IsValid () && subset < parent->GetSubsetCount ());
    subsetList.Push (subset);
  }

  size_t MorphTarget::GetSubset (const size_t index) const
  {
    CS_ASSERT (index < subsetList.GetSize ());    
    return subsetList[index];
  }

  size_t MorphTarget::GetSubsetCount () const
  {
    return subsetList.GetSize ();
  }
}
CS_PLUGIN_NAMESPACE_END(Animesh)
