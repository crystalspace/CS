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

#include "csutil/checksum.h"
#include "csutil/databuf.h"

#define ARRAY_SIZE(x)		(sizeof((x))/sizeof((x)[0]))

/**
 * Test CS::Utility::Checksum::Adler32.
 */
class Adler32Test : public CppUnit::TestFixture
{
public:
  void testOnePass ();
  void testTwoPasses ();
  void testManyPasses ();
  
  void testOnePassDatabuffer ();
  
  CPPUNIT_TEST_SUITE(Adler32Test);
    CPPUNIT_TEST(testOnePass);
    CPPUNIT_TEST(testTwoPasses);
    CPPUNIT_TEST(testManyPasses);
 
    CPPUNIT_TEST(testOnePassDatabuffer);
  CPPUNIT_TEST_SUITE_END();
};

static const struct
{
  const char* data;
  uint32 expected;
} testValuesAdler32[] =
{
  { "", 0x00000001 },
  { "CrystalSpace", 0x1F5704CF },
  { "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0xF05D0AEC }
};

void Adler32Test::testOnePass()
{  
  for (size_t i = 0; i < ARRAY_SIZE(testValuesAdler32); i++)
  {
    const char* data = testValuesAdler32[i].data;
    uint32 checksum = CS::Utility::Checksum::Adler32 (data, strlen (data));
    CPPUNIT_ASSERT_EQUAL(testValuesAdler32[i].expected, checksum);
  }
}

void Adler32Test::testTwoPasses()
{  
  for (size_t i = 0; i < ARRAY_SIZE(testValuesAdler32); i++)
  {
    const char* data = testValuesAdler32[i].data;
    size_t dataSize = strlen (data);
    uint32 checksum = CS::Utility::Checksum::Adler32 (data, dataSize / 2);
    checksum = CS::Utility::Checksum::Adler32 (checksum, data + (dataSize / 2), dataSize - (dataSize / 2));
    CPPUNIT_ASSERT_EQUAL(testValuesAdler32[i].expected, checksum);
  }
}

void Adler32Test::testManyPasses()
{  
  for (size_t i = 0; i < ARRAY_SIZE(testValuesAdler32); i++)
  {
    const char* data = testValuesAdler32[i].data;
    size_t dataSize = strlen (data);
    uint32 checksum = CS::Utility::Checksum::Adler32 (nullptr, 0);
    for (size_t c = 0; c < dataSize; c++)
    {
      checksum = CS::Utility::Checksum::Adler32 (checksum, data + c, 1);
    }
    CPPUNIT_ASSERT_EQUAL(testValuesAdler32[i].expected, checksum);
  }
}

void Adler32Test::testOnePassDatabuffer()
{  
  for (size_t i = 0; i < ARRAY_SIZE(testValuesAdler32); i++)
  {
    csRef<iDataBuffer> buf;
    buf.AttachNew (new CS::DataBuffer<> ((char*)testValuesAdler32[i].data,
				         strlen (testValuesAdler32[i].data),
				         false));
    uint32 checksum = CS::Utility::Checksum::Adler32 (buf);
    CPPUNIT_ASSERT_EQUAL(testValuesAdler32[i].expected, checksum);
  }
}
