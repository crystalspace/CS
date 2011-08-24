/*
  Copyright (C) 2011 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSUTIL_DIGEST_H__
#define __CS_CSUTIL_DIGEST_H__

/**\file
 * Message digest data
 */

#include "csutil/csstring.h"
#include "csutil/hash.h"

namespace CS
{
  namespace Utility
  {
    namespace Checksum
    {
      /// Helper class to provide Digest<> formatting methods
      struct CS_CRYSTALSPACE_EXPORT DigestFormat
      {
	/// Returns a lowercase hex-string representing some raw digest data.
	static csString HexString (const uint8* data, uint size);
	/// Returns an uppercase hex-string representing some raw digest data.
	static csString HEXString (const uint8* data, uint size);
      };
      
      /**
       * Message digest data with a size of \a Size bytes.
       */
      template<uint Size>
      class Digest : protected DigestFormat
      {
      public:
	enum { DigestLen = Size };
	/// The raw digest data.
	uint8 data[DigestLen];
	/// Returns a lowercase hex-string representing the raw digest data.
	csString HexString() const
	{ return DigestFormat::HexString (data, Size); }
	/// Returns an uppercase hex-string representing the raw digest data.
	csString HEXString() const
	{ return DigestFormat::HEXString (data, Size); }
	
	bool operator==(const Digest& other) const
	{ return memcmp (data, other.data, sizeof (data)) == 0; }
	bool operator!=(const Digest& other) const
	{ return memcmp (data, other.data, sizeof (data)) != 0; }
      };
    } // namespace Checksum
  } // namespace Utility
} // namespace CS

template<uint Size>
class csHashComputer<CS::Utility::Checksum::Digest<Size> > : 
  public csHashComputerStruct<CS::Utility::Checksum::Digest<Size> > {};

template<uint Size>
class csComparator<CS::Utility::Checksum::Digest<Size> > : 
  public csComparatorStruct<CS::Utility::Checksum::Digest<Size> > {};

#endif // __CS_CSUTIL_DIGEST_H__
