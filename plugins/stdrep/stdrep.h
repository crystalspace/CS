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

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "ivaria/stdrep.h"
#include "ivaria/reporter.h"
#include "ivideo/fontserv.h"
#include "csutil/csstring.h"
#include "csutil/refcount.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"
#include "csutil/util.h"
#include "csutil/scopedmutexlock.h"

struct iConsoleOutput;
struct iFile;
struct iNativeWindowManager;

/**
 * Class to keep track of timed messages.
 */
class csTimedMessage : public csRefCount
{
public:
  char* msg;
  csTicks time;

  csTimedMessage () : msg (0) { }
  csTimedMessage (const char* m)
  {
    msg = csStrNew (m);
    time = 0;
  }

protected:
  virtual ~csTimedMessage ()
  {
    delete[] msg;
  }
};

/**
 * This plugin will listen to a reporter plugin and uses the console,
 * and other output devices to show appropriate messages based on
 * what comes from the reporter plugin.
 */
class csReporterListener : public iStandardReporterListener
{
private:
  iObjectRegistry* object_reg;
  csWeakRef<iConsoleOutput> console;
  csWeakRef<iNativeWindowManager> nativewm;
  iReporter* reporter;	// Not a csRef! We don't want to keep a reference.
  csString debug_filename;
  csRef<iFile> debug_file;
  bool dest_stdout[5];
  bool dest_stderr[5];
  bool dest_console[5];
  bool dest_alert[5];
  bool dest_debug[5];
  bool dest_popup[5];
  bool msg_remove[5];
  bool show_msgid[5];
  csRef<csMutex> mutex;
  csRefArray<csTimedMessage> messages;
  csString lastID;
  csRef<iFont> fnt;
  bool silent;	// Don't show the yellow warnings if true.
  bool append;  // If data should be appended to debug file instead of new    
  static csString DefaultDebugFilename();
  csString stdoutTmp;

public:
  SCF_DECLARE_IBASE;

  csReporterListener (iBase *iParent);
  virtual ~csReporterListener ();
  virtual bool Initialize (iObjectRegistry *object_reg);
  bool HandleEvent (iEvent& event);

  virtual void SetOutputConsole (iConsoleOutput* console);
  virtual void SetNativeWindowManager (iNativeWindowManager* wm);
  virtual void SetReporter (iReporter* wm);
  virtual void SetDebugFile (const char* filename, bool append ); // VFS path.
  virtual void SetDefaults ();
  virtual void SetMessageDestination (int severity,
  	bool do_stdout, bool do_stderr, bool do_console,
	bool do_alert, bool do_debug, bool do_popup = false);
  virtual void RemoveMessages (int severity, bool remove);
  virtual void ShowMessageID (int severity, bool showid);
  virtual const char* GetDebugFile ();

  bool Report (iReporter* reporter, int severity, const char* msgId,
  	const char* description);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csReporterListener);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;

  struct ReporterListener : public iReporterListener
  {
    SCF_DECLARE_EMBEDDED_IBASE (csReporterListener);
    virtual bool Report (iReporter* reporter, int severity, const char* msgId,
  	const char* description)
    {
      return scfParent->Report (reporter, severity, msgId, description);
    }
  } scfiReporterListener;

  // This is not an embedded interface in order to avoid
  // a circular reference between this registered event handler
  // and the parent object.
  class EventHandler : public iEventHandler
  {
  private:
    csReporterListener* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csReporterListener* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& ev)
    {
      return parent->HandleEvent (ev);
    }
  } *scfiEventHandler;
};

#endif // __CS_STDREP_H__
