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
#include "cssys/sysfunc.h"
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "stdrep.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivaria/conout.h"

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_FACTORY (csReporterListener)

SCF_EXPORT_CLASS_TABLE (stdrep)
  SCF_EXPORT_CLASS_DEP (csReporterListener, "crystalspace.utilities.stdrep",
    "Standard Reporter Listener",
    "crystalspace.utilities.reporter, crystalspace.console.output.")
SCF_EXPORT_CLASS_TABLE_END

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

csReporterListener::csReporterListener (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiReporterListener);
  object_reg = NULL;
  console = NULL;
  nativewm = NULL;
  reporter = NULL;
  debug_file = csStrNew ("debug.txt");
#ifdef CS_DEBUG
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_BUG, false, true, true, true, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_ERROR, false, true, true, true, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_WARNING, true, false, true, false, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_NOTIFY, true, false, true, false, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_DEBUG, true, false, true, false, true);
#else
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_BUG, false, true, true, true, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_ERROR, false, true, true, true, true);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_WARNING, true, false, true, false, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_NOTIFY, false, false, true, false, false);
  SetMessageDestination (
  	CS_REPORTER_SEVERITY_DEBUG, false, false, false, false, true);
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
  delete[] debug_file;
  // @@@ this is to prevent the chicken/egg problem, note that this solution
  // lacks in the sense that we might listen to  a totally different
  // reporter then the one in the registry.
  // A more general solution could be to introduce a method in the
  // iReporterListener interface, something like
  // DontUseMeAnymoreSincerelyYourReporterOfChoice () which is called by
  // iReporter destructor for all listeners still in the listenerqueue
  //  .. norman
  iReporter *rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    if (rep == reporter)
      reporter->RemoveReporterListener (&scfiReporterListener);
    rep->DecRef ();
  }
  if (console) console->DecRef ();
}

bool csReporterListener::Initialize (iObjectRegistry *object_reg)
{
  csReporterListener::object_reg = object_reg;
  SetDefaults ();
  return true;
}

bool csReporterListener::Report (iReporter*, int severity,
	const char* msgID, const char* description)
{
  char msgbuf[4096];
  if (show_msgid[severity])
    sprintf (msgbuf, "%s: %s\n", msgID, description);
  else
    sprintf (msgbuf, "%s\n", description);

  if (dest_stdout[severity])
    csPrintf ("%s", msgbuf);
  if (dest_stderr[severity])
    fputs (msgbuf, stderr);
  if (dest_console[severity] && console)
    console->PutText (msgbuf);
  if (dest_alert[severity] && nativewm)
    nativewm->Alert (CS_ALERT_ERROR, "Fatal Error!",
    	"Ok", msgbuf);
  if (dest_debug[severity] && debug_file)
  {
    static FILE *f = NULL;
    if (!f)
      f = fopen (debug_file, "a+");
    if (f)
    {
      fputs (msgbuf, f);
      fflush (f);
    }
  }
  return msg_remove[severity];
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

void csReporterListener::SetDebugFile (const char* filename)
{
  delete[] debug_file;
  debug_file = csStrNew (filename);
}

void csReporterListener::SetDefaults ()
{
  if (console) console->DecRef ();
  console = CS_QUERY_REGISTRY (object_reg, iConsoleOutput);
  nativewm = NULL;
  iGraphics3D* g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d)
  {
    iGraphics2D* g2d = g3d->GetDriver2D ();
    if (g2d)
    {
      nativewm = SCF_QUERY_INTERFACE (g2d, iNativeWindowManager);
      if (nativewm) nativewm->DecRef ();
    }
    g3d->DecRef ();
  }
  if (reporter)
  {
    reporter->RemoveReporterListener (&scfiReporterListener);
  }
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
  {
    reporter->AddReporterListener (&scfiReporterListener);
    reporter->DecRef ();
  }
  delete[] debug_file;
  debug_file = csStrNew ("debug.txt");
}

void csReporterListener::SetMessageDestination (int severity,
  	bool do_stdout, bool do_stderr, bool do_console,
	bool do_alert, bool do_debug)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_stdout[severity] = do_stdout;
  dest_stderr[severity] = do_stderr;
  dest_console[severity] = do_console;
  dest_alert[severity] = do_alert;
  dest_debug[severity] = do_debug;
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

