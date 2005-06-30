#ifndef _KEILA_FUNCTION_REGISTRY_H__
#define _KEILA_FUNCTION_REGISTRY_H__

#include "functor.h"
#include "split.h"

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
				
		/** The type for container pointers. */
		typedef container * cont_ptr;
		
	private:
		/** A container holds both functions and other containers. */
		struct container
		{
			/** The type for mapping functions to names. */
			typedef std::map <std::string, func_ptr> func_map_type;
			
			/** The type for mapping containers to names. */
			typedef std::map <std::string, cont_ptr> cont_map_type;
			
			/** The map of names to functions. */
			func_map_type func_map;
			
			/** The map of names to containers. */
			cont_map_type cont_map;			
		};
		
		/** The root container is called the lobby. */
		cont_ptr lobby;		
		
	public:
	
		registrar():lobby(new container()) 
		{
		}
		
		~registrar() 
		{		
		}
				
		/** Performs lookup on the function.  All lookups start from the lobby and work deeper. 
		 * The function returns a pair: result, pointer.  If result is true, the pointer is valid,
		 * otherwise the pointer is invalid. */
		std::pair<bool, func_ptr> lookup(const std::string& name)
		{			
			std::vector<std::string> parts;
			
			cont_ptr cont=lobby;
			
			if (std::split(const_cast<std::string&>(name), '@', parts)>1)
			{
				std::vector<std::string> cont_names;
				
				if (std::split(parts[1], '.', cont_names))
				{					
					// Find the container, but do not create it. It MUST exist.
					for(std::vector<std::string>::iterator pos=cont_names.begin(); pos!=cont_names.end(); ++pos)				
					{
						container::cont_map_type::iterator tmp = cont->cont_map.find(*pos);
						
						if (tmp!=cont->cont_map.end()) cont=tmp->second;
						else return std::make_pair(false, func_ptr(0,0));
					}
				}					
				
			}			
			
			container::func_map_type::iterator func=cont->func_map.find(parts[0]);
			
			if (func!=cont->func_map.end()) return std::make_pair(true, func->second);
			else return std::make_pair(false, func_ptr(0,0));			
		}
		
		/** Register a function name with a function object.  It will create all the needed containers
		 * in line to the function.  For example:
		 *
		 *  assign("size@user.profile.usage", fn_ptr)
		 *
		 *  Would create containers user, profile, and usage; and then create size inside the usage container. 
		 */
		void assign(const std::string &name, func_ptr func);
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
	#define AUTOM_REGISTER(funcname, object_pointer, member_function) autom::Registrar()->assign(funcname, std::make_pair(object_pointer, (autom::function::slot_mem_ptr)member_function));

} //end namespace

#endif
