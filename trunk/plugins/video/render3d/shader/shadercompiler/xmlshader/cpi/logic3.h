/*
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_LOGIC3_H__
#define __CS_LOGIC3_H__

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  /**
   * Three-state logic: False, True, Indeterminate.
   * Used to convey the information whether something is definitely
   * true, definitely false, or that neither can be determined from
   * the information available.
   */
  class Logic3
  {
  public:
    enum State { Lie, Truth, Uncertain };
     
    State state;
    Logic3 () : state (Uncertain) {}
    Logic3 (State state) : state (state) {}
    Logic3 (bool b) : state (b ? Truth : Lie) {}
    
    const char* ToString() const
    {
      switch (state)
      {
        case Lie: return "Lie";
        case Truth: return "Truth";
        case Uncertain: return "Uncertain";
      }
      return "WTF?";
    }

    Logic3 operator! ()
    {
      switch (state)
      {
      case Lie:
        return Logic3 (Truth);
      case Truth:
        return Logic3 (Lie);
      default:
        return Logic3 (Uncertain);
      }
    }
    Logic3& operator= (const Logic3 other)
    {
      state = other.state;
      return *this;
    }
    friend Logic3 operator&& (const Logic3& a, const Logic3& b)
    {
      if ((a.state == Lie) || (b.state == Lie))
        return Lie;
      else if ((a.state == Truth) && (b.state == Truth))
        return Truth;
      return Uncertain;
    }
    friend Logic3 operator|| (const Logic3& a, const Logic3& b)
    {
      if ((a.state == Truth) || (b.state == Truth))
        return Truth;
      else if ((a.state == Lie) && (b.state == Lie))
        return Lie;
      return Uncertain;
    }
    friend Logic3 operator== (const Logic3& a, const Logic3& b)
    {
      if (a.state == Lie)
      {
        if (b.state == Lie)
          return Truth;
        else if (b.state == Truth)
          return Lie;
        else
          return Uncertain;
      }
      else if (a.state == Truth)
      {
        if (b.state == Truth)
          return Truth;
        else if (b.state == Lie)
          return Lie;
        else
          return Uncertain;
      }
      else
        return Uncertain;
    }
    friend Logic3 operator!= (const Logic3& a, const Logic3& b)
    { return !operator== (a, b); }
  };

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_LOGIC3_H__
