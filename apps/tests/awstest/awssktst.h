#ifndef __AWS_SINK_TEST_H
#define __AWS_SINK_TEST_H

#include "iaws/iaws.h"
#include "iutil/string.h"

class awsTestSink 
{

  iAws     *wmgr;
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
  void SetWindowManager(iAws *_wmgr);
};

#endif