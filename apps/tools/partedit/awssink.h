#ifndef __AWS_SINK_H__
#define __AWS_SINK_H__

#include "iaws/aws.h"
#include "iutil/string.h"

class csString;

class awsSink
{

  iAws     *wmgr;
  csRef<iAwsSink> sink;
  static awsSink *asink;
  iAwsComponent *textbox1;
  iAwsComponent *textbox2;
  iAwsComponent *textbox3;
  iAwsComponent *textbox4;
  iAwsComponent *label1;
  iAwsComponent *label2;
  iAwsComponent *label3;
  iAwsComponent *label4;
  iAwsComponent *frame;

  float *valueX;
  float *valueY;
  float *valueZ;
  float *valueOther;

  bool updatestate;

  char *setinput;
  iString *getinput1;
  iString *getinput2;
  iString *getinput3;
  iString *getinput4;

  iAwsWindow *test;

private:
  static void SetScrollBarX(void *sink, iAwsSource *source);
  static void SetScrollBarY(void *sink, iAwsSource *source);
  static void SetScrollBarZ(void *sink, iAwsSource *source);
  static void SetScrollBarOther(void *sink, iAwsSource *source);
  static void SetInput1(void *sk, iAwsSource *source);  
  static void SetInput2(void *sk, iAwsSource *source);  
  static void SetInput3(void *sk, iAwsSource *source);  
  static void SetInput4(void *sk, iAwsSource *source);  
  static void SetSpeed(void *sk, iAwsSource *source);
  static void SetAccel(void *sk, iAwsSource *source);
  static void SetPosition(void *sk, iAwsSource *source);
  static void SetRectangle(void *sk, iAwsSource *source);
  static void SetNumParticle(void *sk, iAwsSource *source);
  static void SetParticleTime(void *sk, iAwsSource *source);
  static void SetAddAge(void *sk, iAwsSource *source);
  static void RegisterInput1(void *sk, iAwsSource *source);
  static void RegisterInput2(void *sk, iAwsSource *source);
  static void RegisterInput3(void *sk, iAwsSource *source);
  static void RegisterInput4(void *sk, iAwsSource *source);
  static void RegisterLabel1(void *sk, iAwsSource *source);
  static void RegisterLabel2(void *sk, iAwsSource *source);
  static void RegisterLabel3(void *sk, iAwsSource *source);
  static void RegisterLabel4(void *sk, iAwsSource *source);
  static void RegisterFrame(void *sk, iAwsSource *source);

public:
  awsSink();
  virtual ~awsSink();

  void SetSink(iAwsSink *s);
  iAwsSink *GetSink() { return sink; }

  void SetTestWin(iAwsWindow *testwin);
  void SetWindowManager(iAws *_wmgr);

  void UpdateInput(float value,int textboxnum);
  void UpdateLabel(csString txt,int textboxnum);
  
  float GetValueX(float scale,float phase);
  float GetValueY(float scale,float phase);
  float GetValueZ(float scale,float phase);
  float GetValueOther(float scale);
  int GetState();
  bool GetUpdateState();
  void SetUpdateState(bool state);
};

#endif // __AWS_SINK_TEST_H__


