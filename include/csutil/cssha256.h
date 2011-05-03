/*
    Crystal Space utility library: SHA256 class
    Original C code written by Jesse Kornblum (see related source)
    Adapted for Crystal Space by Stefano Angeleri <weltall2@gmail.com>

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

#ifndef _CSSHA256_H
#define _CSSHA256_H

#include "csextern.h"
#include "cssysdef.h"
#include "csutil/csstring.h"
#include "csutil/hash.h"

class CS_CRYSTALSPACE_EXPORT csSHA256
{
public:

  /// A SHA256 digest is 32 unsigned characters (not 0-terminated).
  struct CS_CRYSTALSPACE_EXPORT Digest
  {
    enum { DigestLen = 32 };
    /// The raw digest data.
    uint8_t data[DigestLen];
    /// Returns a lowercase hex-string representing the raw digest data.
    csString HexString() const;
    /// Returns an uppercase hex-string representing the raw digest data.
    csString HEXString() const;
    
    bool operator==(const Digest& other) const
    { return memcmp (data, other.data, sizeof (data)) == 0; }
    bool operator!=(const Digest& other) const
    { return memcmp (data, other.data, sizeof (data)) != 0; }
  };

  ///A context used to generate the hash
  class Context
  {
    private:
    uint32_t total[2];
    uint32_t state[8];
    uint8_t buffer[64];

    public:    
    /// Used to initialize this Context.
    void sha256_starts();

    /** Used to hash the input data.
     *  \param input A pointer to an array of the input data to hash.
     *  \param length The leght of the input data to hash (in bytes).
     */
    void sha256_update(uint8_t *input, uint32_t length);

    /** Used to complete the hashing process and obtain the calculated hash.
     *  \param digest The calculated hash.
     */
    void sha256_finish(uint8_t digest[32]);

    /** Used to complete the hashing process and obtain the calculated hash.
     *  \param digest A \see Digest class where to store the result.
     */
    void sha256_finish(Digest &digest);

    private:
    /** Inner function which does the processing of the hash.
     *  \param data The data to process.
     */
    void sha256_process(uint8_t data[64]);
  };

  /// Encode a string.
  static Digest Encode(csString const&);
  /// Encode a null-terminated string buffer.
  static Digest Encode(const char*);
  /// Encode a buffer.
  static Digest Encode(const void*, size_t nbytes);
};

template<>
class csHashComputer<csSHA256::Digest> : 
  public csHashComputerStruct<csSHA256::Digest> {};

template<>
class csComparator<csSHA256::Digest> : 
  public csComparatorStruct<csSHA256::Digest> {};

#endif /* cssha256.h */
