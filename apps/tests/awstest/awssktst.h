#ifndef __AWS_SINK_TEST_H
#define __AWS_SINK_TEST_H

#include "aws/iaws.h"
#include "aws/awsslot.h"
#include "iutil/string.h"

class awsTestSink 
{

  iAwsSink *sink;
  iString  *user;
  iString  *pass;

  iAwsWindow *test;
  
private:
  static void RedClicked(void *sink,   iAwsSource *source);
  static void BlueClicked(void *sink,  iAwsSource *source);
  static void GreenClicked(void *sink, iAwsSource *source);

  static void SetPass(void *sink, iAwsSource *source);
  static void SetUser(void *sink, iAwsSource *source);
  static void Login(void *sink, iAwsSource *source);

public:
  awsTestSink();
  virtual ~awsTestSink();
  
  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }

  void SetTestWin(iAwsWindow *testwin);
};

#endif