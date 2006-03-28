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
#include "csgfx/gradient.h"
#include "script_manager.h"
#include "script_console.h"
#include "color.h"
#include "gradient.h"


#define CHECK(objname, check) if (!(check)) { \
	    msg.Format(objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
		ScriptCon()->Message(msg); \
	}


/** @brief The prototype object for gradients. */
static JSObject *gradient_proto_object=0;


/** @brief The constructor for gradient objects. */
static JSBool
Gradient(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	csString msg;
	csGradient *go = new csGradient();
		
	// Store this widget object with the new widget instance.
  	CHECK("Gradient", JS_SetPrivate(cx, obj, (void *)go)==JS_TRUE);
  	 	
	
	return JS_TRUE;
}


/** @brief Sets a gradient. Any or all parameters may be included. */
static JSBool
AddColor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	csGradient *go = (csGradient *)JS_GetPrivate(cx, obj);
			
	if (JS_TypeOfValue(cx, argv[0])!=JSTYPE_VOID)
	{
		JSObject *o = JSVAL_TO_OBJECT(argv[0]);		
		
		if (IsColorObject(o))
		{
			csColor4 *co = (csColor4 *)JS_GetPrivate(cx, o);
			jsdouble pos;
			
			JS_ValueToNumber(cx, argv[1], &pos);			
			
			go->AddShade(csGradientShade(*co, (float)pos)); 
		}
		else
		{
			return JS_FALSE;	
		}
	}
	
	return JS_TRUE;
}


JSClass gradient_object_class = {
    "Gradient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,JS_PropertyStub,
    JS_PropertyStub,JS_PropertyStub,
    JS_EnumerateStub,JS_ResolveStub,
    JS_ConvertStub,JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS 
};


static JSFunctionSpec gradient_methods[] = {
    {"AddColor",		AddColor,		2, 0, 0},    
    {0,0,0,0,0}
};    


bool 
IsGradientObject(JSObject *obj)
{
	return JS_InstanceOf(ScriptMgr()->GetContext(), obj, &gradient_object_class, NULL) == JS_TRUE;
}


void 
Gradient_SetupAutomation()
{
	if (gradient_proto_object==0)
	{
		csString msg;
						
	    CHECK("Gradient", gradient_proto_object = 
					   JS_InitClass(ScriptMgr()->GetContext(),
								    ScriptMgr()->GetGlobalObject(), 
									NULL /* no parent */, &gradient_object_class,

		                            /* native constructor function and min arg count */
		                            Gradient, 4,
		
		                            /* prototype object properties and methods -- these
		                               will be "inherited" by all instances through
		                               delegation up the instance's prototype link. */
		                            NULL, gradient_methods,
		
		                            /* class constructor (static) properties and methods */
		                            NULL, NULL));    
		                            
		                            
		 ScriptCon()->Message("Gradient builtin-object initialized.");
	 }
}

// newline
