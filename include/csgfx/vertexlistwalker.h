/*
  Copyright (C) 2005 by Marten Svanfeldt
            (C) 2006 by Frank Richter

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

#ifndef __CS_CSGFX_VERTEXLISTWALKER_H__
#define __CS_CSGFX_VERTEXLISTWALKER_H__

#include "cstool/rbuflock.h"

/**\file
 * Helper to access elements from renderbuffers.
 */

/**
 * Helper class to make it easier to iterate over elements from
 * renderbuffers, with automatic conversion to \a Tbase. Elements
 * can also be queried as \a Tcomplex, which must consist of multiple
 * \a Tbase.
 * \remarks The element actually returned (converted or not) is stored 
 *  in an array internal to the class. That has the consequence that for a
 *  queried element, you will only get what you expect as long as 
 *  operator++() is not called. If you need to retain an element, you
 *  have to copy it! This also means you cannot really manipulate the
 *  buffer this way...
 */
template<typename Tbase, typename Tcomplex = Tbase>
class csVertexListWalker
{
public:
  /**
   * Construct new walker.
   * \param buffer The render buffer to walk over.
   * \param desiredComponents Number of components to fetch from the buffer.
   *   Those can differ from the real number; the components are filled up
   *   with default values, if necessary. If 0, the component count as
   *   set in the buffer will be returned.
   * \param defaultComponents The default values for components, if more
   *   are requested than available. If 0, the values {0,0,0,1} are used.
   */
  csVertexListWalker (iRenderBuffer* buffer, size_t desiredComponents = 0, 
    const Tbase* defaultComponents = 0) : currElement (0), bufLock (buffer), 
    defaultComponents (defaultComponents)
  {
    bufferComponents = buffer->GetComponentCount ();
    components = (desiredComponents != 0) ? desiredComponents : 
      bufferComponents;
    CS_ASSERT (components <= maxComponents);
    elements = buffer->GetElementCount();
    compType = buffer->GetComponentType();
    FetchCurrentElement();
  }

  //@{
  /// Get current element.
  operator Tbase const*() const
  {
    CS_ASSERT(currElement<elements);
    return convertedComps;
  }
  const Tcomplex& operator*() const
  {
    CS_ASSERT(currElement<elements);
    const Tbase* x = convertedComps;
    const Tcomplex* y = (Tcomplex*)x;
    return *y;
  }
  //@}

  //@{
  /// Set current element to the next.
  csVertexListWalker& operator++ ()
  {
    currElement++;
    FetchCurrentElement();
    return *this;
  }
  //@}

  /// Start from the beginning
  void ResetState ()
  {
    currElement = 0;
    FetchCurrentElement();
  }
private:
  /// Number of elements
  size_t elements;
  /// Index of current element
  size_t currElement;
  /// Lock to the buffer in question
  csRenderBufferLock<uint8> bufLock;
  
  /// Number of components to return
  size_t components;
  /// Number of components in the buffer
  size_t bufferComponents;
  /// Max number of components this class can handle
  enum { maxComponents = 4 };
  /// Converted components are stored here
  Tbase convertedComps[maxComponents];
  /// Default component values
  const Tbase* const defaultComponents;
  /// Component type
  csRenderBufferComponentType compType;
  
  /// Obtain a default component value
  const Tbase GetDefaultComponent (size_t n)
  {
    return (defaultComponents != 0) ? defaultComponents[n] : 
      ((n == 3) ? Tbase(1) : Tbase(0));
  }
  /// Fetch a component, convert to T
  template<typename C>
  void FetchCurrentElementReal()
  {
    uint8* data = bufLock + (currElement * bufferComponents * sizeof (C));
    for (size_t c = 0; c < components; c++)
    {
      convertedComps[c] = 
	(c < bufferComponents) ? Tbase(*(C*)data) :
	GetDefaultComponent (c);
      data += sizeof (C);
    }
  }
  /// Fetch a component based on buffer component type
  void FetchCurrentElement()
  {
    /* Avoid reading beyond buffer end
     * Don't assert here since FetchCurrentElement() is also called in
     * legitimate cases (i.e. operator++ while on the last valid element.
     * Assert in retrieval operators, though.
     */
    if (currElement >= elements) return;
    switch (compType)
    {
      default:
	CS_ASSERT(false);
      case CS_BUFCOMP_BYTE:
        FetchCurrentElementReal<char>();
	break;
      case CS_BUFCOMP_UNSIGNED_BYTE:
        FetchCurrentElementReal<unsigned char>();
	break;
      case CS_BUFCOMP_SHORT:
        FetchCurrentElementReal<short>();
	break;
      case CS_BUFCOMP_UNSIGNED_SHORT:
        FetchCurrentElementReal<unsigned short>();
	break;
      case CS_BUFCOMP_INT:
        FetchCurrentElementReal<int>();
	break;
      case CS_BUFCOMP_UNSIGNED_INT:
        FetchCurrentElementReal<unsigned int>();
	break;
      case CS_BUFCOMP_FLOAT:
        FetchCurrentElementReal<float>();
	break;
      case CS_BUFCOMP_DOUBLE:
        FetchCurrentElementReal<double>();
	break;
    }
  }
};

#endif // __CS_CSGFX_VERTEXLISTWALKER_H__
