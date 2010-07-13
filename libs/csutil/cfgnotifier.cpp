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

#include "cssysdef.h"
#include "csutil/cfgnotifier.h"

#include "csutil/eventhandlers.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/cfgmgr.h"


namespace CS
{
  namespace Utility
  {

    ConfigEventNotifier::ConfigEventNotifier(iObjectRegistry* obj_reg)
      : scfImplementationType (this), obj_reg(obj_reg)
    {
      eventQueue = csQueryRegistry<iEventQueue> (obj_reg);
      nameRegistry = csEventNameRegistry::GetRegistry(obj_reg);

      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg);
      csRef<iConfigNotifier> cn = scfQueryInterface<iConfigNotifier>(app_cfg);
      cn->AddListener(this);
    }

    ConfigEventNotifier::~ConfigEventNotifier()
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg);
      csRef<iConfigNotifier> cn = scfQueryInterface<iConfigNotifier>(app_cfg);
      cn->RemoveListener(this);
    }

    void ConfigEventNotifier::SetStr (const char* key, const char* value)
    {
      printf("ConfigEventNotifier::Set char 1\n");
      if (eventQueue && nameRegistry)
      {
        csRef<iEvent> ev = eventQueue->CreateBroadcastEvent(nameRegistry->GetID(key));
        ev->Add("value", value);
        eventQueue->Post(ev);
      }
    }

    void ConfigEventNotifier::SetInt (const char* key, int value)
    {
      printf("ConfigEventNotifier::Set int 1\n");
      if (eventQueue && nameRegistry)
      {
        printf("ConfigEventNotifier::Set 2\n");
        csRef<iEvent> ev = eventQueue->CreateBroadcastEvent(nameRegistry->GetID(key));
        ev->Add("value", value);
        eventQueue->Post(ev);
      }
    }

    void ConfigEventNotifier::SetFloat (const char* key, float value)
    {
      printf("ConfigEventNotifier::Set float 1\n");
      if (eventQueue && nameRegistry)
      {
        csRef<iEvent> ev = eventQueue->CreateBroadcastEvent(nameRegistry->GetID(key));
        ev->Add("value", value);
        eventQueue->Post(ev);
      }
    }

    void ConfigEventNotifier::SetBool (const char* key, bool value)
    {
      printf("ConfigEventNotifier::Set bool 1\n");
      if (eventQueue && nameRegistry)
      {
        csRef<iEvent> ev = eventQueue->CreateBroadcastEvent(nameRegistry->GetID(key));
        ev->Add("value", value);
        eventQueue->Post(ev);
      }
    }

    void ConfigEventNotifier::SetTuple (const char* key, iStringArray* value)
    {
      printf("ConfigEventNotifier::Set iStringArray 1\n");
      if (eventQueue && nameRegistry)
      {
        csRef<iEvent> ev = eventQueue->CreateBroadcastEvent(nameRegistry->GetID(key));
        ev->Add("value", (iBase*)value);
        eventQueue->Post(ev);
      }
    }
    
  } // namespace Utility
} // namespace CS