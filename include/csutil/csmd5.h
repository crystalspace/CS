/*
    Crystal Space utility library: MD5 class
    Original C code written by L. Peter Deutsch (see below)
    Adapted for Crystal Space by Michael Dale Long

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
/*$Id$ */
/*
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

#ifndef __MD5_H__
#define __MD5_H__

#include "csutil/csbase.h"

/* This is the CS MD5 Hash generator class.  It is based on code
 * C code from sources noted above.
 */
class csMD5 : public csBase
{
public:
  // Passing in a buffer is the same as calling SetBuffer(buffer) after new
  csMD5(char *buffer = NULL);
  //virtual ~csMD5();

  // Give csMD5 the buffer to encode
  void SetBuffer(char *buffer);
  // Retrieve the buffer csMD5 is set to encode (or NULL)
  char *GetBuffer();
  // Return the resulting code
  unsigned char *GetHash();
  // Return the resulting code reduced down to an integer
  unsigned int GetReducedHash();

protected:

  typedef unsigned char md5_byte_t; /* 8-bit byte */
  typedef unsigned int md5_word_t; /* 32-bit word */

  /* Define the state of the MD5 Algorithm. */
  typedef struct md5_state_s {
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */
  } md5_state_t;

  void md5_init(md5_state_t *pms);
  void md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes);
  void md5_finish(md5_state_t *pms, md5_byte_t digest[16]);
  void md5_process(md5_state_t *pms, const md5_byte_t *data /*[64]*/);

  md5_byte_t digest[16];

};

#endif // ! __CSMD5_H__
