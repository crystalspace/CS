/*
  Copyright (C) 2004 by Jorrit Tyberghein
	    (C) 2004 by Jonathan Tarbox
	    (C) 2004 by Frank Richter

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

#ifndef __CS_CSUTIL_CSEVENTFLATTENER_H__
#define __CS_CSUTIL_CSEVENTFLATTENER_H__

#include "csextern.h"

/**\file
 * Event flattening helper
 */

struct iEvent;

/**
 * Event flattening/unflattening result.
 */
enum csEventFlattenerError
{
  /// No error.
  csEventFlattenerErrorNone,
  /**
   * An attribute contains an iBase interface; those can't be serialized.
   */
  csEventFlattenerErroriBaseEncountered,
  /// An error occured during retrieval of an attribute
  csEventFlattenerErrorAttributeRetrieval,
  /// The buffer data is not in the expected format.
  csEventFlattenerErrorWrongFormat
};

/**
 * Standard event flattener(also known as serializer).
 * Converts events from/to a binary representation. Can be used for e.g.
 * disk storage or network transfer.
 */
class CS_CRYSTALSPACE_EXPORT csEventFlattener
{
public:
  /// Query the size consumed by the flattened event.
  static csEventFlattenerError FlattenSize (iEvent* event, size_t& size);
  /**
   * Flatten am event.
   * \remark The user is responsible for allocating and deallocating the 
   *  buffer memory.
   */
  static csEventFlattenerError Flatten (iEvent* event, char *buffer);
  /// Unflatten an event.
  static csEventFlattenerError Unflatten (iEvent* event, const char *buffer, 
    size_t length);
}; 

#endif // __CS_CSUTIL_CSEVENTFLATTENER_H__
