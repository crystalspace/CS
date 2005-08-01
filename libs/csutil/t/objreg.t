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

  // Counts the number of registered objects in a registry.
  int countRegisteredObjects();
  int countRegisteredObjects(scfInterfaceID id, int version);

  // Counts number of objects an iterator iterates over.
  int countIterations(csRef<iObjectRegistryIterator> iter);

public:
  /**
   * Overridden from CppUnit::TextFixture.  Contains operations which
   * should be run before tests.
   */
  void setUp();

  /**
   * Overridden from CppUnit::TextFixture.  Contains operations which
   * should be run after tests complete.
   */
  void tearDown();

  /// Contains tests run against csObjectRegistry::Register().
  void testRegister();
  /// Contains tests run against csObjectRegistry::Unregister().
  void testUnregister();
  /// Contains tests run against csObjectRegistry::Get() in all forms.
  void testGet();

  CPPUNIT_TEST_SUITE(csObjectRegistryTest);
    CPPUNIT_TEST(testRegister);
    CPPUNIT_TEST(testUnregister);
    CPPUNIT_TEST(testGet);
  CPPUNIT_TEST_SUITE_END();
};

int csObjectRegistryTest::countIterations(csRef<iObjectRegistryIterator> iter)
{
  int count = 0;
  for ( ; iter->HasNext(); iter->Next())
    count++;
  return count;
}

int csObjectRegistryTest::countRegisteredObjects()
{
  return countIterations(objreg->Get());
}

int csObjectRegistryTest::countRegisteredObjects(scfInterfaceID id, int ver)
{
  return countIterations(objreg->Get(id, ver));
}

void csObjectRegistryTest::setUp()
{
  // Ensure that we have an initialized iSCF::SCF object
  if (iSCF::SCF == 0)
    scfInitialize(0);
  objreg = new csObjectRegistry();
}

void csObjectRegistryTest::tearDown()
{
  CPPUNIT_ASSERT_EQUAL(objreg->GetRefCount(), 1);
  objreg->Clear();
  objreg->DecRef();
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

  // Clean up
  objreg->Clear();

  b2->DecRef();
  b1->DecRef();
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

  b2->DecRef();
  b1->DecRef();
}

void csObjectRegistryTest::testGet()
{
  // Create some test objects
  scfString *v1 = new scfString();
  scfString *v2 = new scfString();

  // Clear the registry
  objreg->Clear();
  CPPUNIT_ASSERT_EQUAL(0, countRegisteredObjects());

  // Register some test objects
  objreg->Register(v1, "Test Object Registry one");
  objreg->Register(v1);
  
  objreg->Register(v2, "Test Object Registry two");
  objreg->Register(v2);

  csRef<iString> if1 = SCF_QUERY_INTERFACE(v1, iString);
  csRef<iString> if2 = SCF_QUERY_INTERFACE(v2, iString);

  CPPUNIT_ASSERT(if1.IsValid());
  CPPUNIT_ASSERT(if2.IsValid());

  csRef<iString> cmp1, cmp2;
  cmp1.AttachNew(CS_STATIC_CAST(iString*,objreg->Get(
    "Test Object Registry one",
    scfInterfaceTraits<iString>::GetID(),
    scfInterfaceTraits<iString>::GetVersion())));
  cmp2.AttachNew(CS_STATIC_CAST(iString*,objreg->Get(
    "Test Object Registry two",
    scfInterfaceTraits<iString>::GetID(),
    scfInterfaceTraits<iString>::GetVersion())));

  CPPUNIT_ASSERT_EQUAL(if1, cmp1);
  CPPUNIT_ASSERT_EQUAL(if2, cmp2);

  // This should return v1's and v2's registered objects for a total of two
  CPPUNIT_ASSERT_EQUAL(4, countRegisteredObjects(
    scfInterfaceTraits<iString>::GetID(),
    scfInterfaceTraits<iString>::GetVersion()));

  // Clean up
  objreg->Clear();

  v2->DecRef();
  v1->DecRef();
}
