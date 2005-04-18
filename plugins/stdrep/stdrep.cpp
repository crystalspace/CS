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
#include "stdrep.h"
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/sysfunc.h"
#include "iutil/cfgmgr.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "iutil/cmdline.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/verbositymanager.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"
#include "ivideo/natwin.h"
#include "ivaria/conout.h"
  
CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csReporterListener)


SCF_IMPLEMENT_IBASE (csReporterListener::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csReporterListener)
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
  mutex = csMutex::Create (true);
  object_reg = 0;
  reporter = 0;
  scfiEventHandler = 0;
  silent = false;
  append = false;

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

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiReporterListener);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
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

  csRef<iConfigManager> cfg(CS_QUERY_REGISTRY (r, iConfigManager));
  if ( cfg )
  {
    append = cfg->GetBool("Reporter.FileAppend", false );
  }
  
  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
    iCommandLineParser);
  if (cmdline)
  {
    silent = cmdline->GetOption ("silent") != 0;
    append = cmdline->GetOption ("append") != 0;
  }
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
  {
    bool verbose = verbosemgr->Enabled ("stdrep");
    if (verbose)
    {
      dest_stdout[CS_REPORTER_SEVERITY_WARNING] = true;
      dest_stderr[CS_REPORTER_SEVERITY_WARNING] = false;
      dest_stdout[CS_REPORTER_SEVERITY_NOTIFY] = true;
      dest_stderr[CS_REPORTER_SEVERITY_NOTIFY] = false;
      dest_stdout[CS_REPORTER_SEVERITY_DEBUG] = true;
      dest_stderr[CS_REPORTER_SEVERITY_DEBUG] = false;
    }
  }

  return true;
}

static const char* consolePrefix[5] = 
{
  CS_ANSI_BK CS_ANSI_FM CS_ANSI_FI,
  CS_ANSI_BK CS_ANSI_FR CS_ANSI_FI,
  CS_ANSI_BK CS_ANSI_FY CS_ANSI_FI,
  "",
  ""
};

static const char* consoleSuffix[5] = 
{
  CS_ANSI_RST,
  CS_ANSI_RST,
  CS_ANSI_RST,
  "",
  ""
};

bool csReporterListener::Report (iReporter*, int severity,
	const char* msgID, const char* description)
{
  bool repeatedID = false;
  if (lastID.Compare (msgID))
    repeatedID = true;
  else
    lastID = msgID;

  csString msg;
  if (show_msgid[severity])
    msg.Format("%s:  %s\n", msgID, description);
  else
    msg.Format("%s\n", description);

  if (dest_stdout[severity])
  {
    if (!repeatedID)
    {
      stdoutTmp << '\n' << msgID << ":\n";
    }
    stdoutTmp << consolePrefix[severity];
    int offset = 0;
    while (strlen(description+offset)>77)
    {
      csString str (description+offset);
      str.Truncate (77);
      int linebreak = strrchr (str.GetData (), ' ')-str.GetData ();
      if (linebreak>0)
      {
        str.Truncate (linebreak);
	stdoutTmp << "  " << str << '\n';
        offset += linebreak+1;
      }
      else
      {
        //csPrintf ("  %0.77s\n", str.GetData ());
	stdoutTmp << "  " << str << '\n';
        offset += 77;
      }
    }
    //csPrintf ("  %s\n", description+offset);
    stdoutTmp << "  " << description+offset << '\n';
    stdoutTmp << consoleSuffix[severity];
    csPrintf ("%s", stdoutTmp.GetData());
    stdoutTmp.Truncate (0);
  }
  if (dest_stderr[severity])
    csPrintfErr ("%s%s%s", consolePrefix[severity], msg.GetData(), 
      consoleSuffix[severity]);
  if (dest_console[severity] && console)
    console->PutText ("%s", msg.GetData());
  if (dest_alert[severity] && nativewm)
    nativewm->Alert (CS_ALERT_ERROR, "Fatal Error!", "Ok", "%s",
      msg.GetData());
  if (dest_debug[severity] && !debug_filename.IsEmpty())
  {
    if (!debug_file.IsValid())
    {
      csRef<iVFS> vfs (CS_QUERY_REGISTRY (object_reg, iVFS));
      if (vfs.IsValid())
      {  
        // If log does not exists then create a new one    
        if ( !(vfs->Exists(debug_filename)) )
        {
            debug_file = vfs->Open (debug_filename, VFS_FILE_WRITE);
        } 
        else               
        {
          // if the log file exists open up using desired behaviour.
          if ( append )
          {
            debug_file = vfs->Open (debug_filename, VFS_FILE_APPEND);            
          }
          else
          { 
            debug_file = vfs->Open (debug_filename, VFS_FILE_WRITE);
          }    
        }        
      }        
    }
    if (debug_file.IsValid())
    {
      debug_file->Write (msg.GetData(), msg.Length());
      debug_file->Flush ();
    }
  }
  if (dest_popup[severity])
  {
    if (!silent)
    {
      csScopedMutexLock lock (mutex);
      csString popmsg;
      if (!repeatedID)
      {
        popmsg.Format ("%s:", msgID);
        csRef<csTimedMessage> tm = csPtr<csTimedMessage> (
          new csTimedMessage (popmsg.GetData ()));
        messages.Push (tm);
      }
      popmsg.Format (" %s", description);
      csRef<csTimedMessage> tm = csPtr<csTimedMessage> (
    	  new csTimedMessage (popmsg.GetData ()));
      messages.Push (tm);
    }
  }
  return msg_remove[severity];
}

bool csReporterListener::HandleEvent (iEvent& event)
{
  if (event.Type == csevBroadcast)
  {
    if (event.Command.Code == cscmdPostProcess)
    {
      csScopedMutexLock lock (mutex);
      size_t l = messages.Length ();
      if (l > 0)
      {
	size_t i;
        csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

        csRef<iGraphics2D> g2d = g3d->GetDriver2D ();
	if (!fnt)
	{
	  iFontServer* fntsvr = g2d->GetFontServer ();
	  if (fntsvr)
	  {
	    fnt = fntsvr->LoadFont (CSFONT_LARGE);
	  }
	}
	if (fnt)
	{
          g3d->BeginDraw (CSDRAW_2DGRAPHICS);
	  int sw = g2d->GetWidth ();
	  int sh = g2d->GetHeight ();
	  int fw, fh;
	  fnt->GetMaxSize (fw, fh);
	  int fg = g2d->FindRGB (0, 0, 0);
          int bg[2] = {g2d->FindRGB (255, 255, 180),
                        g2d->FindRGB (int (255*0.9), int (255*0.9),
			      int (180*0.9))};
          int sep = g2d->FindRGB (int (255*0.7), int (255*0.7),
	      int (180*0.7));

	  size_t max_l = (sh-4-6-4-6) / (fh+6);
	  if (l > max_l) l = max_l;
          int h = 0;
          int c = 0;
	  for (i = 0 ; i < l ; i++)
	  {
	    csTimedMessage* tm = messages[i];
            if (tm->msg[0] != ' ')
            {
              c = 1-c;
              // Assume that the ID fits
              g2d->DrawBox (4, 4+h*(fh+6), sw-8, fh+6, bg[c]);
              g2d->DrawLine (4, 4+h*(fh+6), 4+sw-8-1, 4+h*(fh+6), sep);
              g2d->Write (fnt, 4+6, 4+3+h*(fh+6), fg, bg[c], tm->msg);
            }
	    else
	    {
              csString msg (tm->msg+1);
              int chars;
              csString str;
              str.Format ("  %s", msg.GetData ());
              while ((chars = fnt->GetLength (str.GetData (), sw-20)) - 2 <
                (int) msg.Length ())
              {
                str.Truncate (chars);
                g2d->DrawBox (4, 4+h*(fh+6), sw-8, fh+6, bg[c]);
                int linebreak = strrchr (str.GetData (), ' ')-str.GetData ();
                if (linebreak > 1) // >1 accounts for two leading spaces in str
                {
                  str.Truncate (linebreak);
                  g2d->Write (fnt, 4+6, 4+3+h*(fh+6), fg, bg[c], 
                    str.GetData ());
                  msg = msg.GetData ()+linebreak-1;
                }
		else
		{
                  g2d->Write (fnt, 4+6, 4+3+h*(fh+6), fg, bg[c], 
                    str.GetData ());
                  msg = msg.GetData ()+chars-2;
                }
                str.Format ("  %s", msg.GetData ());
                h++;
                if (l > 0) l--;
              }
              str.Format ("  %s", msg.GetData ());
              g2d->DrawBox (4, 4+h*(fh+6), sw-8, fh+6, bg[c]);
              g2d->Write (fnt, 4+6, 4+3+h*(fh+6), fg, bg[c], str.GetData ());
            }
            h++;
	    // Set the time the first time we could actually display it.
	    if (tm->time == 0) tm->time = csGetTicks () + 5000;
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
	      messages.DeleteIndex (i);
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

void csReporterListener::SetDebugFile (const char* s, bool appendFile )
{
  debug_file = 0;
  debug_filename = s;
  append = appendFile;
}

void csReporterListener::SetDefaults ()
{
  console = CS_QUERY_REGISTRY (object_reg, iConsoleOutput);
  nativewm = 0;
  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
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

const char* csReporterListener::GetDebugFile ()
{
  return debug_filename.GetData();
}
