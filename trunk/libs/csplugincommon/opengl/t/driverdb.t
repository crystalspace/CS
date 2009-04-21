/*
    Copyright (C) 2005 by Eric Sunshine

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

#include "csplugincommon/opengl/driverdb.h"

/**
 * Test iSCF operations and related SCF utilities.
 */
class GLdriverDBTest : public CppUnit::TestFixture
{
public:
  void setUp();
  void tearDown();
  
  void testVersionCompare();

  CPPUNIT_TEST_SUITE(GLdriverDBTest);
    CPPUNIT_TEST(testVersionCompare);
  CPPUNIT_TEST_SUITE_END();
};

class TestDB : public csGLDriverDatabase
{
public:
  using csGLDriverDatabase::eq;
  using csGLDriverDatabase::neq;
  using csGLDriverDatabase::le;
  using csGLDriverDatabase::lt;
  using csGLDriverDatabase::ge;
  using csGLDriverDatabase::gt;
  using csGLDriverDatabase::VersionCompare;
};

void GLdriverDBTest::setUp()
{
}

void GLdriverDBTest::tearDown()
{
}

void GLdriverDBTest::testVersionCompare()
{
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "7.1", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "7.0", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.5", "7.0", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "6.0", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.5", "6.0", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6", "6.0", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6", "6.1", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6", "7.0", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "6", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.5", "6", TestDB::le));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.5", "7", TestDB::le));
  
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "7.1", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "7.0", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.5", "7.0", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.0", "6.0", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.5", "6.0", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6", "6.0", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6", "6.1", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6", "7.0", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.0", "6", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.5", "6", TestDB::lt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.5", "7", TestDB::lt));
  
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.1", "6.0", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0", "6.0", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0", "6.5", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "6.0", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.0", "6.5", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6", "6.0", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0", "6", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "6", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.1", "6", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6", "6.5", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7", "6.5", TestDB::ge));

  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.1", "6.0", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0", "6.0", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0", "6.5", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.0", "6.0", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.0", "6.5", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6", "6.0", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0", "6", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.0", "6", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.1", "6", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6", "6.5", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7", "6.5", TestDB::gt));

  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6", "6.0", TestDB::eq));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("6.0", "6", TestDB::eq));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6", "7.0", TestDB::eq));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("6.0", "7", TestDB::eq));
  
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0.4", "6", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0.4", "6.5", TestDB::gt));
  
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0.4", "7", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0.4", "7.0", TestDB::gt));
  CPPUNIT_ASSERT_EQUAL(false, TestDB::VersionCompare ("7.0.4", "7.0.4", TestDB::gt));

  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0.4", "7", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0.4", "7.0", TestDB::ge));
  CPPUNIT_ASSERT_EQUAL(true,  TestDB::VersionCompare ("7.0.4", "7.0.4", TestDB::ge));
}
