#ifndef __AWS_STANDARD_SINK__
#define __AWS_STANDARD_SINK__

/************************************************************************************
  This sink provides the ability to trigger certain standard mechanisms in components,
 such as hiding and showing, from other components in a simple, straightforeward 
 manner.  Imitating the design of awsStandardSink is not encouraged, since it may make
 assumptions based on the internal architecture of AWS that other components and sinks
 may not be privy to.
 ************************************************************************************/

class awsStandardSink : public iAwSink
{
  /// Hides the source component
  static void Hide(void *sink, iAwsSource *source);

  /// Shows the source component
  static void Show(void *sink, iAwsSource *source);

  /// Hides the window that the source component belongs to
  static void HideWindow(void *sink, iAwsSource *source);

  /// Invalidates the source component  
  static void Invalidate(void *sink, iAwsSource *source);

public:
  awsStandardSink();
  virtual ~awsStandardSink();
};


#endif