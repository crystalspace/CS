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

#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

/**
 * Test iSCF operations and related SCF utilities.
 */
class scfTest : public CppUnit::TestFixture
{
public:
  void setUp();
  void tearDown();

  void testScfInterface();
  void testVersion();

  CPPUNIT_TEST_SUITE(scfTest);
    CPPUNIT_TEST(testScfInterface);
    CPPUNIT_TEST(testVersion);
  CPPUNIT_TEST_SUITE_END();
};

struct iBaseTest  : public virtual iBase
{
  SCF_INTERFACE(iBaseTest,  1, 2, 3);
};
struct iEmbedTest : public virtual iBase
{
  SCF_INTERFACE(iEmbedTest, 4, 5, 6);
};
class  csBaseTest : public scfImplementation2<csBaseTest, iBaseTest, iEmbedTest>
{
  csBaseTest(iBase* p) : scfImplementationType (this, p)
  {
  }
};

void scfTest::setUp()
{
  if (iSCF::SCF == 0)
    scfInitialize(0);
}

void scfTest::tearDown()
{
}

void scfTest::testScfInterface()
{
  int const v = scfInterfaceTraits<iBaseTest>::GetVersion();
  scfInterfaceID const i = scfInterfaceTraits<iBaseTest>::GetID();
  char const* n = scfInterfaceTraits<iBaseTest>::GetName();
  char const* q = iSCF::SCF->GetInterfaceName(i);
  CPPUNIT_ASSERT_EQUAL(SCF_CONSTRUCT_VERSION(1,2,3), v);
  CPPUNIT_ASSERT_EQUAL(i, iSCF::SCF->GetInterfaceID("iBaseTest"));
  CPPUNIT_ASSERT_EQUAL(std::string("iBaseTest"), std::string(n));
  CPPUNIT_ASSERT(q != 0);
  CPPUNIT_ASSERT_EQUAL(std::string(n), std::string(q));
}

void scfTest::testVersion()
{
#define V(A,B,C) SCF_CONSTRUCT_VERSION(A,B,C)
  int const v = scfInterfaceTraits<iBaseTest>::GetVersion();
  CPPUNIT_ASSERT_EQUAL(V(1,2,3), v);
  CPPUNIT_ASSERT( scfCompatibleVersion(V(1,2,3), v));
  CPPUNIT_ASSERT( scfCompatibleVersion(V(1,2,0), v));
  CPPUNIT_ASSERT( scfCompatibleVersion(V(1,0,0), v));
  CPPUNIT_ASSERT(!scfCompatibleVersion(V(2,0,0), v));
  CPPUNIT_ASSERT(!scfCompatibleVersion(V(1,3,3), v));
  CPPUNIT_ASSERT(!scfCompatibleVersion(V(1,2,4), v));
#undef V
}
