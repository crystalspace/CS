#include "cssysdef.h"
#include "awssktst.h"
#include "csutil/scfstr.h"
#include "aws/awsfparm.h"

#include <stdio.h>

static char *names[10] = { "Yellow", "Green", "Blue", "Orange", "Purple", "Red", "White", "Teal", "Black" };
static int   namec = 0;

awsTestSink::awsTestSink():sink(NULL), user(NULL), pass(NULL), test(NULL)
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
 }
}

void 
awsTestSink::SetTestWin(iAwsWindow *testwin)
{
  test=testwin;
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
    awsParmList pl;

    comp->Execute("HideWindow", pl);
    if (sink->test) sink->test->Show();
  }

}