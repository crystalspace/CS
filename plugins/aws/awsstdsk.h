/*
    Copyright (C) 2000-2001 by Christopher Nelson
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_AWS_STDSK_H__
#define __CS_AWS_STDSK_H__
 
#include "iaws/aws.h"
#include "awsslot.h"

/**
 * This sink provides the ability to trigger certain standard mechanisms
 * in components, such as hiding and showing, from other components in a
 * simple, straightforeward manner.  Imitating the design of awsStandardSink
 * is not encouraged, since it may make assumptions based on the internal
 * architecture of AWS that other components and sinks may not be privy to.
 */

class awsStandardSink : public awsSink
{
private:
  iAws *wmgr;

  /// Hides the source component.
  static void Hide (intptr_t sink, iAwsSource *source);

  /// Shows the source component.
  static void Show (intptr_t sink, iAwsSource *source);

  /// Hides the window that the source component belongs to.
  static void HideWindow (intptr_t sink, iAwsSource *source);

  /// Maximizes the window that the source component belongs to.
  static void MaximizeWindow (intptr_t sink, iAwsSource *source);

  /// UnMaximizes the window that the source component belongs to.
  static void UnMaximizeWindow (intptr_t sink, iAwsSource *source);

  /// Invalidates the source component.
  static void Invalidate (intptr_t sink, iAwsSource *source);

  /// Slides a window down and out.
  static void WindowSlideOutDown (intptr_t sink, iAwsSource *source);

  /// Slides a window up and out.
  static void WindowSlideOutUp (intptr_t sink, iAwsSource *source);

  /// Slides a window left and out.
  static void WindowSlideOutLeft (intptr_t sink, iAwsSource *source);
  
  /// Slides a window right and out.
  static void WindowSlideOutRight (intptr_t sink, iAwsSource *source);
public:
  awsStandardSink (iAws *_wmgr);
  virtual ~awsStandardSink ();
};

#endif // __CS_AWS_STDSK_H__
