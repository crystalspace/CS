#ifndef __AWS_STANDARD_SINK__
# define __AWS_STANDARD_SINK__

/************************************************************************************
  This sink provides the ability to trigger certain standard mechanisms in components,
 such as hiding and showing, from other components in a simple, straightforeward
 manner.  Imitating the design of awsStandardSink is not encouraged, since it may make
 assumptions based on the internal architecture of AWS that other components and sinks
 may not be privy to.
 ************************************************************************************/
# include "iaws/aws.h"
# include "awsslot.h"

class awsStandardSink :
  public awsSink
{
  iAws *wmgr;

  /// Hides the source component
  static void Hide (void *sink, iAwsSource *source);

  /// Shows the source component
  static void Show (void *sink, iAwsSource *source);

  /// Hides the window that the source component belongs to
  static void HideWindow (void *sink, iAwsSource *source);

  /// Invalidates the source component
  static void Invalidate (void *sink, iAwsSource *source);

  /// Slides a window down and out
  static void WindowSlideOutDown (void *sink, iAwsSource *source);

  /// Slides a window up and out
  static void WindowSlideOutUp (void *sink, iAwsSource *source);

  /// Slides a window left and out
  static void WindowSlideOutLeft (void *sink, iAwsSource *source);
  
  /// Slides a window right and out
  static void WindowSlideOutRight (void *sink, iAwsSource *source);

public:
  awsStandardSink (iAws *_wmgr);
  virtual ~awsStandardSink ();
};
#endif
