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

#include "cssysdef.h"
#include <string.h>
#include "csutil/csstring.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/refarr.h"
#include "reporter.h"



SCF_IMPLEMENT_FACTORY (csReporter)


//-----------------------------------------------------------------------

class csReporterIterator :
  public scfImplementation1<csReporterIterator, iReporterIterator>
{
public:
  csPDelArray<csReporterMessage> messages;
  size_t idx;

public:
  csReporterIterator () : scfImplementationType(this)
  {
    idx = 0;
  }

  virtual ~csReporterIterator ()
  {
  }

  virtual bool HasNext ()
  {
    return idx < messages.GetSize ();
  }

  virtual void Next ()
  {
    idx++;
  }

  virtual int GetMessageSeverity () const
  {
    return messages[idx-1]->severity;
  }

  virtual const char* GetMessageId () const
  {
    return messages[idx-1]->id;
  }

  virtual const char* GetMessageDescription () const
  {
    return messages[idx-1]->description;
  }
};

//-----------------------------------------------------------------------

csReporter::csReporter (iBase *iParent) :
  scfImplementationType(this, iParent), inReporting (false)
{
  object_reg = 0;
}

csReporter::~csReporter ()
{
  Clear (-1);
}

void csReporter::ActualReport (const csRefArray<iReporterListener>& listeners,
                               int severity, const char* msgId, 
                               const char* buf)
{
  bool add_msg = true;
  size_t i;
  for (i = 0 ; i < listeners.GetSize () ; i++)
  {
    iReporterListener* listener = listeners[i];
    if (listener->Report (this, severity, msgId, buf))
    {
      add_msg = false;
      break;
    }
  }

  if (add_msg)
  {
    csReporterMessage* msg = new csReporterMessage ();
    msg->severity = severity;
    msg->id = csStrNew (msgId);
    msg->description = csStrNew (buf);
    CS::Threading::RecursiveMutexScopedLock lock (mutex);
    messages.Push (msg);
    if (listeners.GetSize () == 0 && (severity == CS_REPORTER_SEVERITY_ERROR
    	|| severity == CS_REPORTER_SEVERITY_BUG))
    {
      csPrintf ("%s\n", buf);
    }
  }
}

bool csReporter::Initialize (iObjectRegistry *object_reg)
{
  csReporter::object_reg = object_reg;
  return true;
}

void csReporter::Report (int severity, const char* msgId,
  	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportV (severity, msgId, description, arg);
  va_end (arg);
}

void csReporter::ReportV (int severity, const char* msgId,
  	const char* description, va_list arg)
{
  csStringFast<768> buf;
  buf.FormatV (description, arg);

  if (inReporting)
  {
    ReportedMessage msg;
    msg.buf = buf;
    msg.msgID = msgId;
    msg.severity = severity;
    CS::Threading::RecursiveMutexScopedLock mqlock (messageQueueMutex);
    messageQueue.Push (msg);
    return;
  }
  inReporting = true;

  // To ensure thread-safety we first copy the listeners.
  csRefArray<iReporterListener> copy;
  {
    CS::Threading::RecursiveMutexScopedLock lock (mutex);
    size_t i;
    for (i = 0 ; i < listeners.GetSize () ; i++)
    {
      iReporterListener* listener = listeners[i];
      copy.Push (listener);
    }
  }

  ActualReport (copy, severity, msgId, buf);

  size_t n = 0;
  CS::Threading::RecursiveMutexScopedLock mqlock (messageQueueMutex);
  while (n < messageQueue.GetSize())
  {
    // Copy to protect against memory being reallocated
    ReportedMessage msg (messageQueue[n]);
    messageQueueMutex.Unlock ();
    ActualReport (copy, msg.severity, msg.msgID, msg.buf);
    messageQueueMutex.Lock ();
    n++;
  }
  messageQueue.DeleteAll();

  inReporting = false;
}

void csReporter::Clear (int severity)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  size_t i = 0;
  size_t len = messages.GetSize ();
  while (i < len)
  {
    csReporterMessage* msg = messages[i];
    if (severity == -1 || msg->severity == severity)
    {
      messages.DeleteIndex (i);
      len--;
    }
    else
    {
      i++;
    }
  }
}

void csReporter::Clear (const char* mask)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t i = 0;
  size_t len = messages.GetSize ();
  while (i < len)
  {
    csReporterMessage* msg = messages[i];
    if (csGlobMatches (msg->id, mask))
    {
      messages.DeleteIndex (i);
      len--;
    }
    else
    {
      i++;
    }
  }
}

csPtr<iReporterIterator> csReporter::GetMessageIterator ()
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  csReporterIterator* it = new csReporterIterator ();
  size_t i;
  for (i = 0 ; i < messages.GetSize () ; i++)
  {
    csReporterMessage* msg = new csReporterMessage ();
    msg->severity = messages[i]->severity;
    msg->id = csStrNew (messages[i]->id);
    msg->description = csStrNew (messages[i]->description);
    it->messages.Push (msg);
  }
  return csPtr<iReporterIterator> (it);
}

void csReporter::AddReporterListener (iReporterListener* listener)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  listeners.Push (listener);
}

void csReporter::RemoveReporterListener (iReporterListener* listener)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t idx = listeners.Find (listener);
  if (idx != csArrayItemNotFound)
  {
    listeners.DeleteIndex (idx);
  }
}

bool csReporter::FindReporterListener (iReporterListener* listener)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t idx = listeners.Find (listener);
  return idx != csArrayItemNotFound;
}

csReporterMessage::~csReporterMessage ()
{
  delete[] id;
  delete[] description;
}

