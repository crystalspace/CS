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


/* Contains the built-in functions that a Keila interpreter will always have. */
#include "cssysdef.h"
#include "registrar.h"

namespace autom
{

class int_builtin : public function::slot
{
public:
	int_builtin() {}
	~int_builtin() {}
			
	function::rc_parm bits(function &fn)
	{
		return func_parm(new integer(64));	
	}
	
	func_parm add(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		return func_parm(new integer(left->toInt() + right->toInt()));
	}
	
	func_parm sub(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		return func_parm(new integer(left->toInt() - right->toInt()));
	}
	
	func_parm mul(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		return func_parm(new integer(left->toInt() * right->toInt()));
	}
	
	func_parm div(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		return func_parm(new integer(left->toInt() / right->toInt()));
	}
	
	func_parm mod(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		return func_parm(new integer(left->toInt() % right->toInt()));
	}
	
	
	func_parm equal(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		return func_parm(new integer(left->toInt() == right->toInt()));
	}
	
	func_parm min(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
						
		longlong l = left->toInt().Value();
		longlong r = right->toInt().Value();
											
		return func_parm(new integer((l < r ? l : r)));

	}
	
	func_parm max(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		longlong l = left->toInt().Value();
		longlong r = right->toInt().Value();
											
		return func_parm(new integer((l > r ? l : r)));

	}
	
	func_parm less(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		longlong l = left->toInt().Value();
		longlong r = right->toInt().Value();
											
		return func_parm(new integer(l < r));

	}
	
	func_parm greater(function &fn)
	{
		func_parm left  = fn["l"],
				  right = fn["r"];
										
		longlong l = left->toInt().Value();
		longlong r = right->toInt().Value();
											
		return func_parm(new integer(l < r));

	}

	
	void setup()
	{
		Registrar()->assign("bits@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::bits));	
		Registrar()->assign("add@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::add));
		Registrar()->assign("sub@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::sub));
		Registrar()->assign("mul@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::mul));
		Registrar()->assign("div@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::div));
		Registrar()->assign("mod@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::mod));
		Registrar()->assign("equal@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::equal));
		Registrar()->assign("less@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::less));
		Registrar()->assign("greater@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::greater));		
		Registrar()->assign("min@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::min));
		Registrar()->assign("max@int", std::make_pair(this, (function::slot_mem_ptr)&int_builtin::max));		
	}
	
};

class list_builtin : public function::slot
{
public:
	list_builtin() {}
	~list_builtin() {}
			
	function::rc_parm size_(function &fn)
	{
		func_parm ln = fn["list"];
		
		if (ln->ObjectType()!=object::T_LIST) return func_parm(Nil());
		else
		{
			object *o = ln;
			list *l = CS_STATIC_CAST(list*,o);
			return func_parm(new integer(l->size()));
		}		
	}
	
	function::rc_parm at_(function &fn)
	{
		func_parm ln = fn["list"],
				  index=fn["index"];
		
		if (ln->ObjectType()!=object::T_LIST) return func_parm(Nil());
		else
		{
			object *o = ln;
			list *l = CS_STATIC_CAST(list*,o);
			return func_parm(l->at(index->toInt().Value()));
		}		
	}
	
	void setup()
	{
		Registrar()->assign("size@list", std::make_pair(this, (function::slot_mem_ptr)&list_builtin::size_));	
		Registrar()->assign("at@list", std::make_pair(this, (function::slot_mem_ptr)&list_builtin::at_));	
	}
};

/** A bunch of builtins that give information about keila itself. */
class keila_builtin : public function::slot
{
public:
	keila_builtin() {}
	~keila_builtin() {}
	
	/** Returns the version of keila. */		
	function::rc_parm version_(function &fn)
	{				
		return func_parm(new floating(1.0));		
	}

	/** Registers all of the kernel builtins. */
	void setup()
	{
		Registrar()->assign("version@keila", std::make_pair(this, (function::slot_mem_ptr)&keila_builtin::version_));	
	}		
	
};


/** Global builtins for the lobby class.  These don't belong to any particular object but are more or less 
* considered to be language extensions. */
class lobby_builtin : public function::slot
{
	/** Stores "defined" function objects.  Like shortcuts, macros.  Can also be function definitions. */
	function::parm_map_type def_funcs;
	
public:
	function::rc_parm if_(function &fn)
	{
		func_parm test   = fn["test"],
				  _true  = fn["true"],
				  _false = fn["false"];
				  
		if (test->toInt().Value()) return _true;	
		else return _false;		
	}
	
	function::rc_parm loop_(function &fn)
	{
		func_parm test    = fn["while"],
				  action  = fn["do"];
				  
				  
		if (test->toInt().Value()) return action;	
		else return func_parm(Nil());		
	}
	
	/** Defines a named function in the code.  Connects it to def_exec_, which looks up the function's name
	 * and returns the body as the return value. */
	func_parm def_(function &fn)
	{
		func_parm name    = fn["name"],
				  body    = fn["body"];		
		
		def_funcs[name->toString().Value()] = body;
				  		  
		Registrar()->assign(name->toString().Value(), std::make_pair(this, (function::slot_mem_ptr)&lobby_builtin::def_exec_));

		return func_parm(Nil());
	}
	
	/** Performs the actual execution of the function using some slight of hand to 
	 * redirect the call to the appropriate handler. */
	func_parm def_exec_(function &fn)
	{
		func_parm body = def_funcs[fn.getName()];		

		if (body->ObjectType() == object::T_FUNCTION)
		{
			function *b = CS_STATIC_CAST(function*,(object*)body);
			return func_parm(b->clone(&fn));
		}
		else
			return body;							
	}
		
	void setup()
	{
		Registrar()->assign("if", std::make_pair(this, (function::slot_mem_ptr)&lobby_builtin::if_));	
		Registrar()->assign("loop", std::make_pair(this, (function::slot_mem_ptr)&lobby_builtin::loop_));
		Registrar()->assign("def", std::make_pair(this, (function::slot_mem_ptr)&lobby_builtin::def_));	
	}	
};


/** Installs all the builtin functions. */
void
install_builtin()
{
	int_builtin *int_b = new int_builtin();
	list_builtin *list_b = new list_builtin();
	keila_builtin *keila_b = new keila_builtin();
	lobby_builtin *lobby_b = new lobby_builtin();
		
	
	int_b->setup();	
	list_b->setup();
	keila_b->setup();
	lobby_b->setup();	
}

} // end namespace
