/*
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

#include "csutil/objreg.h"
#include "csutil/scfstr.h"

/**
 * Test iObjectRegistry operations.
 */
class csObjectRegistryTest : public CppUnit::TestFixture
{
private:
  iObjectRegistry* objreg;
  csRef<iObjectRegistryIterator> iter;
  int countRegisteredObjects();

public:
  void setUp();
  void tearDown();

  void testRegister();
  void testUnregister();

  CPPUNIT_TEST_SUITE(csObjectRegistryTest);
    CPPUNIT_TEST(testRegister);
    CPPUNIT_TEST(testUnregister);
  CPPUNIT_TEST_SUITE_END();
};

int csObjectRegistryTest::countRegisteredObjects()
{
  int count = 0;
  iter = objreg->Get();
  while (iter->HasNext())
  {
    count++;
    iter->Next();
  }
  return count;
}

void csObjectRegistryTest::setUp()
{
  objreg = new csObjectRegistry();
}

void csObjectRegistryTest::tearDown()
{
  delete objreg;
}

void csObjectRegistryTest::testRegister()
{
  // Create some test objects
  iBase* b1 = new scfString();
  iBase* b2 = new scfString();

  // Attempting to register a NULL object should fail
  CPPUNIT_ASSERT(!objreg->Register(0));
  CPPUNIT_ASSERT_EQUAL(0, countRegisteredObjects());

  // Registering the test csObjectRegistry object should succeed
  CPPUNIT_ASSERT(objreg->Register(b1, "Test Object Registry"));
  CPPUNIT_ASSERT_EQUAL(1, countRegisteredObjects());

  // Doing the same again should fail
  CPPUNIT_ASSERT(!objreg->Register(b1, "Test Object Registry"));
  CPPUNIT_ASSERT_EQUAL(1, countRegisteredObjects());

  // But doing the same again without a tag should succeed
  CPPUNIT_ASSERT(objreg->Register(b1));
  CPPUNIT_ASSERT_EQUAL(2, countRegisteredObjects());

  // Registering a different object should increase the number
  // returned by countRegisteredObjects
  CPPUNIT_ASSERT(objreg->Register(b2));
  CPPUNIT_ASSERT_EQUAL(3, countRegisteredObjects());

  // TODO: clear in one thread and attempt to register in another at
  // the same time.  This will test thread safety
}

void csObjectRegistryTest::testUnregister()
{
  // Create some test objects
  iBase* b1 = new scfString();
  iBase* b2 = new scfString();

  // Clearing the registry
  objreg->Clear();
  CPPUNIT_ASSERT_EQUAL(0, countRegisteredObjects());

  // Register some test objects
  objreg->Register(b1, "Test Object Registry One");
  objreg->Register(b1);

  objreg->Register(b2, "Test Object Registry Two");
  objreg->Register(b2);

  // There should now be four items in the registry.
  CPPUNIT_ASSERT_EQUAL(4, countRegisteredObjects());

  // Calling Unregister with a null pointer should fail
  objreg->Unregister(0);
  CPPUNIT_ASSERT_EQUAL(4, countRegisteredObjects());
}
