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

#ifndef CS_SNDSYS_EVENTRECORDER_H
#define CS_SNDSYS_EVENTRECORDER_H

struct iSndSysEventRecorder;


typedef enum
{
  SSEL_BUG,      // This event is surely a bug in our code - think of it like a loggable assertion
  SSEL_CRITICAL, // This event is so important you must absolutely know about it.  It may or may not be a bug in our code.
  SSEL_ERROR,    // Something bad happened for sure, but we can deal with it
  SSEL_WARNING,  // Something bad _might_ be happening
  SSEL_DEBUG     // General information that's really good to know for debugging purposes
} SndSysEventLevel;

typedef enum
{
  SSEC_DRIVER, // Event is related to the driver
  SSEC_RENDERER, // Event is related to the renderer, but not particularly a source or stream
  SSEC_SOURCE,   // Event is related to a particular source
  SSEC_STREAM,   // Event is related to a particular stream element
  SSEC_DATA      // Event is relayed to a data element
} SndSysEventCategory;





/// An interface designed for the debugging needs of the sound system
//
//   Important events in the sound system happen many times per second.  Logging or reporting such a massive amount
//   of data can be unweildy (and annoying if you're not debugging the sound system).
//
//   Additionally, sound events are often time critical - just knowing that a buffer was filled after some other action
//   is frequently not enough.  A frequent requirement is to know the exact time between two events - as precise as possible.
//
//   This interface provides these services for the sound system.
struct iSndSysEventRecorder : public virtual iBase
{
  /// SCF2006 - See http://www.crystalspace3d.org/cseps/csep-0010.html
  SCF_INTERFACE (iSndSysEventRecorder, 1, 0, 0);

  /// Log an event with typical variable argument format.
  virtual void RecordEvent(SndSysEventCategory, SndSysEventLevel, const char* Description, ...) CS_GNUC_PRINTF(4, 5) = 0;

  /// Log an event with va_list argument passing - useful if you have a logging wrapper function.
  virtual void RecordEventV(SndSysEventCategory, SndSysEventLevel, const char* Description, va_list) CS_GNUC_PRINTF(4, 0) = 0;

};


#endif // #ifndef CS_SNDSYS_EVENTRECORDER_H


