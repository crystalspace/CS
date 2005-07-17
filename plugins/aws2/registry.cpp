/*
    Copyright (C) 2005 by Christopher Nelson

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
#include "registry.h"

namespace aws
{

void registry::addChild (const csString &category, csRef<registry> _child) 
{ 
  child_map_type::iterator pos = children.find(category);

  if (pos==children.end())	
    pos=children.insert(std::make_pair(category, child_list_type())).first; 
  
  pos->second.push_back(_child);		
}

csRef<registry> registry::findChild (const csString &category, 
				     const csString &name)
{
  csRef<registry> reg, empty;

  child_map_type::iterator pos = children.find(category);

  if (pos != children.end())
  {
    for (child_list_type::iterator pos2 = pos->second.begin(); 
      pos2 != pos->second.end(); ++pos2)
    {
      reg = *pos2;
      if (reg->Name() == name)
	return reg;
    }
  }

  return empty;    
}


bool registry::findValue (const csString &name, autom::keeper &k)
{
  value_type::iterator pos = reg.find (name);

  if (pos == reg.end()) return false;

  k = pos->second;

  return true;   
}

}
