/*
Copyright (C) 2006 by Andrew Mann

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

#include "iutil/plugin.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "iutil/cmdline.h"
#include "iutil/cfgfile.h"
#include "csutil/sysfunc.h"
#include "csutil/eventnames.h"
#include "ivaria/reporter.h"

#include "csutil/cfgacc.h"

#include "csutil/csstring.h"

#include "eventrecorder.h"



SCF_IMPLEMENT_FACTORY (csSndSysBasicEventRecorder)


#define QUALIFIED_PLUGIN_NAME "crystalspace.sndsys.utility.eventrecorder"

csSndSysBasicEventRecorder::csSndSysBasicEventRecorder(iBase *piBase) 
  : scfImplementationType(this, piBase)
{
  // Start out true, we'll revert to false if needed
  m_Active=true;
}

csSndSysBasicEventRecorder::~csSndSysBasicEventRecorder()
{
}



bool csSndSysBasicEventRecorder::Initialize(iObjectRegistry *pObjectRegistry)
{
  /// Interface to the Configuration file
  csConfigAccess Config;

  m_pObjectRegistry=pObjectRegistry;

  // Read an extra config file just for this plugin
  Config.AddConfig(m_pObjectRegistry, "/config/eventrecorder.cfg");


  // Check for overriding sound event log from the command line
  csRef<iCommandLineParser> cmdline (
    csQueryRegistry<iCommandLineParser> (m_pObjectRegistry));

  const char *output_location = cmdline->GetOption("soundeventlog");
  if (!output_location)
  {
    // Nothing on the command line, let's try the config file
    output_location = Config->GetStr("SndSys.EventLog", "this/soundevents.log");
  }

  csReport(m_pObjectRegistry, CS_REPORTER_SEVERITY_DEBUG, QUALIFIED_PLUGIN_NAME,
    "Sound system events logging to [%s]", output_location);

  m_pVFS = csQueryRegistry<iVFS> (m_pObjectRegistry);
  if (!m_pVFS)
  {
    csReport(m_pObjectRegistry, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
      "Sound system failed to find VFS plugin!");
    m_Active=false;

    // We did initialize, we just won't record any events
    return true;
  }

  // Copy the output location into a local buffer
  m_LogFileName=output_location;


  // set event callback
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (m_pObjectRegistry));
  m_evSystemOpen = csevSystemOpen(m_pObjectRegistry);
  m_evSystemClose = csevSystemClose(m_pObjectRegistry);
  m_evFrame = csevFrame(m_pObjectRegistry);
  if (q != 0) {
    csEventID subEvents[] = { m_evSystemOpen, m_evSystemClose, m_evFrame, CS_EVENTLIST_END };
    q->RegisterListener(this, subEvents);
  }

  return true;
}

bool csSndSysBasicEventRecorder::HandleEvent (iEvent &e)
{
  if (e.Name == m_evFrame) 
  {
    ProcessEventQueue();
  }
  else if (e.Name == m_evSystemOpen) 
  {
    Open();
  } else if (e.Name == m_evSystemClose) 
  {
    Close();
  }
  return false;
}

bool csSndSysBasicEventRecorder::Open()
{
  // Open the logfile
  m_pLogFile=m_pVFS->Open(m_LogFileName, VFS_FILE_APPEND);
  if (!m_pLogFile.IsValid())
  {
    m_Active=false;
    return false;
  }

  return true;
}

bool csSndSysBasicEventRecorder::Close()
{
  // Lock out the queue so no more entries may be added
  m_EventQueue.SetClosed(true);

  // Close the logfile
  m_pLogFile = 0;
  m_Active = false;
  return true;
}

int csSndSysBasicEventRecorder::ProcessEventQueue()
{
  int EventCount=0;
  SndSysEventEntry *pEntry;
  // Start with a reasonable size so we hopefully wont need to grow this very often
  csStringBase LogString((size_t)1024);

  // Empty the queue
  while ((pEntry=m_EventQueue.DequeueEntry(false)) != 0)
  {
    if (m_Active && m_pLogFile)
    {
      LogString.Format("[%012" PRIu64 "] [%s] [%s] %s\n", pEntry->Time, GetEventLevelString(pEntry->Level),
                       GetEventCategoryString(pEntry->Category), pEntry->Message.GetData());
      m_pLogFile->Write(LogString.GetData(), LogString.Length());
    }
    delete pEntry;
    EventCount++;
  }

  m_pLogFile->Flush();
  return EventCount;
}

void csSndSysBasicEventRecorder::RecordEvent(SndSysEventCategory Category, SndSysEventLevel Level, const char* Description, ...)
{
  if (!m_Active)
    return;

  va_list arg;
  va_start (arg, Description);
  RecordEventV(Category, Level, Description, arg);
  va_end (arg);

}

void csSndSysBasicEventRecorder::RecordEventV(SndSysEventCategory Category, SndSysEventLevel Level, const char* Description, va_list arg)
{
  if (!m_Active)
    return;
  SndSysEventEntry *pEntry=new SndSysEventEntry(Category, Level, Description, arg);

  if (!pEntry)
    return;

  if (m_EventQueue.QueueEntry(pEntry) != QUEUE_SUCCESS)
    delete pEntry;
}


csSndSysBasicEventRecorder::SndSysEventEntry::SndSysEventEntry(SndSysEventCategory Cat, SndSysEventLevel Lev, const char* Desc, va_list Args) :
  Category(Cat), Level(Lev)
{
  Time=csGetMicroTicks();
  Message.AppendFmtV(Desc, Args);
}


const char *csSndSysBasicEventRecorder::GetEventLevelString(SndSysEventLevel Level)
{
  switch (Level)
  {
    case SSEL_BUG:
      return "  BUG  ";
    case SSEL_CRITICAL:
      return "CRITICAL";
    case SSEL_ERROR:
      return "  ERROR ";
    case SSEL_WARNING:
      return " WARNING";
    case SSEL_DEBUG:
      return "  DEBUG ";
    default:
      return " UNKLEV ";
  }
}

/// Retrieve the event category textual description
const char *csSndSysBasicEventRecorder::GetEventCategoryString(SndSysEventCategory Category)
{
  switch (Category)
  {
    case SSEC_DRIVER:
      return " DRIVER ";
    case SSEC_RENDERER:
      return "RENDERER";
    case SSEC_SOURCE:
      return " SOURCE ";
    case SSEC_STREAM:
      return " STREAM ";
    case SSEC_DATA:
      return "  DATA  ";
    default:
      return " UNKCAT ";
  }

}
