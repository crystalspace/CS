#include "cssysdef.h"
#include "awssktst.h"
#include "csutil/scfstr.h"

#include <stdio.h>

static char *names[10] = { "Yellow", "Green", "Blue", "Orange", "Purple", "Red", "White", "Teal", "Black" };
static int   namec = 0;

awsTestSink::awsTestSink():sink(NULL)
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
 }
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

