#ifndef __AWS_SINK_TEST_H__
#define __AWS_SINK_TEST_H__

#include "iaws/aws.h"
#include "iutil/string.h"

class awsTestSink
{

  iAws     *wmgr;
  csRef<iAwsSink> sink;
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

  static void FillListBox(void *sink, iAwsSource *source);
  static void FillBarChart(void *sk, iAwsSource *source);
public:
  awsTestSink();
  virtual ~awsTestSink();

  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }

  void SetTestWin(iAwsWindow *testwin);
  void SetWindowManager(iAws *_wmgr);
};

#endif // __AWS_SINK_TEST_H__

