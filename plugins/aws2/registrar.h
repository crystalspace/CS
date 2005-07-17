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

#ifndef _KEILA_FUNCTION_REGISTRY_H__
#define _KEILA_FUNCTION_REGISTRY_H__

#include "functor.h"
#include "split.h"

namespace aws
{

namespace autom
{

	/** The registrar is a manager object, one per process, which keeps track of all the registered
	 * functions.  It also supports containers that essentially serve as namespaces. */
	class registrar
	{	
	private:
		struct container;
		
	public:				
			
		/** The type for function pointers. */
		typedef std::pair<function::slot_ptr, function::slot_mem_ptr> func_ptr;

		/** The type for mapping functions to names. */
		typedef std::map <csString, func_ptr> func_map_type;
			
		
	private:		
		/** The container is called the lobby. */
		func_map_type lobby;		
		
	public:
	
		registrar():lobby() 
		{
		}
		
		~registrar() 
		{		
		}
				
		/** Performs lookup on the function.  All lookups start from the lobby and work deeper. 
		 * The function returns a pair: result, pointer.  If result is true, the pointer is valid,
		 * otherwise the pointer is invalid. */
		std::pair<bool, func_ptr> lookup(const csString& name)
		{	
		   func_map_type::iterator func = lobby.find(name);
			
		   if (func!=lobby.end()) return std::make_pair(true, func->second);
		   else return std::make_pair(false, func_ptr(0,0));			
		}
		
		/** Register a function name with a function object.  It will create all the needed containers
		 * in line to the function.  For example:
		 *
		 *  assign("size@user.profile.usage", fn_ptr)
		 *
		 *  Would create containers user, profile, and usage; and then create size inside the usage container. 
		 */
		void assign(const csString &name, func_ptr func);
	};
	
	
	/** Returns the global registrar instance. */
	registrar *Registrar();
	
	/** Returns the global nil object. */
	nil *Nil();
	
	/** Returns a fully populated object parsed from the given string, or zero on failure. */
	object *Parse(std::string::iterator &pos, const std::string::iterator &end);
	
	/** Returns a fully populated object parsed from the given string, or zero on failure.  Also performs
	 * the additional work of setting parent functions when it finds a function. */
	object *ParseParameter(std::string::iterator &pos, const std::string::iterator &end, function *parent);
	
	/** Returns a garbage-collected pointer that is safe for application usage. */
	keeper Compile(std::string &str);
	
	/** Makes it easier to register a callback function for the automation handler. */
	#define AUTOM_REGISTER(funcname, object_pointer, member_function)   \
	  aws::autom::Registrar()->assign(funcname,			    \
	    std::make_pair(object_pointer,				    \
	    (aws::autom::function::slot_mem_ptr)member_function));

} // namespace autom

} // namespace aws

#endif
