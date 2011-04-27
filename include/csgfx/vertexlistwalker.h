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

#include "csutil/csendian.h"
#include "cstool/rbuflock.h"

/**\file
 * Helper to access elements from renderbuffers.
 */

/**
 * Helper class to make it easier to iterate over elements from
 * renderbuffers, with automatic conversion to \a Tbase. Elements
 * can also be queried as \a Tcomplex, which must consist of multiple
 * \a Tbase and support left- and right-value operator[].
 * \remarks The element actually returned (converted or not) is stored 
 *  in an array internal to the class. That has the consequence that for a
 *  queried element, you will only get what you expect as long as 
 *  operator++() is not called. If you need to retain an element, you
 *  have to copy it! This also means you cannot really manipulate the
 *  buffer this way...
 */
template<typename Tbase, typename Tcomplex = Tbase[4]>
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
    const Tbase* defaultComponents = 0) : currElement (0), 
    bufLock (buffer, CS_BUF_LOCK_READ), defaultComponents (defaultComponents)
  {
    bufferComponents = buffer ? buffer->GetComponentCount () : 0;
    components = (desiredComponents != 0) ? desiredComponents : 
      bufferComponents;
    if (buffer)
    {
      elements = buffer->GetElementCount();
      compType = buffer->GetComponentType();
      FetchCurrentElement();
    }
    else
    {
      elements = 0;
      compType = (csRenderBufferComponentType)~0;
    }
  }

  //@{
  /// Get current element.
  operator Tcomplex const& () const
  {
    CS_ASSERT(currElement<elements);
    return converted;
  }
  const Tcomplex& operator*() const
  {
    CS_ASSERT(currElement<elements);
    return converted;
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
  
  /// Get number of elements in the iterated buffer.
  size_t GetSize() const { return elements; }
  
  /// Set position to a specific element
  void SetElement (size_t newElement)
  {
    CS_ASSERT(newElement < elements);
    currElement = newElement;
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
  /// Converted components are stored here
  Tcomplex converted;
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
  //@{
  /// Fetch a component, convert to T
  template<typename C>
  void FetchCurrentElementReal()
  {
    uint8* data = bufLock + (currElement * bufferComponents * sizeof (C));
    for (size_t c = 0; c < components; c++)
    {
      converted[c] = 
	(c < bufferComponents) ? Tbase(*(C*)data) :
	GetDefaultComponent (c);
      data += sizeof (C);
    }
  }

  void FetchCurrentElementHalf()
  {
    uint16* data = (uint16*)((uint8*)bufLock) + (currElement * bufferComponents);
    for (size_t c = 0; c < components; c++)
    {
      converted[c] = 
	(c < bufferComponents) ? Tbase (csIEEEfloat::ToNative (*data)) :
	GetDefaultComponent (c);
      data++;
    }
  }
  
  template<typename C, bool Signed, int range>
  void FetchCurrentElementRealNorm()
  {
    uint8* data = bufLock + (currElement * bufferComponents * sizeof (C));
    for (size_t c = 0; c < components; c++)
    {
      Tbase newComp;
      if (c < bufferComponents)
      {
        double orgVal = double (*(C*)data);
        if (Signed)
        {
          orgVal = (orgVal + (-range - 1)) / double ((int64)range*2+1);
          newComp = Tbase (-1.0 + orgVal * 2.0);
        }
        else
        {
          orgVal = orgVal / double (range);
          newComp = Tbase (orgVal);
        }
      }
      else
        newComp = GetDefaultComponent (c);
      converted[c] = newComp;
      data += sizeof (C);
    }
  }
  //@}
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
      case CS_BUFCOMP_BYTE_NORM:
        FetchCurrentElementRealNorm<char, true, 127>();
	break;
      case CS_BUFCOMP_UNSIGNED_BYTE:
        FetchCurrentElementReal<unsigned char>();
	break;
      case CS_BUFCOMP_UNSIGNED_BYTE_NORM:
        FetchCurrentElementRealNorm<unsigned char, false, 255>();
	break;
      case CS_BUFCOMP_SHORT:
        FetchCurrentElementReal<short>();
	break;
      case CS_BUFCOMP_SHORT_NORM:
        FetchCurrentElementRealNorm<short, true, 32767>();
	break;
      case CS_BUFCOMP_UNSIGNED_SHORT:
        FetchCurrentElementReal<unsigned short>();
	break;
      case CS_BUFCOMP_UNSIGNED_SHORT_NORM:
        FetchCurrentElementRealNorm<unsigned short, false, 65535>();
	break;
      case CS_BUFCOMP_INT:
        FetchCurrentElementReal<int>();
	break;
      case CS_BUFCOMP_INT_NORM:
        FetchCurrentElementRealNorm<int, true, 2147483647>();
	break;
      case CS_BUFCOMP_UNSIGNED_INT:
        FetchCurrentElementReal<unsigned int>();
	break;
      case CS_BUFCOMP_UNSIGNED_INT_NORM:
        FetchCurrentElementRealNorm<unsigned int, false, 4294967295u>();
	break;
      case CS_BUFCOMP_FLOAT:
        FetchCurrentElementReal<float>();
	break;
      case CS_BUFCOMP_DOUBLE:
        FetchCurrentElementReal<double>();
	break;
      case CS_BUFCOMP_HALF:
        FetchCurrentElementHalf ();
	break;
    }
  }
};

#endif // __CS_CSGFX_VERTEXLISTWALKER_H__
