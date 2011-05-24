/*
    Crystal Space utility library: MD5 class
    Original C code written by L. Peter Deutsch (see below)
    Adapted for Crystal Space by Michael Dale Long
    Completely re-engineered by Eric Sunshine <sunshine@sunshineco.com>

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

/*
  Copyright (C) 1999 Aladdin Enterprises.  All rights reserved.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch
  ghost@aladdin.com

 */
/**\file
  Independent implementation of MD5 (RFC 1321).

  This code implements the MD5 Algorithm defined in RFC 1321.
  It is derived directly from the text of the RFC and not from the
  reference implementation.

  The original and principal author of md5.h is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):

  1999-11-04 lpd Edited comments slightly for automatic TOC extraction.
  1999-10-18 lpd Fixed typo in header comment (ansi2knr rather than md5);
	added conditionalization for C++ compilation from Martin
	Purschke <purschke@bnl.gov>.
  1999-05-03 lpd Original version.
 */

#ifndef __CS_MD5_H__
#define __CS_MD5_H__

#include "csextern.h"
#include "csutil/csstring.h"
#include "csutil/digest.h"
#include "csutil/hash.h"

namespace CS
{
  namespace Utility
  {
    namespace Checksum
    {
      /**
       * Compute a MD5 message digest.
       * This is an encapsulation of a C-implementation of MD5 digest algorithm by
       * Peter Deutsch <ghost@aladdin.com>.
       * 
       * It provides an interface to compute a digest in a "streaming" manner
       * (the message can be split into chunks which are processed
       * sequentially) as well as a convenient interface which allows one to
       * create a digest in a single step.
       */
      class CS_CRYSTALSPACE_EXPORT MD5
      {
      protected:
	/// 8-bit byte
	typedef uint8 md5_byte_t;
	/// 32-bit word
	typedef uint32 md5_word_t;

	//@{
	/// State of the MD5 Algorithm.
	size_t count[2]; // message length in bits, lsw first
	md5_word_t abcd[4];  // digest buffer
	md5_byte_t buf[64];  // accumulate block
	//@}

	void Process(const md5_byte_t* data/*[64]*/);

	/// Update hash with a (at most) 4GB chunk of data
	void AppendInternal (const uint8* input, uint32 length);
      public:
	/// An MD5 digest is 16 unsigned characters (not 0-terminated).
	typedef CS::Utility::Checksum::Digest<16> Digest;
	
	MD5 ();

	/**
	 * Used to update the the input data hash.
	 * \param input A pointer to an array of the input data with which to update the hash.
	 * \param length The length of the input data to hash (in bytes).
	 */
	void Append (const uint8* input, size_t length);
	
	/** 
	 * Used to complete the hashing process and obtain the calculated hash.
	 * After finishing, don't append further data to the hash --
	 * the resulting digest will be bogus.
	 * \return The calculated hash.
	 */
	Digest Finish ();

	/// Encode a string.
	static Digest Encode(csString const&);
	/// Encode a null-terminated string buffer.
	static Digest Encode(const char*);
	/// Encode a buffer.
	static Digest Encode(const void*, size_t nbytes);
      };
    } //namespace Checksum
  } //namespace Utility
} //namespace CS

typedef CS_DEPRECATED_TYPE_MSG("Use CS::Utility::Checksum::MD5 instead")
  CS::Utility::Checksum::MD5 csMD5;

#endif // __CS_MD5_H__

