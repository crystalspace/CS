#ifndef __AWS_SINK_TEST_H
#define __AWS_SINK_TEST_H

#include "aws/iaws.h"
#include "aws/awsslot.h"

class awsTestSink 
{

  iAwsSink *sink;

private:
  static void RedClicked(void *sink,   iAwsSource *source);
  static void BlueClicked(void *sink,  iAwsSource *source);
  static void GreenClicked(void *sink, iAwsSource *source);

public:
  awsTestSink();
  virtual ~awsTestSink();
  
  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }
};

#endif