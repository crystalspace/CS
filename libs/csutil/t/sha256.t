/*
    Copyright (C) 2011 by Stefano Angeleri
    Raw api test original code written by Christophe Devine (see below)

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
 *  FIPS-180-2 compliant SHA-256 implementation
 *  written by Christophe Devine
 *
 *  This code has been distributed as PUBLIC DOMAIN.
 *
 *  Although normally licensed under the GPL on the author's web site,
 *  he has given me permission to distribute it as public domain as 
 *  part of md5deep. THANK YOU! Software authors are encouraged to
 *  use the GPL'ed version of this code available at:
 *  http://www.cr0.net:8040/code/crypto/sha256/ whenever possible.
 */

#include "csutil/sha256.h"

using namespace CS::Utility::Checksum;

/*
 * those are the standard FIPS-180-2 test vectors
 */

static const char *msg[] = 
  {
    "abc",
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    NULL
  };

static const char *val[] =
  {
    "ba7816bf8f01cfea414140de5dae2223" \
    "b00361a396177a9cb410ff61f20015ad",
    "248d6a61d20638b8e5c026930c3e6039" \
    "a33ce45964ff2167f6ecedd419db06c1",
    "cdc76e5c9914fb9281a1c7e284d73e67" \
    "f1809a48a497200e046d39ccc7112cd0"
  };


/**
 * Test sha256 functionality.
 */
class csSHA256Test : public CppUnit::TestFixture
{
public:
  void setUp();

  void testSHA256encode();

  CPPUNIT_TEST_SUITE(csSHA256Test);
    CPPUNIT_TEST(testSHA256encode);
  CPPUNIT_TEST_SUITE_END();
};

void csSHA256Test::setUp()
{
}

void csSHA256Test::testSHA256encode()
{
  //raw api testing
  for(int i = 0; i < 3; i++)
  {
    char output[65];
    SHA256 ctx;
    unsigned char buf[1000];

    if(i < 2)
    {
      ctx.Append ((uint8*) msg[i], strlen(msg[i]));
    }
    else
    {
      memset(buf, 'a', 1000);
      for(int j = 0; j < 1000; j++)
      {
        ctx.Append((uint8*) buf, 1000);
      }
    }

    SHA256::Digest sha256sum = ctx.Finish ();

    for(int  j = 0; j < 32; j++)
    {
      sprintf(output + j * 2, "%02x", sha256sum.data[j]);
    }

    CPPUNIT_ASSERT(!memcmp(output, val[i], 64));
  }

  //cs simplified api testing
  for(int i = 0; i < 2; i++)
  {
      CPPUNIT_ASSERT_EQUAL(SHA256::Encode(msg[i], strlen(msg[i])).HexString(), csString(val[i]));
  }
}
