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


#ifndef SNDSYS_UTILITY_EVENTRECORDER_H
#define SNDSYS_UTILITY_EVENTRECORDER_H

#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "cstypes.h"

#include "csplugincommon/sndsys/queue.h"
#include "isndsys/ss_eventrecorder.h"

using namespace CS::SndSys;

class csSndSysBasicEventRecorder : 
  public scfImplementation3<csSndSysBasicEventRecorder,
                            iSndSysEventRecorder,
                            iEventHandler,
                            iComponent>
{
  public:   
  
    csSndSysBasicEventRecorder(iBase *piBase);
    virtual ~csSndSysBasicEventRecorder();


    /// Log an event with typical variable argument format.
    void RecordEvent(SndSysEventCategory, SndSysEventLevel, const char* Description, ...) CS_GNUC_PRINTF(4, 5);

    /// Log an event with va_list argument passing - useful if you have a logging wrapper function.
    void RecordEventV(SndSysEventCategory, SndSysEventLevel, const char* Description, va_list) CS_GNUC_PRINTF(4, 0);

protected:

  class SndSysEventEntry
  {
    public:
      SndSysEventEntry(SndSysEventCategory, SndSysEventLevel, const char* Description, va_list);
      ~SndSysEventEntry() {};

    public:
      /// The time of this event, currently in microseconds since program start (approx)
      int64 Time; 
      /// The Category of this event - see SSEC_* enumeration members in ss_eventrecorder.h
      SndSysEventCategory Category;
      /// The Severity Level of this event - see SSEC_* enumeration members in ss_eventrecorder.h
      SndSysEventLevel Level;
      /// The event message itself
      csStringBase Message;
  };

  /// The thread-safe queue for events
  Queue<SndSysEventEntry> m_EventQueue;

  // The object registry
  iObjectRegistry *m_pObjectRegistry;

  /// ID of the 'Open' event fired on system startup
  csEventID m_evSystemOpen;

  /// ID of the 'Close' event fired on system shutdown
  csEventID m_evSystemClose;

  /// ID of the 'Frame' event fired once each frame
  csEventID m_evFrame;

  /// Pointer to the Virtual File System interface used to work with our log file
  csRef<iVFS> m_pVFS;

  /// The name of our logfile
  csStringBase m_LogFileName;

  /// Our logfile interface
  csRef<iFile> m_pLogFile;

  /// This can be set to false if the output channel is unavailable.  In that case events will be silently dropped.
  bool m_Active;

protected:
  /// Called from the event handler on SystemOpen event
  bool Open();

  /// Called from the event handler on SystemClose event
  bool Close();

  /// Called from the event handler on Frame event to write queued events to the log
  int ProcessEventQueue();

  /// Retrieve the event level textual description
  static const char *GetEventLevelString(SndSysEventLevel Level);

  /// Retrieve the event category textual description
  static const char *GetEventCategoryString(SndSysEventCategory Category);

public:
  ////
  //
  // Interface implementation
  //
  ////

  // iComponent
  virtual bool Initialize (iObjectRegistry *obj_reg);

  // iEventHandler
  virtual bool HandleEvent (iEvent &e);
  CS_EVENTHANDLER_NAMES("crystalspace.sndsys.utility.eventrecorder")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

};





#endif //  #ifndef SNDSYS_UTILITY_EVENTRECORDER_H
