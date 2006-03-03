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
#include "script_manager.h"
#include "script_console.h"


/** @brief The prototype object for the skin. */
static JSObject *skin_proto_object=0;
static JSObject *skins_array=0;


/** @brief Adds a skin to the collection. */
static JSBool
Add(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	jsval name;
	
	// Find the name of the skin to add.
	if (JSVAL_IS_OBJECT(argv[0]))
	{
		// Fail if the object doesn't have a "name" property.
		if (JS_LookupProperty(cx, JSVAL_TO_OBJECT(argv[0]), "Name", &name)==JS_FALSE)
		{
			return JS_FALSE;
		}
		
		if (!JSVAL_IS_STRING(name))
		{
			return JS_FALSE;	
		}
	}
		
	// First store the skin's name into the array
	jsuint length=0;
	
	JS_GetArrayLength(cx, skins_array, &length); 
	JS_SetElement(cx, skins_array, length, &name);	
	
	// Next, associate the skin with the object container for quick access.	
	JS_SetProperty(cx, obj, JS_GetStringBytes(JSVAL_TO_STRING(name)), &argv[0]);
	
	return JS_TRUE;
}

/** @brief Resolves a Color GetProperty. */
// static JSBool
// color_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
// {
// 	csColor4 *co = (csColor4 *)JS_GetPrivate(cx, obj);
//     
// 	if (JSVAL_IS_INT(id)) 
//   	{
//     	switch (JSVAL_TO_INT(id)) 
//     	{
// 	    	case COLOR_R: *vp = DOUBLE_TO_JSVAL((jsdouble)co->red); break;
// 	    	case COLOR_G: *vp = DOUBLE_TO_JSVAL((jsdouble)co->green); break;
// 	    	case COLOR_B: *vp = DOUBLE_TO_JSVAL((jsdouble)co->blue); break;
// 	    	case COLOR_ALPHA: *vp = DOUBLE_TO_JSVAL((jsdouble)co->alpha); break;
// 	    	default:
// 	    		return JS_FALSE;
//     	}
//     	
//     	return JS_TRUE;
// 	}
// 	
//     return JS_FALSE;
// }

/** @brief Resolves a Color SetProperty call. */
// static JSBool
// color_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
// {
// 	csColor4 *co = (csColor4 *)JS_GetPrivate(cx, obj);    
// 	jsdouble c;
// 	
// 	if (JSVAL_IS_INT(id)) 
//   	{
// 	  	JS_ValueToNumber(cx, *vp, &c);
// 	  	
//     	switch (JSVAL_TO_INT(id)) 
//     	{
// 	    	case COLOR_R: co->red = (float)c; break;
// 	    	case COLOR_G: co->green = (float)c; break;
// 	    	case COLOR_B: co->blue = (float)c; break;
// 	    	case COLOR_ALPHA: co->alpha = (float)c; break;
// 	    	default:
// 	    		return JS_FALSE;
//     	}
//     	
//     	return JS_TRUE;
// 	}
// 	
//     return JS_FALSE;
// }

JSClass skin_object_class = {
    "Skin", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,JS_PropertyStub,
    JS_PropertyStub,JS_PropertyStub,
    //color_getProperty,color_setProperty,
    JS_EnumerateStub,JS_ResolveStub,
    JS_ConvertStub,JS_FinalizeStub 
};


// static JSPropertySpec skin_props[] =
// {
//         {"r",       COLOR_R,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"g",       COLOR_G,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"b",      	COLOR_B,   	   JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"alpha",   COLOR_ALPHA,   JSPROP_ENUMERATE | JSPROP_PERMANENT},        
//         {0,0,0}
// };

static JSFunctionSpec skin_static_methods[] = {
    {"Add",		Add,		1, 0, 0},    
    {0,0,0,0,0}
};    

#define CHECK(objname, check) if (!(check)) { \
	    msg.Format(objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
		ScriptCon()->Message(msg); \
	}

void 
Skin_SetupAutomation()
{
	if (skin_proto_object==0)
	{
		csString msg;
						
	    JSObject *skin;
		
		CHECK("Skin", skin = JS_DefineObject(ScriptMgr()->GetContext(),
								    		 ScriptMgr()->GetGlobalObject(), 
								    		 "Skin", &skin_object_class, NULL, 0));
		
		if (skin)
		{		
			jsval null = JSVAL_NULL;	
			
			// Populate the skin object with functions.
			CHECK("Skin", JS_DefineFunctions(ScriptMgr()->GetContext(), 
											 skin, skin_static_methods)==JS_TRUE);		
			// Create the "default" property.
			CHECK("Skin", JS_SetProperty(ScriptMgr()->GetContext(), skin, "current", &null)==JS_TRUE);
											 
			// Create the skins array.								 
			CHECK("Skin", skins_array = JS_NewArrayObject(ScriptMgr()->GetContext(), 0, NULL));
			
			// Attach it to the skin object.
			if (skins_array)
			{
				jsval tmp = OBJECT_TO_JSVAL(skins_array);
				CHECK("Skin", JS_SetProperty(ScriptMgr()->GetContext(), skin, "skins", &tmp)==JS_TRUE);
			}	
		}
		                            
		skin_proto_object = skin;
		                            
		ScriptCon()->Message("Skin builtin-object initialized.");
	 }
}

// newline
