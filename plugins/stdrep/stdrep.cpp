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
#include "cssysdef.h"
#include "stdrep.h"
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "cssys/sysfunc.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"
#include "ivideo/natwin.h"
#include "ivaria/conout.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csReporterListener)

SCF_EXPORT_CLASS_TABLE (stdrep)
  SCF_EXPORT_CLASS_DEP (csReporterListener, "crystalspace.utilities.stdrep",
    "Standard Reporter Listener",
    "crystalspace.utilities.reporter, crystalspace.console.output.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csReporterListener::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

void csReporterListener::DecRef ()
{
  scfRefCount--;
  if (scfRefCount <= 0)
  {
    SCF_TRACE (("  delete (%s *)%p\n", "csReporterListener", this));
    if (scfParent)
      scfParent->DecRef ();
    delete this;
  }
  else
    SCF_TRACE (("  (%s *)%p->DecRef (%d)\n", "csReporterListener", this, scfRefCount));
}

//SCF_IMPLEMENT_IBASE (csReporterListener)

SCF_IMPLEMENT_IBASE_INCREF(csReporterListener)
SCF_IMPLEMENT_IBASE_GETREFCOUNT(csReporterListener)
SCF_IMPLEMENT_IBASE_QUERY(csReporterListener)
  SCF_IMPLEMENTS_INTERFACE (iStandardReporterListener)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iReporterListener)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csReporterListener::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csReporterListener::ReporterListener)
  SCF_IMPLEMENTS_INTERFACE (iReporterListener)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csString csReporterListener::DefaultDebugFilename()
{
  csString username = csGetUsername();
  username.Collapse();
  csString s = "/tmp/csdebug";
  if (!username.IsEmpty())
    s << '-' << username;
  s << ".txt";
  return s;
}

csReporterListener::csReporterListener (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiReporterListener);
  object_reg = NULL;
  reporter = NULL;
  scfiEventHandler = NULL;

  debug_filename = DefaultDebugFilename ();
#ifdef CS_DEBUG
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_BUG, false, true, true, true, true, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_ERROR, false, true, true, true, true, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_WARNING, true, false, true, false, true, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_NOTIFY, true, false, true, false, true, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_DEBUG, true, false, true, false, true, false);
#else
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_BUG, false, true, true, true, true, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_ERROR, false, true, true, true, true, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_WARNING, true, false, true, false, false, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_NOTIFY, false, false, true, false, false, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_DEBUG, false, false, false, false, true, false);
#endif
  RemoveMessages (CS_REPORTER_SEVERITY_BUG, true);
  RemoveMessages (CS_REPORTER_SEVERITY_ERROR, true);
  RemoveMessages (CS_REPORTER_SEVERITY_WARNING, true);
  RemoveMessages (CS_REPORTER_SEVERITY_NOTIFY, true);
  RemoveMessages (CS_REPORTER_SEVERITY_DEBUG, true);
  ShowMessageID (CS_REPORTER_SEVERITY_BUG, true);
  ShowMessageID (CS_REPORTER_SEVERITY_ERROR, true);
#ifdef CS_DEBUG
  ShowMessageID (CS_REPORTER_SEVERITY_WARNING, true);
  ShowMessageID (CS_REPORTER_SEVERITY_NOTIFY, true);
#else
  ShowMessageID (CS_REPORTER_SEVERITY_WARNING, false);
  ShowMessageID (CS_REPORTER_SEVERITY_NOTIFY, false);
#endif
  ShowMessageID (CS_REPORTER_SEVERITY_DEBUG, true);
}

csReporterListener::~csReporterListener ()
{
  // @@@ this is to prevent the chicken/egg problem, note that this solution
  // lacks in the sense that we might listen to  a totally different
  // reporter then the one in the registry.
  // A more general solution could be to introduce a method in the
  // iReporterListener interface, something like
  // DontUseMeAnymoreSincerelyYourReporterOfChoice () which is called by
  // iReporter destructor for all listeners still in the listenerqueue
  //  .. norman
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
  {
    if (rep == reporter)
      reporter->RemoveReporterListener (&scfiReporterListener);
  }

  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
}

bool csReporterListener::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  SetDefaults ();

  if (!scfiEventHandler)
  {
    scfiEventHandler = new EventHandler (this);
  }
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing);

  return true;
}

bool csReporterListener::Report (iReporter*, int severity,
	const char* msgID, const char* description)
{
  csString msg;
  if (show_msgid[severity])
    msg.Format("%s: %s\n", msgID, description);
  else
    msg.Format("%s\n", description);

  if (dest_stdout[severity])
    csPrintf ("%s", msg.GetData());
  if (dest_stderr[severity])
    fputs (msg.GetData(), stderr);
  if (dest_console[severity] && console)
    console->PutText (msg.GetData());
  if (dest_alert[severity] && nativewm)
    nativewm->Alert (CS_ALERT_ERROR, "Fatal Error!", "Ok", msg.GetData());
  if (dest_debug[severity] && !debug_filename.IsEmpty())
  {
    if (!debug_file.IsValid())
    {
      csRef<iVFS> vfs (CS_QUERY_REGISTRY (object_reg, iVFS));
      if (vfs.IsValid())
        debug_file = vfs->Open (debug_filename, VFS_FILE_WRITE);
    }
    if (debug_file.IsValid())
    {
      debug_file->Write (msg.GetData(), msg.Length());
      debug_file->Flush ();
    }
  }
  if (dest_popup[severity])
  {
    csRef<csTimedMessage> tm = csPtr<csTimedMessage> (
    	new csTimedMessage (msg.GetData ()));
    messages.Push (tm);
  }
  return msg_remove[severity];
}

bool csReporterListener::HandleEvent (iEvent& event)
{
  if (event.Type == csevBroadcast)
  {
    if (event.Command.Code == cscmdPostProcess)
    {
      int l = messages.Length ();
      if (l > 0)
      {
	int i;
        csRef<iGraphics2D> g2d = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
        iFontServer* fntsvr = g2d->GetFontServer ();
        if (fntsvr)
        {
          csRef<iFont> fnt (fntsvr->GetFont (0));
	  if (fnt)
	  {
	    int sw = g2d->GetWidth ();
	    int sh = g2d->GetHeight ();
	    int fw, fh;
	    fnt->GetMaxSize (fw, fh);
	    int fg = g2d->FindRGB (0, 0, 0);
	    int bg = g2d->FindRGB (255, 255, 0);
	    int max_l = (sh-4-6-4-6) / (fh+4);
	    if (l > max_l) l = max_l;
	    g2d->DrawBox (4, 4, sw-8, 6 + l*(fh+4)-4 + 6, bg);
	    for (i = 0 ; i < l ; i++)
	    {
	      csTimedMessage* tm = messages[i];
              g2d->Write (fnt, 4+6, 4+6+i*(fh+4), fg, bg, tm->msg);
	      // Set the time the first time we could actually display it.
	      if (tm->time == 0) tm->time = csGetTicks () + 10000;
	    }
	    csTicks t = csGetTicks ();
	    i = 0;
	    // We only time out the messages we could actually display.
	    // That way the messages that didn't have a chance to display
	    // will be displayed later.
	    while (i < l)
	    {
	      csTimedMessage* tm = messages[i];
	      if (tm->time != 0 && t > tm->time)
	      {
	        messages.Delete (i);
	        l--;
	      }
	      else
	      {
	        i++;
	      }
	    }
          }
        }
      }
    }
  }

  return false;
}

void csReporterListener::SetOutputConsole (iConsoleOutput* console)
{
  csReporterListener::console = console;
}

void csReporterListener::SetNativeWindowManager (iNativeWindowManager* wm)
{
  nativewm = wm;
}

void csReporterListener::SetReporter (iReporter* reporter)
{
  if (csReporterListener::reporter)
    csReporterListener::reporter->RemoveReporterListener (
    	&scfiReporterListener);
  csReporterListener::reporter = reporter;
  if (reporter) reporter->AddReporterListener (&scfiReporterListener);
}

void csReporterListener::SetDebugFile (const char* s)
{
  debug_file = 0;
  debug_filename = s;
}

void csReporterListener::SetDefaults ()
{
  console = CS_QUERY_REGISTRY (object_reg, iConsoleOutput);
  nativewm = NULL;
#ifndef CS_USE_NEW_RENDERER
  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
#else
  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iRender3D));
#endif // CS_USE_NEW_RENDERER
  if (g3d)
  {
    iGraphics2D* g2d = g3d->GetDriver2D ();
    if (g2d)
    {
      nativewm = SCF_QUERY_INTERFACE (g2d, iNativeWindowManager);
    }
  }
  if (reporter)
  {
    reporter->RemoveReporterListener (&scfiReporterListener);
  }
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  reporter = rep;
  if (reporter)
  {
    reporter->AddReporterListener (&scfiReporterListener);
  }
  debug_file = 0;
  debug_filename = DefaultDebugFilename();
}

void csReporterListener::SetMessageDestination (int severity,
  	bool do_stdout, bool do_stderr, bool do_console,
	bool do_alert, bool do_debug, bool do_popup)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_stdout[severity] = do_stdout;
  dest_stderr[severity] = do_stderr;
  dest_console[severity] = do_console;
  dest_alert[severity] = do_alert;
  dest_debug[severity] = do_debug;
  dest_popup[severity] = do_popup;
}

void csReporterListener::RemoveMessages (int severity, bool remove)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  msg_remove[severity] = remove;
}

void csReporterListener::ShowMessageID (int severity, bool showid)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  show_msgid[severity] = showid;
}
