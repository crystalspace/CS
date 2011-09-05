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

#include <iutil/comp.h>
#include <iutil/plugin.h>

#include <csutil/plugmgr.h>

/**
 * Plugin manager tests.
 */
class PluginManagerTest : public CppUnit::TestFixture
{
private:
  iObjectRegistry* objreg;
  csPluginManager* plugmgr;
public:
  void setUp();
  void tearDown();

  // Load single instance of a plugin
  void testLoadSingle ();
  // Load multiple instances of a plugin
  void testLoadMultiple ();
  // Query same instance of a plugin
  void testLoadSame ();
  // Query same instance of a plugin
  void testLoadQuery ();
  // Query instance of a plugin that has been unloaded
  void testLoadUnloadQuery ();

  CPPUNIT_TEST_SUITE(PluginManagerTest);
    CPPUNIT_TEST(testLoadSingle);
    CPPUNIT_TEST(testLoadMultiple);
    CPPUNIT_TEST(testLoadSame);
    CPPUNIT_TEST(testLoadQuery);
    CPPUNIT_TEST(testLoadUnloadQuery);
  CPPUNIT_TEST_SUITE_END();
};

void PluginManagerTest::setUp()
{
  // Ensure that we have an initialized iSCF::SCF object and plugin scanning was done
  const char* const fake_argv[] = { "", 0 };
  scfInitialize (0, fake_argv, true);
  // Set up minimal environment for plugin manager
  objreg = new csObjectRegistry();
  plugmgr = new csPluginManager (objreg);
}

void PluginManagerTest::tearDown()
{
  CPPUNIT_ASSERT_EQUAL(plugmgr->GetRefCount(), 1);
  plugmgr->DecRef ();
  CPPUNIT_ASSERT_EQUAL(objreg->GetRefCount(), 1);
  objreg->Clear();
  objreg->DecRef();
}

/* Plugin to use for tests.
 * xmlread has no dependencies (beyond what basic CS needs) and can
 * be loaded multiple times without issues. */
#define TEST_PLUGIN_ID          "crystalspace.documentsystem.xmlread"

void PluginManagerTest::testLoadSingle ()
{
  csRef<iComponent> plugin (csLoadPlugin<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(plugin);
  CPPUNIT_ASSERT_EQUAL(1, plugin->GetRefCount());
}

void PluginManagerTest::testLoadMultiple ()
{
  csRef<iComponent> plugin (csLoadPlugin<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(plugin);
  CPPUNIT_ASSERT_EQUAL(1, plugin->GetRefCount());

  csRef<iComponent> plugin2 (csLoadPlugin<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(plugin2);
  CPPUNIT_ASSERT_EQUAL(1, plugin2->GetRefCount());
}

void PluginManagerTest::testLoadSame ()
{
  csRef<iComponent> plugin (csLoadPluginCheck<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(plugin);
  CPPUNIT_ASSERT_EQUAL(1, plugin->GetRefCount());

  csRef<iComponent> plugin2 (csLoadPluginCheck<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT_EQUAL(plugin, plugin2);
  CPPUNIT_ASSERT_EQUAL(2, plugin2->GetRefCount());
}

void PluginManagerTest::testLoadQuery ()
{
  csRef<iComponent> plugin (csLoadPluginCheck<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(plugin);
  CPPUNIT_ASSERT_EQUAL(1, plugin->GetRefCount());

  csRef<iComponent> plugin2 (csQueryPluginClass<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT_EQUAL(plugin, plugin2);
  CPPUNIT_ASSERT_EQUAL(2, plugin2->GetRefCount());
}

void PluginManagerTest::testLoadUnloadQuery ()
{
  csRef<iComponent> plugin (csLoadPluginCheck<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(plugin);
  CPPUNIT_ASSERT_EQUAL(1, plugin->GetRefCount());
  plugin.Invalidate(); // This should unload the plugin

  // No plugin of that ID should be available
  csRef<iComponent> plugin2 (csQueryPluginClass<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(!plugin2);

  // Should give a fresh instance
  csRef<iComponent> plugin3 (csLoadPluginCheck<iComponent> (plugmgr, TEST_PLUGIN_ID));
  CPPUNIT_ASSERT(plugin3);
  CPPUNIT_ASSERT_EQUAL(1, plugin3->GetRefCount());
}

