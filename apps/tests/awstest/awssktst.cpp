#include "cssysdef.h"
#include "awssktst.h"
#include "csutil/scfstr.h"
#include "iaws/awsparm.h"

#include <stdio.h>

static char *names[10] = { "Yellow", "Green", "Blue", "Orange", "Purple", "Red", "White", "Teal", "Black" };
static int   namec = 0;

awsTestSink::awsTestSink():wmgr(NULL), sink(NULL), user(NULL), pass(NULL), test(NULL)
{  
}

awsTestSink::~awsTestSink()
{
  if (sink) sink->DecRef();
}

void
awsTestSink::SetSink(iAwsSink *s)
{
 sink=s;

 if (sink)
 {
   sink->IncRef();

   sink->RegisterTrigger("RedClicked", &RedClicked);
   sink->RegisterTrigger("BlueClicked", &BlueClicked);
   sink->RegisterTrigger("GreenClicked", &GreenClicked);
   sink->RegisterTrigger("SetUserName", &SetUser);
   sink->RegisterTrigger("SetPassword", &SetPass);
   sink->RegisterTrigger("Login", &Login);
   sink->RegisterTrigger("FillListBox", &FillListBox);
 }
}


void 
awsTestSink::SetTestWin(iAwsWindow *testwin)
{
  test=testwin;
}

void 
awsTestSink::SetWindowManager(iAws *_wmgr)
{
  wmgr=_wmgr;
}

void 
awsTestSink::FillListBox(void *sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  iAwsComponent *comp = source->GetComponent();

  iAwsParmList *pl=0;
  int parent;

  printf("awstest: Filling list box.\n");

  if (sink->wmgr)
    pl = sink->wmgr->CreateParmList();
  else
    printf("awstest: window manager is null.\n");

  if (pl==NULL)
  {
    printf("awstest: internal error, parameter list NULL.\n");
    return;
  }

  int q;
  for (q=0; q<10; ++q)
  {

    // Setup first row
    pl->AddString("text0", new scfString("Human"));

    pl->AddString("text1", new scfString("Enabled"));
    pl->AddBool("stateful1", true);

    pl->AddString("text2", new scfString("Shenobi"));

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    pl->Clear();

    //////////////////////////
    // Setup second row
    pl->AddString("text0", new scfString("Android"));

    pl->AddString("text1", new scfString("Enabled"));
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", true);

    pl->AddString("text2", new scfString("Tenalt"));

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    // Get the id of the last item for hierarchical support.
    pl->GetInt("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup third row (hierarchical)
    pl->AddString("text0", new scfString("Ship"));

    pl->AddString("text1", new scfString("Active"));
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", true);

    pl->AddString("text2", new scfString("Daedalus"));
    pl->AddInt("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", *pl);
    pl->Clear();

    //////////////////////////
    // Setup fourth row (hierarchical)
    pl->AddString("text0", new scfString("Ship"));

    pl->AddString("text1", new scfString("Active"));
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", new scfString("Temtor"));
    pl->AddInt("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    // Get the id of the last item for hierarchical support.
    pl->GetInt("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup fifth row (hierarchical)
    pl->AddString("text0", new scfString("TurboLaser"));

    pl->AddString("text1", new scfString("Active"));
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", new scfString("Johnny"));
    pl->AddInt("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    pl->Clear();

    //////////////////////////
    // Setup sixth row
    pl->AddString("text0", new scfString("Betarus"));

    pl->AddString("text1", new scfString("Enabled"));
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", new scfString("Sloth"));

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    pl->Clear();

    //////////////////////////
    // Setup seventh row
    pl->AddString("text0", new scfString("Vegan"));

    pl->AddString("text1", new scfString("Enabled"));
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", new scfString("Klamath"));

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    // Get the id of the last item for hierarchical support.
    pl->GetInt("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup third row (hierarchical)
    pl->AddString("text0", new scfString("Ship"));

    pl->AddString("text1", new scfString("Active"));
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", true);

    pl->AddString("text2", new scfString("Tertullia"));
    pl->AddInt("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", *pl);
    pl->Clear();

    //////////////////////////
    // Setup fourth row (hierarchical)
    pl->AddString("text0", new scfString("Ship"));

    pl->AddString("text1", new scfString("Active"));
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", new scfString("Gempus"));
    pl->AddInt("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    // Get the id of the last item for hierarchical support.
    pl->GetInt("id", &parent);
    pl->Clear();

    //////////////////////////
    // Setup fifth row (hierarchical)
    pl->AddString("text0", new scfString("BatterSlam"));

    pl->AddString("text1", new scfString("Active"));
    pl->AddBool("stateful1", true);
    pl->AddBool("groupstate1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", new scfString("James"));
    pl->AddInt("parent", parent);

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    pl->Clear();

    //////////////////////////
    // Setup eigth row
    pl->AddString("text0", new scfString("Antarian"));

    pl->AddString("text1", new scfString("Enabled"));
    pl->AddBool("stateful1", true);
    pl->AddBool("state1", false);

    pl->AddString("text2", new scfString("Gelvin"));

    // Add it into the list
    comp->Execute("InsertItem", *pl);

    pl->Clear();

  }

  pl->DecRef();
}

void 
awsTestSink::RedClicked(void *sink,   iAwsSource *source)
{
  printf("awstest: red button clicked, source: %p, owner: %p, component: %p\n", source, sink, source->GetComponent());

  namec++;
  if (namec > 8) namec=0;

  iAwsComponent *comp = source->GetComponent();
  comp->SetProperty("Caption", new scfString(names[namec]));
}

void 
awsTestSink::BlueClicked(void *sink,  iAwsSource *source)
{
  printf("awstest: blue button clicked, source: %p, owner: %p\n", source, sink);
}


void 
awsTestSink::GreenClicked(void *sink, iAwsSource *source)
{
  printf("awstest: green button clicked, source: %p, owner: %p\n", source, sink);
}

void 
awsTestSink::SetPass(void *sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  if (sink->pass) sink->pass->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (void**)&sink->pass); 
}

void 
awsTestSink::SetUser(void *sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  if (sink->user) sink->user->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (void**)&sink->user); 
}

void 
awsTestSink::Login(void *sk, iAwsSource *source)
{
  awsTestSink *sink = (awsTestSink *)sk;
  if (sink->user==NULL || sink->pass==NULL)
    printf("awstest: You must enter a username AND password.\n");

  else {
    printf("awstest: Logging in as %s with password: %s  (not really.)\n", sink->user->GetData(), sink->pass->GetData());
    iAwsComponent *comp = source->GetComponent();

    if (sink->wmgr) {
      iAwsParmList *pl = sink->wmgr->CreateParmList();

      comp->Execute("HideWindow", *pl);
      if (sink->test) sink->test->Show();

      pl->DecRef();
    }
  }
}

