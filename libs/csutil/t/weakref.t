/*
    Copyright (C) 2011 by Marten Svanfeldt

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTAManagerILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakrefarr.h"

class weakRefTest : public CppUnit::TestFixture
{
public:
  void setUp();
  void tearDown();

  void testCyclicTeardown();

  CPPUNIT_TEST_SUITE(weakRefTest);
    CPPUNIT_TEST(testCyclicTeardown);
  CPPUNIT_TEST_SUITE_END();
};

void weakRefTest::setUp()
{
  if (iSCF::SCF == 0)
    scfInitialize (0);
}

void weakRefTest::tearDown()
{
}

class Manager;

class Client : public scfImplementation0<Client>
{
public:
  Client (Manager *b);
  ~Client ();

private:
  Manager *b;
};

class Manager
{
public:
  void RegisterClient (Client *a);
  void UnregisterClient (Client *a);

private:
  csWeakRefArray<Client> clientArray;
};

Client::Client (Manager *b)
  : scfImplementationType (this), b(b)
{
}

Client::~Client ()
{
  if (b) {
    b->UnregisterClient (this);
  }
}

void Manager::RegisterClient (Client *a)
{
  clientArray.Push (a);
}

void Manager::UnregisterClient (Client *a)
{
  size_t index = clientArray.Find (a);
  if (index != csArrayItemNotFound) 
    clientArray.DeleteIndexFast (index);
}

void weakRefTest::testCyclicTeardown()
{
  Manager *ourManager;
  Client *ourClient;

  ourManager = new Manager;
  ourClient = new Client (ourManager);

  ourManager->RegisterClient (ourClient);
  ourClient->DecRef ();

  // Clean up
  delete ourManager;

  CPPUNIT_ASSERT(1 == 1);
}

