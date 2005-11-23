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
  static void RedClicked(unsigned long, intptr_t sink,   iAwsSource *source);
  static void BlueClicked(unsigned long, intptr_t sink,  iAwsSource *source);
  static void GreenClicked(unsigned long, intptr_t sink, iAwsSource *source);

  static void SetPass(unsigned long, intptr_t sink, iAwsSource *source);
  static void SetUser(unsigned long, intptr_t sink, iAwsSource *source);
  static void Login(unsigned long, intptr_t sink, iAwsSource *source);

  static void FillListBox(unsigned long, intptr_t sink, iAwsSource *source);
  static void FillBarChart(unsigned long, intptr_t sk, iAwsSource *source);
public:
  awsTestSink();
  virtual ~awsTestSink();

  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }

  void SetTestWin(iAwsWindow *testwin);
  void SetWindowManager(iAws *_wmgr);
};

#endif // __AWS_SINK_TEST_H__

