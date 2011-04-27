/*
    Copyright (C) 2005 by Adam D. Bradley

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

#include "cppunit/Message.h"

#include "csutil/objreg.h"
#include "csutil/scfstr.h"
#include "csutil/inputdef.h"
#include "iutil/eventnames.h"
#include "csutil/eventnames.h"

/**
 * Test csInputDefinition parser.
 */
class csInputDefinitionTest : public CppUnit::TestFixture
{
private:
  iObjectRegistry* objreg;
  iEventNameRegistry* namereg;

public:
  void setUp();
  void testKeys();
  void testMouse();
  void testJoystick();

  CPPUNIT_TEST_SUITE(csInputDefinitionTest);
    CPPUNIT_TEST(testKeys);
    CPPUNIT_TEST(testMouse);
    CPPUNIT_TEST(testJoystick);
  CPPUNIT_TEST_SUITE_END();
};

void csInputDefinitionTest::setUp()
{
  if (iSCF::SCF == 0)
    scfInitialize(0);
  objreg = new csObjectRegistry();
  namereg = csEventNameRegistry::GetRegistry (objreg);
}

void csInputDefinitionTest::testKeys()
{
  const char *keys[] = { "0", "1", "esc", 0 };
  for (int i=0 ; keys[i]!=0 ; i++) {
    csInputDefinition key(namereg,keys[i],0,false);
    CPPUNIT_ASSERT(key.GetDeviceNumber() == 0);
    CPPUNIT_ASSERT(key.GetName() != CS_EVENT_INVALID);
    CPPUNIT_ASSERT(key.GetName() == csevKeyboardEvent(namereg));
  }
}

void csInputDefinitionTest::testMouse()
{
  const char *moves[3][11] = {
	{ "mouseX", "mouseY", "mouseAxis0", "mouseAxis1", "mouseAxis2", 
	  "0mouseX", "0mouseY", "0mouseAxis0", "0mouseAxis1", "0mouseAxis2", 0 },
	{ "1mouseX", "1mouseY", "1mouseAxis0", "1mouseAxis1", "1mouseAxis2", 0 },
	{ "2mouseX", "2mouseY", "2mouseAxis0", "2mouseAxis1", "2mouseAxis2", 0 }
  };

  for (uint dev=0 ; dev<3 ; dev++) {
    for (uint i=0 ; moves[dev][i]!=0 ; i++) {
      csInputDefinition mv(namereg,moves[dev][i],0,false);
      CPPUNIT_ASSERT(mv.GetName() != CS_EVENT_INVALID);
      CPPUNIT_ASSERT(mv.GetName() == csevMouseMove(namereg, dev));
    }
  }

  const char *buttons[3][13] = {
    { "mouseButton0", "mouseButton1", "mouseButton2",
      "0mouse0", "0mouse1", "0mouse2",
      "0mouseButton0", "0mouseButton1", "0mouseButton2", 0 },
    { "1mouse0", "1mouse1", "1mouse2",
      "1mouseButton0", "1mouseButton1", "1mouseButton2", 0 },
    { "2mouse0", "2mouse1", "2mouse2",
      "2mouseButton0", "2mouseButton1", "2mouseButton2", 0 }
  };

  for (uint dev=0 ; dev<3 ; dev++) {
    for (uint i=0 ; buttons[dev][i]!=0 ; i++) {
      csInputDefinition bt(namereg,buttons[dev][i],0,false);
      CPPUNIT_ASSERT(bt.GetDeviceNumber() == dev);
      CPPUNIT_ASSERT(bt.GetName() != CS_EVENT_INVALID);
      CPPUNIT_ASSERT(bt.GetName() == csevMouseButton(namereg, dev));
    }
  }
}

void csInputDefinitionTest::testJoystick()
{
  const char *moves[3][11] = {
	{ "joystickX", "joystickY", "joystickAxis0", "joystickAxis1", "joystickAxis2", 
	  "0joystickX", "0joystickY", "0joystickAxis0", "0joystickAxis1", "0joystickAxis2", 0 },
	{ "1joystickX", "1joystickY", "1joystickAxis0", "1joystickAxis1", "1joystickAxis2", 0 },
	{ "2joystickX", "2joystickY", "2joystickAxis0", "2joystickAxis1", "2joystickAxis2", 0 }
  };

  for (uint dev=0 ; dev<3 ; dev++) {
    for (uint i=0 ; moves[dev][i]!=0 ; i++) {
      csInputDefinition mv(namereg,moves[dev][i],0,false);
      CPPUNIT_ASSERT(mv.GetName() != CS_EVENT_INVALID);
      CPPUNIT_ASSERT(mv.GetName() == csevJoystickMove(namereg, dev));
    }
  }

  const char *buttons[3][13] = {
    { "joystickButton0", "joystickButton1", "joystickButton2",
      "0joystick0", "0joystick1", "0joystick2",
      "0joystickButton0", "0joystickButton1", "0joystickButton2", 0 },
    { "1joystick0", "1joystick1", "1joystick2",
      "1joystickButton0", "1joystickButton1", "1joystickButton2", 0 },
    { "2joystick0", "2joystick1", "2joystick2",
      "2joystickButton0", "2joystickButton1", "2joystickButton2", 0 }
  };

  for (uint dev=0 ; dev<3 ; dev++) {
    for (uint i=0 ; buttons[dev][i]!=0 ; i++) {
      csInputDefinition bt(namereg,buttons[dev][i],0,false);
      CPPUNIT_ASSERT(bt.GetDeviceNumber() == dev);
      CPPUNIT_ASSERT(bt.GetName() != CS_EVENT_INVALID);
      CPPUNIT_ASSERT(bt.GetName() == csevJoystickButton(namereg, dev));
    }
  }
}

