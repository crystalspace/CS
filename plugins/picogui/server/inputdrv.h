/*
    Crystal Space Input Interface for PicoGUI
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_PICOGUI_SERVER_INPUTDRV_H__
#define __CS_PICOGUI_SERVER_INPUTDRV_H__

#include "iutil/eventh.h"
#include "iutil/eventq.h"

/// @@@ Solve this better. "timeval" and "fd_set" is what we're after.
#ifdef WIN32
#include "winsock.h"
#endif

struct inlib;
struct cursor;
class csPicoGUIServer;
struct csPGInputHandler;

/**
 * The static methods of this class make up a PicoGUI input driver.
 * PicoGUI will call them, and they access the CS input interfaces.
 * They may be used as C-style function pointers, for passing to PicoGUI.
 */
class csPGInputDriver
{
 private:
  static csRef<iEventHandler> EvH;
  static csRef<iEventQueue> EvQ;
  static inlib *Inlib;
  static cursor *Cursor;

 protected:
  static bool Construct (iEventQueue *);
  friend class csPicoGUIServer;

  static inline cursor* GetCursor () { return Cursor; }
  friend class csPGInputHandler;

 public:
  static g_error RegFunc (inlib *);
  static g_error Init ();
  static void Close ();
  static void csPGInputDriver::FDInit (int *, fd_set *, timeval *);

};

/**
 * This is the CS event handler for the PicoGUI server input driver.
 */
struct csPGInputHandler : public iEventHandler
{
  SCF_DECLARE_IBASE;
  csPGInputHandler () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~csPGInputHandler () {}
  bool HandleEvent (iEvent &ev);
};

#endif
