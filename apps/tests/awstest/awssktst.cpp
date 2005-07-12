#include "cssysdef.h"
#include "awstest_config.h"

#ifndef TEST_AWS2

#include "awssktst.h"
#include "csutil/scfstr.h"
#include "csutil/sysfunc.h"
#include "iaws/awsparm.h"

#include <stdio.h>

static char *names[10] = { "Yellow", "Green", "Blue", "Orange", "Purple", "Red", "White", "Teal", "Black" };
static int   namec = 0;

awsTestSink::awsTestSink() : wmgr(0), user(0), pass(0), test(0)
{
}

awsTestSink::~awsTestSink()
{
}

void awsTestSink::SetSink(iAwsSink *s)
{
 sink = s;

 if (sink)
 {
   sink->RegisterTrigger("RedClicked", &RedClicked);
   sink->RegisterTrigger("BlueClicked", &BlueClicked);
   sink->RegisterTrigger("GreenClicked", &GreenClicked);
   sink->RegisterTrigger("SetUserName", &SetUser);
   sink->RegisterTrigger("SetPassword", &SetPass);
   sink->RegisterTrigger("Login", &Login);
   sink->RegisterTrigger("FillListBox", &FillListBox);
   sink->RegisterTrigger("FillBarChart", &FillBarChart);
 }
}


void awsTestSink::SetTestWin(iAwsWindow *testwin)
{
  test=testwin;
}

void awsTestSink::SetWindowManager(iAws *_wmgr)
{
  wmgr=_wmgr;
}

void awsTestSink::FillBarChart(intptr_t sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  iAwsComponent *comp = source->GetComponent();

  iAwsParmList *pl=0;

  if (sink->wmgr)
    pl = sink->wmgr->CreateParmList();
  else
    csPrintf("awstest: window manager is null.\n");


  pl->AddFloat("value", 10);
  comp->Execute("AddItem", pl);
  pl->Clear();

  pl->AddFloat("value", 20);
  comp->Execute("AddItem", pl);
  pl->Clear();

  pl->AddFloat("value", 30);
  comp->Execute("AddItem", pl);
  pl->Clear();

  pl->AddFloat("value", 5);
  comp->Execute("AddItem", pl);
  pl->Clear();
  
  pl->DecRef();
}


void awsTestSink::FillListBox(intptr_t sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  iAwsComponent *comp = source->GetComponent();

  iAwsParmList *pl=0;
  intptr_t  parent;

  csPrintf("awstest: Filling list box.\n");

  if (sink->wmgr)
    pl = sink->wmgr->CreateParmList();
  else
    csPrintf("awstest: window manager is null.\n");

  if (pl==0)
  {
    csPrintf("awstest: internal error, parameter list 0.\n");
    return;
  }

  int q;
  for (q=0; q<10; ++q)
  {
    // Setup first row
    pl->AddString("text0", "Human");

    pl->AddString("text1", "Enabled");
    pl->AddBool("stateful1", true);

    pl->AddString("text2", "Shenobi");

    // Add it into the list
    comp->Execute("InsertItem", pl);

    pl->Clear();

    //////////////////////////
    // Setup second row
    pl->AddString("text0", "Android");

    pl->AddString("text1", "Enabled");
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", true);

    pl->AddString("text2", "Tenalt");

    // Add it into the list
    comp->Execute("InsertItem", pl);

    // Get the id of the last item for hierarchical support.
    pl->GetOpaque("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup third row (hierarchical)
    pl->AddString("text0", "Ship");

    pl->AddString("text1", "Active");
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", true);

    pl->AddString("text2", "Daedalus");
    pl->AddOpaque("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", pl);
    pl->Clear();

    //////////////////////////
    // Setup fourth row (hierarchical)
    pl->AddString("text0", "Ship");

    pl->AddString("text1", "Active");
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", "Temtor");
    pl->AddOpaque("Opaque", parent);

    // Add it into the list
    comp->Execute("InsertItem", pl);

    // Get the id of the last item for hierarchical support.
    pl->GetOpaque("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup fifth row (hierarchical)
    pl->AddString("text0", "TurboLaser");

    pl->AddString("text1", "Active");
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", "Johnny");
    pl->AddOpaque("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", pl);

    pl->Clear();

    //////////////////////////
    // Setup sixth row
    pl->AddString("text0", "Betarus");

    pl->AddString("text1", "Enabled");
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", "Sloth");

    // Add it into the list
    comp->Execute("InsertItem", pl);

    pl->Clear();

    //////////////////////////
    // Setup seventh row
    pl->AddString("text0", "Vegan");

    pl->AddString("text1", "Enabled");
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", "Klamath");

    // Add it into the list
    comp->Execute("InsertItem", pl);

    // Get the id of the last item for hierarchical support.
    pl->GetOpaque("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup third row (hierarchical)
    pl->AddString("text0", "Ship");

    pl->AddString("text1", "Active");
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", true);

    pl->AddString("text2", "Tertullia");
    pl->AddOpaque("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", pl);
    pl->Clear();

    //////////////////////////
    // Setup fourth row (hierarchical)
    pl->AddString("text0", "Ship");

    pl->AddString("text1", "Active");
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", "Gempus");
    pl->AddOpaque("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", pl);

    // Get the id of the last item for hierarchical support.
    pl->GetOpaque("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup fifth row (hierarchical)
    pl->AddString("text0", "BatterSlam");

    pl->AddString("text1", "Active");
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", "James");
    pl->AddOpaque("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", pl);

    pl->Clear();

    //////////////////////////
    // Setup eigth row
    pl->AddString("text0", "Antarian");

    pl->AddString("text1", "Enabled");
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", "Gelvin");

    // Add it into the list
    comp->Execute("InsertItem", pl);

    pl->Clear();
  }

  pl->DecRef();
}

void awsTestSink::RedClicked(intptr_t sink, iAwsSource *source)
{
  csPrintf("awstest: red button clicked, source: %p, owner: %p, component: %p\n", source, (void*)sink, source->GetComponent());

  namec++;
  if (namec > 8) namec=0;

  iAwsComponent *comp = source->GetComponent();
  comp->SetProperty("Caption", (intptr_t)new scfString(names[namec]));
}

void awsTestSink::BlueClicked(intptr_t sink, iAwsSource *source)
{
  csPrintf("awstest: blue button clicked, source: %p, owner: %p\n", source, (void*)sink);
}


void awsTestSink::GreenClicked(intptr_t sink, iAwsSource *source)
{
  csPrintf("awstest: green button clicked, source: %p, owner: %p\n", source, (void*)sink);
}

void awsTestSink::SetPass(intptr_t sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  if (sink->pass) sink->pass->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (intptr_t*)&sink->pass);
}

void awsTestSink::SetUser(intptr_t sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  if (sink->user) sink->user->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (intptr_t*)&sink->user);
}

void awsTestSink::Login(intptr_t sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  if (sink->user==0 || sink->pass==0)
    csPrintf("awstest: You must enter a username AND password.\n");

  else {
    csPrintf("awstest: Logging in as %s with password: %s  (not really.)\n", sink->user->GetData(), sink->pass->GetData());
    iAwsComponent *comp = source->GetComponent();

    if (sink->wmgr) {
      comp->Execute("HideWindow");
      if (sink->test) sink->test->Show();
    }
  }
}

#endif // end only compile if NOT testing aws2