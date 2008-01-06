/*
    Copyright (C) 2007 by Ronie Salgado <roniesalg@gmail.com>

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

using System;
using System.Reflection;

namespace CrystalSpace
{
  public abstract class csSharpEventHandler: csSharpEventHandlerWrap
  {
    public csSharpEventHandler ()
    {
      ConnectHandler ();
    }

    // We need to call the event handler even
    // if we aren't using directors, so we implement 
    // director functionality directly
    private void ConnectHandler ()
    {
      Type derivedType = this.GetType ();
      Type baseType = typeof (csSharpEventHandler);
      if (!derivedType.IsSubclassOf (baseType))
	  return;

        System.Reflection.MethodInfo HandleEventInfo =
	derivedType.GetMethod ("HandleEvent", HandleEvent_types);

      if (HandleEventInfo.DeclaringType.IsSubclassOf (baseType))
      {
	m_eventHandler += new corePINVOKE.EventHandler_t (Base_HandleEvent);
	corePINVOKE.ConnectSharpEventHandler (csSharpEventHandlerWrap.
						getCPtr (this).Handle,
						m_eventHandler);
      }
    }

    private static Type[] HandleEvent_types = { typeof (iEvent) };
    private int Base_HandleEvent (IntPtr eventHandler)
    {
      iEvent ev = new iEvent (eventHandler, false);
      bool ret = HandleEvent (ev);
      if (ret)
	return 1;
      return 0;
    }

    private corePINVOKE.EventHandler_t m_eventHandler;
  }
}; // namespace CrystalSpace
