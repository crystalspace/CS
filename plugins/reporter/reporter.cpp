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

#include <string.h>
#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "reporter.h"
#include "isys/system.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csReporter)

SCF_EXPORT_CLASS_TABLE (reporter)
  SCF_EXPORT_CLASS (csReporter, "crystalspace.utilities.reporter",
    "Reporting utility")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csReporter)
  SCF_IMPLEMENTS_INTERFACE (iReporter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csReporter::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csReporter::csReporter (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);
  System = NULL;
}

csReporter::~csReporter ()
{
  Clear (-1);
}

bool csReporter::Initialize (iSystem *system)
{
  System = system;
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
  char buf[1024];
  vsprintf (buf, description, arg);

  csReporterMessage* msg = new csReporterMessage ();
  msg->severity = severity;
  msg->id = csStrNew (msgId);
  msg->description = csStrNew (buf);
  messages.Push (msg);
}

void csReporter::Clear (int severity)
{
  int i = 0;
  int len = messages.Length ();
  while (i < len)
  {
    csReporterMessage* msg = (csReporterMessage*)messages[i];
    if (severity == -1 || msg->severity == severity)
    {
      delete msg;
      messages.Delete (i);
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
  int i = 0;
  int len = messages.Length ();
  while (i < len)
  {
    csReporterMessage* msg = (csReporterMessage*)messages[i];
    if (csGlobMatches (msg->id, mask))
    {
      delete msg;
      messages.Delete (i);
      len--;
    }
    else
    {
      i++;
    }
  }
}

int csReporter::GetMessageCount () const
{
  return messages.Length ();
}

int csReporter::GetMessageSeverity (int idx) const
{
  if (idx < 0 || idx >= messages.Length ()) return -1;
  csReporterMessage* msg = (csReporterMessage*)messages[idx];
  return msg->severity;
}

const char* csReporter::GetMessageId (int idx) const
{
  if (idx < 0 || idx >= messages.Length ()) return NULL;
  csReporterMessage* msg = (csReporterMessage*)messages[idx];
  return msg->id;
}

const char* csReporter::GetMessageDescription (int idx) const
{
  if (idx < 0 || idx >= messages.Length ()) return NULL;
  csReporterMessage* msg = (csReporterMessage*)messages[idx];
  return msg->description;
}

csReporterMessage::~csReporterMessage ()
{
  delete[] id;
  delete[] description;
}

