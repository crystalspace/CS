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
  static void RedClicked(intptr_t sink,   iAwsSource *source);
  static void BlueClicked(intptr_t sink,  iAwsSource *source);
  static void GreenClicked(intptr_t sink, iAwsSource *source);

  static void SetPass(intptr_t sink, iAwsSource *source);
  static void SetUser(intptr_t sink, iAwsSource *source);
  static void Login(intptr_t sink, iAwsSource *source);

  static void FillListBox(intptr_t sink, iAwsSource *source);
  static void FillBarChart(intptr_t sk, iAwsSource *source);
public:
  awsTestSink();
  virtual ~awsTestSink();

  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }

  void SetTestWin(iAwsWindow *testwin);
  void SetWindowManager(iAws *_wmgr);
};

#endif // __AWS_SINK_TEST_H__

