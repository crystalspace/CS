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

#include "isys/plugin.h"
#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "ivaria/reporter.h"

/**
 * A message.
 */
struct csReporterMessage
{
  int severity;
  char* id;
  char* description;

  csReporterMessage () : id (NULL), description (NULL) { }
  ~csReporterMessage ();
};

/**
 * Reporter plugin. This plugin supports the notion of error and
 * general message reporting.
 */
class csReporter : public iReporter
{
private:
  iSystem *System;
  csVector messages;

public:
  SCF_DECLARE_IBASE;

  csReporter (iBase *iParent);
  virtual ~csReporter ();
  virtual bool Initialize (iSystem *system);

  virtual void Report (int severity, const char* msgId,
  	const char* description, ...);
  virtual void ReportV (int severity, const char* msgId,
  	const char* description, va_list);
  virtual void Clear (int severity = -1);
  virtual void Clear (const char* mask);
  virtual int GetMessageCount () const;
  virtual int GetMessageSeverity (int idx) const;
  virtual const char* GetMessageId (int idx) const;
  virtual const char* GetMessageDescription (int idx) const;

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE (csReporter);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return true; }
  } scfiPlugin;
};

#endif // __CS_REPORTER_H__

