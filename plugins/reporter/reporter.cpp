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
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/refarr.h"
#include "reporter.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csReporter)


//-----------------------------------------------------------------------

class csReporterIterator : public iReporterIterator
{
public:
  csPDelArray<csReporterMessage> messages;
  size_t idx;

public:
  SCF_DECLARE_IBASE;

  csReporterIterator ()
  {
    SCF_CONSTRUCT_IBASE (0);
    idx = 0;
  }

  virtual ~csReporterIterator ()
  {
    SCF_DESTRUCT_IBASE();
  }

  virtual bool HasNext ()
  {
    return idx < messages.Length ();
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

SCF_IMPLEMENT_IBASE (csReporterIterator)
  SCF_IMPLEMENTS_INTERFACE (iReporterIterator)
SCF_IMPLEMENT_IBASE_END

//-----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csReporter)
  SCF_IMPLEMENTS_INTERFACE (iReporter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csReporter::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csReporter::csReporter (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  object_reg = 0;
  mutex = csMutex::Create (true);
}

csReporter::~csReporter ()
{
  Clear (-1);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
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

CS_IMPLEMENT_STATIC_VAR(GetReportBuf, csString, ());

void csReporter::ReportV (int severity, const char* msgId,
  	const char* description, va_list arg)
{
  csString& buf = *GetReportBuf();
  buf.FormatV (description, arg);

  // To ensure thread-safety we first copy the listeners.
  csRefArray<iReporterListener> copy;
  {
    csScopedMutexLock lock (mutex);
    size_t i;
    for (i = 0 ; i < listeners.Length () ; i++)
    {
      iReporterListener* listener = listeners[i];
      copy.Push (listener);
    }
  }

  bool add_msg = true;
  size_t i;
  for (i = 0 ; i < copy.Length () ; i++)
  {
    iReporterListener* listener = copy[i];
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
    csScopedMutexLock lock (mutex);
    messages.Push (msg);
    if (listeners.Length () == 0 && (severity == CS_REPORTER_SEVERITY_ERROR
    	|| severity == CS_REPORTER_SEVERITY_BUG))
    {
      csPrintfV (description, arg);
      csPrintf ("\n");
    }
  }
}

void csReporter::Clear (int severity)
{
  csScopedMutexLock lock (mutex);

  size_t i = 0;
  size_t len = messages.Length ();
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
  csScopedMutexLock lock (mutex);
  size_t i = 0;
  size_t len = messages.Length ();
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
  csScopedMutexLock lock (mutex);
  csReporterIterator* it = new csReporterIterator ();
  size_t i;
  for (i = 0 ; i < messages.Length () ; i++)
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
  csScopedMutexLock lock (mutex);
  listeners.Push (listener);
}

void csReporter::RemoveReporterListener (iReporterListener* listener)
{
  csScopedMutexLock lock (mutex);
  size_t idx = listeners.Find (listener);
  if (idx != csArrayItemNotFound)
  {
    listeners.DeleteIndex (idx);
  }
}

bool csReporter::FindReporterListener (iReporterListener* listener)
{
  csScopedMutexLock lock (mutex);
  size_t idx = listeners.Find (listener);
  return idx != csArrayItemNotFound;
}

csReporterMessage::~csReporterMessage ()
{
  delete[] id;
  delete[] description;
}

