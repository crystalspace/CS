#include "cssysdef.h"
#include "registry.h"

namespace aws
{

void registry::addChild (const std::string &category, csRef<registry> _child) 
{ 
  child_map_type::iterator pos = children.find(category);

  if (pos==children.end())	
    pos=children.insert(std::make_pair(category, child_list_type())).first; 
  
  pos->second.push_back(_child);		
}

csRef<registry> registry::findChild (const std::string &category, 
				     const std::string &name)
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


bool registry::findValue (const std::string &name, autom::keeper &k)
{
  value_type::iterator pos = reg.find (name);

  if (pos == reg.end()) return false;

  k = pos->second;

  return true;   
}

}
