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

#ifndef __CS_STDREP_H__
#define __CS_STDREP_H__

#include "isys/plugin.h"
#include "csutil/scf.h"
#include "ivaria/stdrep.h"
#include "ivaria/reporter.h"

struct iConsoleOutput;
struct iNativeWindowManager;

/**
 * This plugin will listen to a reporter plugin and uses the console,
 * and other output devices to show appropriate messages based on
 * what comes from the reporter plugin.
 */
class csReporterListener : public iStandardReporterListener
{
private:
  iObjectRegistry *object_reg;
  iConsoleOutput* console;
  iNativeWindowManager* nativewm;
  iReporter* reporter;
  char* debug_file;
  bool dest_stdout[5];
  bool dest_stderr[5];
  bool dest_console[5];
  bool dest_alert[5];
  bool dest_debug[5];
  bool msg_remove[5];
  bool show_msgid[5];

public:
  SCF_DECLARE_IBASE;

  csReporterListener (iBase *iParent);
  virtual ~csReporterListener ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  virtual void SetOutputConsole (iConsoleOutput* console);
  virtual void SetNativeWindowManager (iNativeWindowManager* wm);
  virtual void SetReporter (iReporter* wm);
  virtual void SetDebugFile (const char* filename);
  virtual void SetDefaults ();
  virtual void SetMessageDestination (int severity,
  	bool do_stdout, bool do_stderr, bool do_console,
	bool do_alert, bool do_debug);
  virtual void RemoveMessages (int severity, bool remove);
  virtual void ShowMessageID (int severity, bool showid);

  bool Report (iReporter* reporter, int severity, const char* msgId,
  	const char* description);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE (csReporterListener);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;

  struct ReporterListener : public iReporterListener
  {
    SCF_DECLARE_EMBEDDED_IBASE (csReporterListener);
    virtual bool Report (iReporter* reporter, int severity, const char* msgId,
  	const char* description)
    {
      return scfParent->Report (reporter, severity, msgId, description);
    }
  } scfiReporterListener;
};

#endif // __CS_STDREP_H__

