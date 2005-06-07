#include "cssysdef.h"
#include "registry.h"

namespace aws
{

bool 
registry::findValue(const std::string &name, autom::keeper &k)
{
	value_type::iterator pos=reg.find(name);

	if (pos==reg.end()) return false;

	k=pos->second;

	return true;   
}

}