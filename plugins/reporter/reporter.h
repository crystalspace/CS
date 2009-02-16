/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_REPORTER_H__
#define __CS_REPORTER_H__

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/scf_implementation.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/threading/mutex.h"
#include "ivaria/reporter.h"

/**
 * A message.
 */
struct csReporterMessage
{
  int severity;
  char* id;
  char* description;

  csReporterMessage () : id (0), description (0) { }
  ~csReporterMessage ();
};

/**
 * Reporter plugin. This plugin supports the notion of error and
 * general message reporting.
 */
class csReporter :
  public scfImplementation2<csReporter, iReporter, iComponent>
{
private:
  CS::Threading::RecursiveMutex mutex;
  iObjectRegistry *object_reg;
  csPDelArray<csReporterMessage> messages;
  csRefArray<iReporterListener> listeners;

  /// Whether Report() call is nested
  bool inReporting;
  struct ReportedMessage
  {
    int severity;
    csString msgID;
    csStringFast<768> buf;
  };
  /// Queue of messages that were reported while nested
  csArray<ReportedMessage> messageQueue;
  CS::Threading::RecursiveMutex messageQueueMutex;
  /// Actually report a message to listeners and record
  void ActualReport (const csRefArray<iReporterListener>& listeners,
    int severity, const char* msgId, const char* buf);
public:
  csReporter (iBase *iParent);
  virtual ~csReporter ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  virtual void Report (int severity, const char* msgId,
  	const char* description, ...) CS_GNUC_PRINTF (4, 5);
  virtual void ReportV (int severity, const char* msgId,
  	const char* description, va_list) CS_GNUC_PRINTF (4, 0);
  virtual void Clear (int severity = -1);
  virtual void Clear (const char* mask);
  virtual csPtr<iReporterIterator> GetMessageIterator ();
  virtual void AddReporterListener (iReporterListener* listener);
  virtual void RemoveReporterListener (iReporterListener* listener);
  virtual bool FindReporterListener (iReporterListener* listener);
};

#endif // __CS_REPORTER_H__

