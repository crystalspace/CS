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

#include "csutil/verbosity.h"

/**
 * Test verbosity-related utilities.
 */
class csVerbosityTest : public CppUnit::TestFixture
{
public:
  void testSilent();
  void testEnable();
  void testDisable();
  void testInherit();
  void testNoInherit();
  void testOverride();
  void testClone();
  void testExtend();

  CPPUNIT_TEST_SUITE(csVerbosityTest);
    CPPUNIT_TEST(testSilent);
    CPPUNIT_TEST(testEnable);
    CPPUNIT_TEST(testDisable);
    CPPUNIT_TEST(testInherit);
    CPPUNIT_TEST(testNoInherit);
    CPPUNIT_TEST(testOverride);
    CPPUNIT_TEST(testClone);
    CPPUNIT_TEST(testExtend);
  CPPUNIT_TEST_SUITE_END();
};

void csVerbosityTest::testSilent()
{
  csVerbosityParser v;
  CPPUNIT_ASSERT(!v.Enabled());
  CPPUNIT_ASSERT(!v.Enabled("foo"));
  CPPUNIT_ASSERT(!v.Enabled("foo.bar"));
}

void csVerbosityTest::testEnable()
{
  csVerbosityParser v("foo,+bar,+baz.cow");
  CPPUNIT_ASSERT( v.Enabled("foo"));
  CPPUNIT_ASSERT( v.Enabled("bar"));
  CPPUNIT_ASSERT( v.Enabled("baz.cow"));
  CPPUNIT_ASSERT(!v.Enabled("baz"));
  CPPUNIT_ASSERT(!v.Enabled("gob"));
}

void csVerbosityTest::testDisable()
{
  csVerbosityParser v("-foo,-bar,-baz.cow");
  CPPUNIT_ASSERT(!v.Enabled("foo"));
  CPPUNIT_ASSERT(!v.Enabled("bar"));
  CPPUNIT_ASSERT(!v.Enabled("baz.cow"));
  CPPUNIT_ASSERT( v.Enabled("baz"));
  CPPUNIT_ASSERT( v.Enabled("gob"));
}

void csVerbosityTest::testInherit()
{
  csVerbosityParser v("+foo,+bar.baz");
  CPPUNIT_ASSERT( v.Enabled("foo"));
  CPPUNIT_ASSERT( v.Enabled("foo.cow"));
  CPPUNIT_ASSERT( v.Enabled("foo.cow.gob"));
  CPPUNIT_ASSERT(!v.Enabled("bar"));
  CPPUNIT_ASSERT( v.Enabled("bar.baz"));
  CPPUNIT_ASSERT( v.Enabled("bar.baz.bop"));
  CPPUNIT_ASSERT(!v.Enabled("bar.bip"));
}

void csVerbosityTest::testNoInherit()
{
  csVerbosityParser v("+foo,+foo.bar");
  CPPUNIT_ASSERT( v.Enabled("foo", true));
  CPPUNIT_ASSERT( v.Enabled("foo.bar", true));
  CPPUNIT_ASSERT( v.Enabled("foo.bar", false));
  CPPUNIT_ASSERT( v.Enabled("foo.bar.cow", true));
  CPPUNIT_ASSERT(!v.Enabled("foo.bar.cow", false));
  CPPUNIT_ASSERT( v.Enabled("foo.baz", true));
  CPPUNIT_ASSERT(!v.Enabled("foo.baz", false));
  CPPUNIT_ASSERT( v.Enabled("foo.baz.cow", true));
  CPPUNIT_ASSERT(!v.Enabled("foo.baz.cow", false));
}

void csVerbosityTest::testOverride()
{
  csVerbosityParser v("-foo,+foo.bar,-foo.bar.baz");
  CPPUNIT_ASSERT(!v.Enabled("foo"));
  CPPUNIT_ASSERT(!v.Enabled("foo.bop"));
  CPPUNIT_ASSERT( v.Enabled("foo.bar"));
  CPPUNIT_ASSERT( v.Enabled("foo.bar.bip"));
  CPPUNIT_ASSERT(!v.Enabled("foo.bar.baz"));
  CPPUNIT_ASSERT(!v.Enabled("foo.bar.baz.cow"));
}

void csVerbosityTest::testClone()
{
  csVerbosityParser v0("+foo,-bar,+bar.baz");
  csVerbosityParser v(v0);
  CPPUNIT_ASSERT( v.Enabled("foo"));
  CPPUNIT_ASSERT( v.Enabled("foo.baz"));
  CPPUNIT_ASSERT(!v.Enabled("bar"));
  CPPUNIT_ASSERT( v.Enabled("bar.baz"));
  CPPUNIT_ASSERT(!v.Enabled("bop"));
  CPPUNIT_ASSERT(!v.Enabled("bop.baz"));
}

void csVerbosityTest::testExtend()
{
  csVerbosityParser v;
  CPPUNIT_ASSERT(!v.Enabled());
  CPPUNIT_ASSERT(!v.Enabled("foo"));
  CPPUNIT_ASSERT(!v.Enabled("bar"));
  v.Parse("-foo");
  CPPUNIT_ASSERT( v.Enabled());
  CPPUNIT_ASSERT(!v.Enabled("foo"));
  CPPUNIT_ASSERT( v.Enabled("bar"));
  v.Parse("+foo.baz,-bar");
  CPPUNIT_ASSERT(!v.Enabled("foo"));
  CPPUNIT_ASSERT( v.Enabled("foo.baz"));
  CPPUNIT_ASSERT(!v.Enabled("bar"));
}
