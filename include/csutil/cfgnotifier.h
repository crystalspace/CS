/*
    Copyright (C) 2010 by Jelle Hellemans

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

#ifndef __CS_CSUTIL_CSCONFIGEVENTNOTIFIER_H__
#define __CS_CSUTIL_CSCONFIGEVENTNOTIFIER_H__

#include "csextern.h"
#include "csutil/ref.h"
#include "csutil/scf_implementation.h"

#include "iutil/eventh.h"
#include "csutil/cseventq.h"
#include "iutil/event.h"

#include "iutil/cfgnotifier.h"

struct iObjectRegistry;
struct iEventQueue;
struct iEventNameRegistry;

namespace CS
{
  namespace Utility
  {
    class CS_CRYSTALSPACE_EXPORT ConfigEventNotifier :
      public scfImplementation1<ConfigEventNotifier, iConfigListener>
    {
    public:
      ConfigEventNotifier(iObjectRegistry* obj_reg);
      ~ConfigEventNotifier();

      /// Called when a null-terminated string value has been set.
      virtual void Set (const char* key, const char* value);
      /// Called when an integer value has been set.
      virtual void Set (const char* key, int value);
      /// Called when a floating-point value has been set.
      virtual void Set (const char* key, float value);
      /// Called when a boolean value has been set.
      virtual void Set (const char* key, bool value);
      /// Called when a tuple value has been set.
      virtual void Set (const char* key, iStringArray* value);

    private:
      iObjectRegistry* obj_reg;
      /// The queue of events waiting to be handled.
      csRef<iEventQueue> eventQueue;
      /// The event name registry, used to convert event names to IDs and back.
      csRef<iEventNameRegistry> nameRegistry;
    };

    class CS_CRYSTALSPACE_EXPORT ConfigListenerBase :
      public scfImplementation1<ConfigListenerBase, iEventHandler>
    {
    public:
      ConfigListenerBase (iObjectRegistry* obj_reg, const char* key);
      ~ConfigListenerBase ();

    private:
      iObjectRegistry* obj_reg;
      /// The queue of events waiting to be handled.
      csRef<iEventQueue> eventQueue;
      /// The event name registry, used to convert event names to IDs and back.
      csRef<iEventNameRegistry> nameRegistry;

      CS_EVENTHANDLER_NAMES ("crystalspace.ConfigListener")
      CS_EVENTHANDLER_NIL_CONSTRAINTS
    };
    
    template<typename T>
    class ConfigListener : public scfImplementationExt0<ConfigListener<T>, ConfigListenerBase>
    {
    public:
      ConfigListener(iObjectRegistry* obj_reg, const char* key, T& val)
        : scfImplementationExt0<ConfigListener<T>, ConfigListenerBase> (this, obj_reg, key),
	  value(val)
      {
      }

    private:
      T& value;

      virtual bool HandleEvent(iEvent& ev)
      {
        if (ev.Retrieve("value", value) != csEventErrNone)
          printf("E: ConfigListener::HandleEvent failed!\n");
        return true;
      }
    };

    template<>
    class ConfigListener<csString> : public scfImplementationExt0<ConfigListener<csString>, ConfigListenerBase>
    {
    public:
      ConfigListener(iObjectRegistry* obj_reg, const char* key, csString& val)
        : scfImplementationExt0<ConfigListener<csString>, ConfigListenerBase> (this, obj_reg, key),
	  value (val)
      {
      }

    private:
      csString& value;

      virtual bool HandleEvent(iEvent& ev)
      {
	const char* tmp;
	if (ev.Retrieve("value", tmp) != csEventErrNone)
	  printf("E: ConfigListener<csString>::HandleEvent failed!\n");
	value = tmp;
	return true;
      }
    };

  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_CSCONFIGEVENTNOTIFIER_H__
