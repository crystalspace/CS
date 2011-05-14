/*
    Copyright (C) 2011 by Frank Richter

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

#include "csutil/md5.h"

#define ARRAY_SIZE(x)		(sizeof((x))/sizeof((x)[0]))

/**
 * Test csMD5.
 */
class MD5Test : public CppUnit::TestFixture
{
public:
  void testMD5 ();
  
  CPPUNIT_TEST_SUITE(MD5Test);
    CPPUNIT_TEST(testMD5);
  CPPUNIT_TEST_SUITE_END();
};

// MD5 test suite, from RFC 1321
static const struct
{
  const char* data;
  const uint8 expected[CS::Utility::Checksum::MD5::Digest::DigestLen];
} testValuesMD5[] =
{
  { "",
    { 0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04, 0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e } },
  { "a",
    { 0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8, 0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61 } },
  { "abc",
    { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 } },
  { "message digest",
    { 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0 } },
  { "abcdefghijklmnopqrstuvwxyz",
    { 0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00, 0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b } },
  { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    { 0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5, 0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f } },
  { "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
    { 0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55, 0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a } }
};

void MD5Test::testMD5()
{  
  for (size_t i = 0; i < ARRAY_SIZE(testValuesMD5); i++)
  {
    const char* data = testValuesMD5[i].data;
    csString digestStr (CS::Utility::Checksum::MD5::Encode (data).HexString());
    CS::Utility::Checksum::MD5::Digest expectedDigest;
    memcpy (&expectedDigest, testValuesMD5[i].expected, sizeof (CS::Utility::Checksum::MD5::Digest));
    csString expectedStr (expectedDigest.HexString());
    CPPUNIT_ASSERT_EQUAL(expectedStr, digestStr);
  }
}
