#ifndef _CS_LINUX_JOYSTICK_
#define _CS_LINUX_JOYSTICK_
/*
    Copyright (C) 2002 by Norman Krämer
  
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

/**
 * This plugin puts joystick events in the CS eventqueue. 
 * Joystick data is gathered via the linux joystick api.
 */

#include "csutil/scf.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "csutil/cfgacc.h"

class csLinuxJoystick : public iComponent
{
  struct joydata
  {
    int number; // joysticknumber
    int fd; // device descriptor
    int nButtons; // number of buttons
    int nAxes; // number of axis
    int16 *axis; // current values of all axis
    int16 *button; // current values of all buttons

    joydata(){axis=NULL; button=NULL;}
    ~joydata(){delete[]axis; delete[] button;}
  };
    
 protected:
  iObjectRegistry *object_reg;
  joydata *joystick;
  int nJoy; // number of joysticks operable
  csConfigAccess config;
  iEventQueue *eq;
  iEventOutlet *EventOutlet;

  bool Init ();
  bool Close ();
  void Report (int severity, const char* msg, ...);

 public:

  SCF_DECLARE_IBASE;

  csLinuxJoystick (iBase *parent);
  virtual ~csLinuxJoystick ();

  virtual bool Initialize (iObjectRegistry *oreg);
  virtual bool HandleEvent (iEvent &Event);

  struct eiEventPlug : public iEventPlug
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLinuxJoystick);
    virtual unsigned GetPotentiallyConflictingEvents (){ return CSEVTYPE_Joystick;}
    virtual unsigned QueryEventPriority (unsigned ){return 110;}
  } scfiEventPlug;
  friend struct eiEventPlug;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLinuxJoystick);
    virtual bool HandleEvent (iEvent &Event){return scfParent->HandleEvent (Event);}
  } scfiEventHandler;
  friend struct eiEventHandler;

};

#endif
