#include "cssysdef.h"
#include "awssktst.h"

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

}

void 
awsTestSink::BlueClicked(void *sink,  iAwsSource *source)
{

}


void 
awsTestSink::GreenClicked(void *sink, iAwsSource *source)
{

}

