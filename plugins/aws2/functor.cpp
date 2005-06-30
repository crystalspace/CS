#include "cssysdef.h"
#include "registrar.h"
#include <ctype.h>

namespace autom
{

/** Converts the object into a string object, if possible. */
string 
function::toString()
{	
	if (Called.Valid() || (bind())) rv=Called(*this);
	
	if (rv.IsValid()) return rv->toString();
 	else return string(); 		
}

/** Converts the object into an integer object, if possible. */
integer 
function::toInt()
{
	if (Called.Valid() || (bind())) rv=Called(*this);
	
 	if (rv.IsValid()) return rv->toInt();
 	else return integer(); 	
}

/** Converts the object into a float object, if possible. */
floating 
function::toFloat()
{
	if (Called.Valid() || (bind())) rv=Called(*this);
	
	if (rv.IsValid()) return rv->toFloat();
 	else return floating(); 		
}

/** Binds the function object to a named function. */
bool
function::bind()
{
	// Look it up.
	std::pair<bool, registrar::func_ptr> result = Registrar()->lookup(getName());
	
	// If we found a function, great, otherwise we'll retry the bind next time we're called.
	if (result.first==true)	
	{
		Called.bind(result.second.first, result.second.second);			
		return true;
	}
	
	return false;
}

bool 
function::addParm(const std::string &parm_name, std::string &_value)
{
	std::string::iterator pos=_value.begin();
	
	object *o = ParseParameter(pos, _value.end(), this);
	
	if (o) parms.insert(std::make_pair(parm_name, o));			
	else return false;
	
	return true;
}

std::string
function::reprObject()
{
	std::string rep;
	
	if (repr_exec==false)
	{		
		rep+=(':');
		rep+=(getName());
	 	rep+=('(');
		
		parm_map_type::iterator pos=parms.begin();
		
		for(;pos!=parms.end(); ++pos)
		{
			rep+=(pos->first);
			rep+=('=');
			rep+=(pos->second->reprObject());
			rep+=(',');	
		}	
		
		rep+=(')');
	}
	else
	{
		if (Called.Valid() || (bind())) rv=Called(*this);
		
		if (rv.IsValid()) rep=rv->reprObject();							
	}
	
	return rep;
}

/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
bool 
function::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	// What does a function look like?
	//
	// funcname@object.parent.parent( parm1=value1, parm2="value2", parm3=value3 )
	// 
	// An example is in the translator gateway:
	//
	//	:open@gateway(path="root://My Documents/store.document")
	//
	// A more sophisticated example would be:
	//
	// :open@gateway(path=
	//				:docRoot(profile=
	//						:myProfileId()
	//					   )
	//				)	
	
//	std::string::iterator start=pos;
	
// 	filter::console_output out;
	
// 	out << "parsing function...\n";
	
	if (pos==end || (*pos!=':' && *pos!='%')) return false;
	
	// Using the second kind of call means that we turn on repr exec.
	if (*pos=='%') repr_exec=true; 
	
	++pos;
	
// 	out << "pass ':'\n";
	
	// The fully qualified name of the function
	std::string fname;
	
	// Get the name of the function.
	for(; pos!=end && *pos!='('; ++pos)
	{					
		if (isspace(*pos)) continue;
			
		fname+=(*pos);
	}	
	
	for(; pos!=end && (isspace(*pos) || *pos=='('); ++pos);
	
// 	out << "pass full name: '" << fname << "'\n";
	
	setName(fname);
	
	// Stop here and bind to native code.
	bind();
	
// 	out << "pass bind\n";
	
	// Now pull out the parameters.
	for(; pos!=end && *pos!=')'; ++pos)
	{	
// 		out << "starting parm, char: " << *pos << " pos: " << (void *)pos << " end: " << (void *)end << "\n";
			
		// ignore whitespace
		for(; pos!=end && (isspace(*pos) || *pos==','); ++pos) {}
		
		// If this is the end, end.
		if (*pos==')') break;
		
		// At this point we have at least one parameter, so go ahead and parse it.
				
		std::string parm_name;
		
		for(; pos!=end && *pos!='='; ++pos)
		{	
			// ignore whitespace
			if (isspace(*pos)) continue;
				
			parm_name+=(*pos);
		}
		
// 		out << "pass parm name: '" << parm_name << "'\n";
		
		// Eat trailing whitespace
		for(; pos!=end && (isspace(*pos) || *pos=='='); ++pos) {}
				
		// Otherwise
		object *o = ParseParameter(pos, end, this);
		
		// Insert it into our parameter map.
		if (o) parms.insert(std::make_pair(parm_name, o));		
		else
		{
// 			out << "failed function parse: " << fname << "->" << parm_name << " col: " << (int)(pos-start) << "\n";
			return false; // Unknown object type
		}
		
// 		out << "done with parm, char: " << *pos << "\n";				
		
		// If this is the end, end.
		if (*pos==')') break;
	}	
		
	if (pos!=end) ++pos;
	
// 	out << "done with function parse (" << fname << "), len: " << (int)(pos-start) << " char: " << *pos << "\n";
	
	return true;	
}

function::rc_parm 
function::operator[](const std::string &name)
{
	
	if (name[0]=='$' && parent)
	{		
			std::string temp(name);
			
			return (*parent)[temp.substr(1)];		
	}
	
	parm_map_type::iterator pos = parms.find(name);
	
	if (pos==parms.end())			
		return rc_parm(Nil());
	
				
	return pos->second;
}
	
} //end namespace
