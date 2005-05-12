
#include "cssysdef.h"
#include "registrar.h"
#include "csutil/snprintf.h"
#include <cstdlib>
#include <cstdio>
#include <ctype.h>

namespace autom
{
	
/////////////////////////////////// Object - Record Allocation with GC ///////////////////////////////////////

object::object(TYPE _otype):otype(_otype) 
{

}
	

object::~object() 
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	

/** Converts the object into a string object if possible. */
string 
string::toString()
{
	return string(value);	
}

/** Converts the object into an integer object, if possible. */
integer 
string::toInt()
{
	return integer(std::strtol(value.c_str(), 0, 10));
}

/** Converts the object into a float object, if possible. */
floating 
string::toFloat()
{
	return floating(std::strtod(value.c_str(), 0));	
}

bool
string::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	value.clear();
	
	if (pos==end || *pos != '"') return false;
	
	++pos;
	for(;pos!=end && *pos!='"'; ++pos)
	{
		if (*pos=='\\')
		{
			++pos;
			switch((*pos))
			{
				case 'n': value+=('\n'); break;	
				case 'r': value+=('\r'); break;	
				case 't': value+=('\t'); break;					
				default: value+=(*pos);
			}	
			
			continue;
		}
		
		value+=(*pos);	
	}
	
	return true;
}


//////////////////// Integer Object //////////////////////////////////////

/** Converts the object into a string object if possible. */
string 
integer::toString()
{
	char buf[128]={0};
		
	return string(std::string(buf, cs_snprintf(buf, sizeof(buf), "%lld", value)));
}

/** Converts the object into an integer object, if possible. */
integer 
integer::toInt()
{
	return integer(value);	
}

/** Converts the object into a float object, if possible. */
floating 
integer::toFloat()
{
	return floating((float)value);	
}

std::string
integer::reprObject()
{
	return toString().Value();
}

bool
integer::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	if (pos==end || !isdigit(*pos)) return false;
	
	std::string temp;
	
	for(; pos!=end && isdigit(*pos); ++pos)	
		temp+=(*pos);			

	#ifdef WIN32
	value = _strtoi64(temp.c_str(), 0, 10);
	#else
	value = strtoll(temp.c_str(), 0, 10);
	#endif
	
	return true;
}

//////////////////// Floating Point Object //////////////////////////////////////

/** Converts the object into a string object if possible. */
string 
floating::toString()
{
	char buf[256];
	cs_snprintf(buf, sizeof(buf), "%g", value);
	
	return string(std::string(buf));
}

/** Converts the object into an integer object, if possible. */
integer 
floating::toInt()
{
	return integer((int)value);	
}

/** Converts the object into a float object, if possible. */
floating 
floating::toFloat()
{
	return floating(value);	
}

std::string
floating::reprObject()
{
	return toString().Value();
}

bool
floating::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	std::string::iterator start = pos;
	
	if (pos==end || !isdigit(*pos)) return false;
	
	std::string temp;
	bool have_radix_point=false;
	
	while(pos!=end && (isdigit(*pos) || (*pos=='.' && have_radix_point==false)))
	{
		if (*pos=='.') have_radix_point=true;
		
		temp+=(*pos);
		++pos;	
	}
	
	if (have_radix_point==false)
	{
		pos=start;
		return false;	
	}
	
	value = std::strtod(temp.c_str(), 0);
	
	return true;
}

//////////////////// List Object //////////////////////////////////////

list::list(std::string &s):object(T_LIST) 
{
	std::string::iterator pos = s.begin();
	
	parseObject(pos, s.end());	
}

/** Converts the object into a string object if possible. */
string 
list::toString()
{
	std::string temp;
	
	temp+=('[');
	
	for(list_type::iterator it=value.begin(); it!=value.end(); ++it)
	{
		keeper k = *it;
		
		switch(k->ObjectType())
		{
			case object::T_STRING:
			{				
				temp+=(k->toString().QuotedValue());				
			} break;
			
			default:
			{				
				temp+=(k->toString().Value());
			}
		}
		
		temp+=(',');		
	}
	
	temp+=(']');
	
	return string(temp);
}

/** Converts the object into an integer object, if possible. */
integer 
list::toInt()
{
	return integer(0);	
}

/** Converts the object into a float object, if possible. */
floating 
list::toFloat()
{
	return floating(0.0);	
}

std::string
list::reprObject()
{
	return toString().Value();
}

bool
list::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{	
	value.clear();
	
	if (pos==end || *pos!='[') return false;
	
	++pos;
	
	for(; pos!=end && *pos!=']'; ++pos)
	{
		// ignore whitespace
		if (isspace(*pos) || *pos==',') continue;
		
		keeper k(Parse(pos, end));	
		
		if (k.IsValid())
		{	
			value.push_back(k);
			(void)k->toInt(); /// Why is this necessary?  I don't know.  I wish it wasn't.  It's definitely wierd.
		}				
		else
			value.push_back(keeper(Nil()));
	}	

	return true;
}


list 
list::operator+=(const keeper &k)
{
	value.push_back(k);
	
	return *this;	
}

keeper 
list::at(long long index)
{
	unsigned int i=(unsigned int)index;
	
	if (i>value.size()) return keeper(Nil());
	else return keeper(value.at(i));	
}

//////////////////// Reference Object //////////////////////////////////////

/** Converts the object into a reference object if possible. */
string
reference::toString()
{
	return (*fn)[value]->toString();
}

/** Converts the object into an integer object, if possible. */
integer 
reference::toInt()
{
	return (*fn)[value]->toInt();
}

/** Converts the object into a float object, if possible. */
floating 
reference::toFloat()
{
	return (*fn)[value]->toFloat();
}

std::string
reference::reprObject()
{
	return value;
}

bool
reference::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	value.clear();
	
	if (pos==end || *pos != '$') return false;
		
	for(; pos!=end && (*pos=='$' || isalnum(*pos)); ++pos)			
		value+=(*pos);		
	
	return true;
}

//////////////////// Nil Object //////////////////////////////////////

/** Converts the object into a string object if possible. */
string 
nil::toString()
{
	return string();
}

/** Converts the object into an integer object, if possible. */
integer 
nil::toInt()
{
	return integer(0);	
}

/** Converts the object into a float object, if possible. */
floating 
nil::toFloat()
{
	return floating(0.0);	
}

std::string
nil::reprObject()
{
	return std::string("nil");
}

bool
nil::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	
	if (pos==end || *pos!='n') return false;
	if (pos==end || *(++pos)!='i') return false;
	if (pos==end || *(++pos)!='l') return false;
	
	++pos;	

	return true;	
}

	
} //end namespace
