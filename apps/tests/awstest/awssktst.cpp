#include "cssysdef.h"
#include "awssktst.h"

#include <stdio.h>

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
  printf("awstest: red button clicked, source: %p, owner: %p\n", source, sink);
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

