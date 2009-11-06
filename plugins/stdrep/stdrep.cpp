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

#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/eventhandlers.h"
#include "csutil/scf.h"
#include "csutil/stringarray.h"
#include "csutil/sysfunc.h"
#include "csutil/threadmanager.h"
#include "csutil/util.h"

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

#include "stdrep.h"



CS_PLUGIN_NAMESPACE_BEGIN(StdRep)
{

SCF_IMPLEMENT_FACTORY (csReporterListener)


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

csReporterListener::csReporterListener (iBase *iParent) : 
scfImplementationType (this, iParent)
{
  object_reg = 0;
  reporter = 0;
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
  csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
  if (rep)
  {
    if (rep == reporter)
      reporter->RemoveReporterListener (this);
  }

  if (eventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      q->RemoveListener (eventHandler);
  }
}

bool csReporterListener::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  SetDefaults ();

  Frame = csevFrame (object_reg);
  if (!eventHandler)
  {
    eventHandler.AttachNew (new EventHandler (this));
  }
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
    q->RegisterListener (eventHandler, Frame);

  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (r);
  if (cfg)
  {
    append = cfg->GetBool ("Reporter.FileAppend", false);
    silent = cfg->GetBool ("Reporter.Silent", silent);
  }

  csRef<iCommandLineParser> cmdline = 
    csQueryRegistry<iCommandLineParser> (object_reg);
  if (cmdline)
  {
    if (cmdline->GetOption ("silent"))
      silent = true;
    if (cmdline->GetOption ("append"))
      append = true;
  }
  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (object_reg));
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
  CS_ANSI_BK CS_ANSI_FM CS_ANSI_TEXT_BOLD_ON,
  CS_ANSI_BK CS_ANSI_FR CS_ANSI_TEXT_BOLD_ON,
  CS_ANSI_BK CS_ANSI_FY CS_ANSI_TEXT_BOLD_ON,
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

void csReporterListener::WriteLine (int severity, const char* msgID, 
  const char* line)
{
  bool repeatedID = false;
  if (lastID.Compare (msgID))
    repeatedID = true;
  else
    lastID = msgID;

  csString msg;
  if (show_msgid[severity])
    msg.Format("%s:  %s\n", msgID, line);
  else
    msg.Format("%s\n", line);

  if (dest_stdout[severity])
  {
    if (!repeatedID)
    {
      stdoutTmp << '\n' << msgID << ":\n";
    }
    stdoutTmp << consolePrefix[severity];
    int offset = 0;
    while (strlen(line+offset)>77)
    {
      csString str (line+offset);
      str.Truncate (77);
      int linebreak = 0;
      const char * space = strrchr (str.GetData (), ' ');
      if (space)
      {
        linebreak = space-str.GetData ();
      }
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
    stdoutTmp << "  " << line+offset << '\n';
    stdoutTmp << consoleSuffix[severity];
    csPrintf ("%s", stdoutTmp.GetData());
    stdoutTmp.Truncate (0);
  }

  if (dest_stderr[severity])
    csPrintfErr ("%s%s%s", consolePrefix[severity], msg.GetData(), 
    consoleSuffix[severity]);

  if (dest_console[severity] && console)
    console->PutText ("%s", msg.GetData());

  if (dest_debug[severity] && !debug_filename.IsEmpty())
  {
    if (!debug_file.IsValid())
    {
      csRef<iVFS> vfs (csQueryRegistry<iVFS> (object_reg));
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
      debug_file->Write (msg.GetData(), msg.Length ());
      debug_file->Flush ();
    }
  }

  if (dest_popup[severity])
  {
    if (!silent)
    {
      CS::Threading::RecursiveMutexScopedLock lock (mutex);
      csString popmsg;
      if (!repeatedID)
      {
        popmsg.Format ("%s:", msgID);
        csRef<csTimedMessage> tm = csPtr<csTimedMessage> (
          new csTimedMessage (popmsg.GetData ()));
        messages.Push (tm);
      }
      popmsg.Format (" %s", line);
      csRef<csTimedMessage> tm = csPtr<csTimedMessage> (
        new csTimedMessage (popmsg.GetData ()));
      messages.Push (tm);
    }
  }
}

THREADED_CALLABLE_IMPL4(csReporterListener, Report, iReporter*, int severity,
  const char* msgID, const char* description)
{
  if (dest_alert[severity] && nativewm)
  {
    csString msg;
    if (show_msgid[severity])
      msg.Format("%s:  %s\n", msgID, description);
    else
      msg.Format("%s\n", description);
    nativewm->Alert (CS_ALERT_ERROR, "Fatal Error!", "Ok", "%s",
      msg.GetData());
  }

  csStringArray lines;
  size_t n = lines.SplitString (description, "\r\n",
      csStringArray::delimIgnoreDifferent);
  for (size_t i = 0; i < n; i++)
    WriteLine (severity, msgID, lines[i] ? lines[i] : "");
  return msg_remove[severity];
}

bool csReporterListener::HandleEvent (iEvent& event)
{
  if (event.Name == Frame)
  {
    CS::Threading::RecursiveMutexScopedLock lock (mutex);
    size_t l = messages.GetSize ();
    if (l > 0)
    {
      size_t i;
      csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (object_reg);
      if(!g3d) return false;

      csRef<iGraphics2D> g2d = g3d->GetDriver2D ();
      if(!g2d) return false;
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
              int linebreak = 0;
              const char * space = strrchr (str.GetData (), ' ');
              if (space)
              {
                linebreak = space-str.GetData ();
              }
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
    csReporterListener::reporter->RemoveReporterListener (this);
  csReporterListener::reporter = reporter;
  if (reporter) reporter->AddReporterListener (this);
}

void csReporterListener::SetDebugFile (const char* s, bool appendFile )
{
  debug_file = 0;
  debug_filename = s;
  append = appendFile;
}

void csReporterListener::SetDefaults ()
{
  console = csQueryRegistry<iConsoleOutput> (object_reg);
  nativewm = 0;
  csRef<iGraphics3D> g3d (csQueryRegistry<iGraphics3D> (object_reg));
  if (g3d)
  {
    iGraphics2D* g2d = g3d->GetDriver2D ();
    if (g2d)
    {
      nativewm = scfQueryInterface<iNativeWindowManager> (g2d);
    }
  }
  if (reporter)
  {
    reporter->RemoveReporterListener (this);
  }
  csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
  reporter = rep;
  if (reporter)
  {
    reporter->AddReporterListener (this);
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

void csReporterListener::SetStandardOutput (int severity, bool en)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_stdout[severity] = en;
}

bool csReporterListener::IsStandardOutput (int severity)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  return dest_stdout[severity];
}

void csReporterListener::SetStandardError (int severity, bool en)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_stderr[severity] = en;
}

bool csReporterListener::IsStandardError (int severity)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  return dest_stderr[severity];
}

void csReporterListener::SetConsoleOutput (int severity, bool en)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_console[severity] = en;
}

bool csReporterListener::IsConsoleOutput (int severity)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  return dest_console[severity];
}

void csReporterListener::SetAlertOutput (int severity, bool en)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_alert[severity] = en;
}

bool csReporterListener::IsAlertOutput (int severity)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  return dest_alert[severity];
}

void csReporterListener::SetDebugOutput (int severity, bool en)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_debug[severity] = en;
}

bool csReporterListener::IsDebugOutput (int severity)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  return dest_debug[severity];
}

void csReporterListener::SetPopupOutput (int severity, bool en)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  dest_popup[severity] = en;
}

bool csReporterListener::IsPopupOutput (int severity)
{
  CS_ASSERT (severity >= 0 && severity <= 4);
  return dest_popup[severity];
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

}
CS_PLUGIN_NAMESPACE_END(StdRep)
