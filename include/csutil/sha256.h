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
   * Compute a SHA-256 message digest.
   * 
   * This class provides an interface to compute a digest in a "streaming"
   * manner (the message can be split into chunks which are processed
   * sequentially) as well as a convenient interface which allows one to
   * create a digest in a single step.
   */
  class CS_CRYSTALSPACE_EXPORT SHA256
  {
    //@{
    /// Context used to generate the hash
    uint32 total[2];
    uint32 state[8];
    uint8 buffer[64];
    //@}
    
    /** 
     * Inner function which does the processing of the hash.
     * \param data The data to process.
     */
    void Process (const uint8 data[64]);
    /// Update hash with a (at most) 4GB chunk of data
    void AppendInternal (const uint8* input, uint32 length);
    /** 
     * Used to complete the hashing process and obtain the calculated hash.
     * \param digest The calculated hash.
     */
    void Finish (uint8 digest[32]);
  public:
    /// A SHA256 digest is 32 unsigned characters (not 0-terminated).
    typedef CS::Utility::Checksum::Digest<32> Digest;
    
    SHA256 ();
    
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
   
}//namespace Checksum
}//namespace Utility
}//namespace CS


#endif // _CSSHA256_H
