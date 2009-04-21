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

#include "iutil/event.h"
#include "csutil/objreg.h"
#include "csutil/scfstr.h"
#include "csutil/cssubscription.h"
#include "csutil/eventnames.h"
#include "csutil/cseventq.h"

static csList<csString *> *handlers;

/**
 * Test csEventQueue and csEventTree.
 */
class csEventQueueTest : public CppUnit::TestFixture
{
private:
  iObjectRegistry* objreg;
  iEventNameRegistry *name_reg;
  iEventHandlerRegistry *handler_reg;

  class HandlerLogic : public scfImplementation1<HandlerLogic, iEventHandler>
  {
   public:
    HandlerLogic (iObjectRegistry *) : scfImplementationType (this) { };
    virtual ~HandlerLogic () { };
    virtual bool HandleEvent (iEvent&) { handlers->PushBack(new csString("Logic")); return false; };
    CS_EVENTHANDLER_PHASE_LOGIC("unit.test.logic");
  };
  class Handler3D : public scfImplementation1<Handler3D, iEventHandler>
  {
   public:
    Handler3D (iObjectRegistry *) : scfImplementationType (this) { };
    virtual ~Handler3D () { };
    virtual bool HandleEvent (iEvent&) { handlers->PushBack(new csString("3D")); return false; };
    CS_EVENTHANDLER_PHASE_3D("unit.test.3d");
  };
  class Handler2D : public scfImplementation1<Handler2D, iEventHandler>
  {
   public:
    Handler2D (iObjectRegistry *) : scfImplementationType (this) { };
    virtual ~Handler2D () { };
    virtual bool HandleEvent (iEvent&) { handlers->PushBack(new csString("2D")); return false; };
    CS_EVENTHANDLER_PHASE_2D("unit.test.2d");
  };
  class HandlerConsole : public scfImplementation1<HandlerConsole, iEventHandler>
  {
   public:
    HandlerConsole (iObjectRegistry *) : scfImplementationType (this) { };
    virtual ~HandlerConsole () { };
    virtual bool HandleEvent (iEvent&) { handlers->PushBack(new csString("Console")); return false; };
    CS_EVENTHANDLER_PHASE_CONSOLE("unit.test.console");
  };
  class HandlerDebug : public scfImplementation1<HandlerDebug, iEventHandler>
  {
   public:
    HandlerDebug (iObjectRegistry *) : scfImplementationType (this) { };
    virtual ~HandlerDebug () { };
    virtual bool HandleEvent (iEvent&) { handlers->PushBack(new csString("Debug")); return false; };
    CS_EVENTHANDLER_PHASE_DEBUG("unit.test.debug");
  };
  class HandlerFrame : public scfImplementation1<HandlerFrame, iEventHandler>
  {
   public:
    HandlerFrame (iObjectRegistry *) : scfImplementationType (this) { };
    virtual ~HandlerFrame () { };
    virtual bool HandleEvent (iEvent&) { handlers->PushBack(new csString("Frame")); return false; };
    CS_EVENTHANDLER_PHASE_FRAME("unit.test.frame");
  };
  class HandlerOther : public scfImplementation1<HandlerOther, iEventHandler>
  {
   public:
    csString title;
    HandlerOther (iObjectRegistry *, const char *t) : scfImplementationType (this), title(t) { };
    virtual ~HandlerOther () { };
    virtual bool HandleEvent (iEvent&) { handlers->PushBack(new csString(title)); return false; };
    CS_EVENTHANDLER_PHASE_FRAME("unit.test.other");
  };


public:
  /**
   * Overridden from CppUnit::TextFixture.  Contains operations which
   * should be run before tests.
   */
  void setUp ();

  /**
   * Overridden from CppUnit::TextFixture.  Contains operations which
   * should be run after tests complete.
   */
  void tearDown ();

  void testSmokeTest ();
  void testPhaseHandlers ();
  void testFrameSubEvents ();
  void testMixedHandlers ();

  CPPUNIT_TEST_SUITE (csEventQueueTest);
    CPPUNIT_TEST (testSmokeTest);
    CPPUNIT_TEST (testPhaseHandlers);
    CPPUNIT_TEST (testMixedHandlers);
  CPPUNIT_TEST_SUITE_END ();
};

void csEventQueueTest::setUp ()
{
  // Ensure that we have an initialized iSCF::SCF object
  if (iSCF::SCF == 0)
    scfInitialize(0);
  objreg = new csObjectRegistry();
  name_reg = csEventNameRegistry::GetRegistry (objreg);
  handler_reg = csEventHandlerRegistry::GetRegistry (objreg);
}

void csEventQueueTest::tearDown ()
{
  CPPUNIT_ASSERT_EQUAL(objreg->GetRefCount(), 1);
  objreg->Clear();
  objreg->DecRef();
}

/**
 * Make sure the code doesn't self-destruct.
 */
void csEventQueueTest::testSmokeTest ()
{
  csEventQueue *queue = new csEventQueue (objreg);
  handlers = new csList<csString *> ();

  iEventHandler *h1 = new HandlerLogic (objreg);
  queue->RegisterListener (h1, csevFrame (objreg));

  queue->Process ();

  CPPUNIT_ASSERT (!handlers->IsEmpty());
  CPPUNIT_ASSERT (handlers->Front ()->Compare ("Logic"));
  handlers->PopFront ();
  CPPUNIT_ASSERT (handlers->IsEmpty());
}

/**
 * Make sure phase-controlled handlers are called in order.
 */
void csEventQueueTest::testPhaseHandlers ()
{
  csEventQueue *queue = new csEventQueue (objreg);
  handlers = new csList<csString *> ();

  iEventHandler *h1 = new HandlerLogic (objreg);
  iEventHandler *h2 = new Handler3D (objreg);
  iEventHandler *h3 = new Handler2D (objreg);
  iEventHandler *h4 = new HandlerConsole (objreg);
  iEventHandler *h5 = new HandlerDebug (objreg);
  iEventHandler *h6 = new HandlerFrame (objreg);

  queue->RegisterListener (h6, csevFrame (objreg));
  queue->RegisterListener (h1, csevFrame (objreg));
  queue->RegisterListener (h5, csevFrame (objreg));
  queue->RegisterListener (h2, csevFrame (objreg));
  queue->RegisterListener (h4, csevFrame (objreg));
  queue->RegisterListener (h3, csevFrame (objreg));

  handlers = new csList<csString *> ();
  queue->Process ();

#ifdef ADB_DEBUG
  csEventTree *frameEvent = queue->EventTree->FindNode(csevFrame (objreg), queue);
  frameEvent->fatRecord->SubscriberGraph->Dump (objreg);
#endif

  CPPUNIT_ASSERT_MESSAGE ("List is empty", !handlers->IsEmpty());

  std::string message ("Expected Logic, got ");
  message += handlers->Front()->GetData();
  CPPUNIT_ASSERT_MESSAGE (message,
	handlers->Front ()->Compare ("Logic"));
  handlers->PopFront ();

  message = "Expected 3D, got ";
  message += handlers->Front()->GetData();
  CPPUNIT_ASSERT_MESSAGE (message,
	handlers->Front ()->Compare ("3D"));
  handlers->PopFront ();

  message = "Expected 2D, got ";
  message += handlers->Front()->GetData();
  CPPUNIT_ASSERT_MESSAGE (message,
	handlers->Front ()->Compare ("2D"));
  handlers->PopFront ();

  message = "Expected Console, got ";
  message += handlers->Front()->GetData();
  CPPUNIT_ASSERT_MESSAGE (message,
	handlers->Front ()->Compare ("Console"));
  handlers->PopFront ();

  message = "Expected Debug, got ";
  message += handlers->Front()->GetData();
  CPPUNIT_ASSERT_MESSAGE (message,
	handlers->Front ()->Compare ("Debug"));
  handlers->PopFront ();

  message = "Expected Frame, got ";
  message += handlers->Front()->GetData();
  CPPUNIT_ASSERT_MESSAGE (message,
	handlers->Front ()->Compare ("Frame"));
  handlers->PopFront ();

  CPPUNIT_ASSERT_MESSAGE ("List not empty", handlers->IsEmpty());
}

/**
 * Make sure combination of PreProc/Proc/PostProc/FinalProc and
 * phased Frame handlers get interleaved properly.
 */
void csEventQueueTest::testMixedHandlers ()
{

}
