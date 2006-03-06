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
#include "font.h"
#include "manager.h"


#define CHECK(objname, check) if (!(check)) { \
	    msg.Format(objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
		ScriptCon()->Message(msg); \
	}


/** @brief The prototype object for fonts. */
static JSObject *font_proto_object=0;

/** @brief The constructor for color objects. */
static JSBool
Font(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	csString msg;
	
	if (JSVAL_IS_OBJECT(argv[0]))
	{
		JSObject *font_object = JSVAL_TO_OBJECT(argv[0]);
		
		 if (IsFontObject(font_object))
		 {		
			// Copy constructor.	
			csRef<iFont> *fo = (csRef<iFont> *)JS_GetPrivate(cx, font_object);		
			
			// Store this widget object with the new widget instance.
	  		CHECK("Font", JS_SetPrivate(cx, obj, (void *)fo)==JS_TRUE);  	
  		}
	}
	else
	{
		csString font_name;
		csString ms;
		
		jsdouble size;
		JSString *name = JS_ValueToString(cx, argv[0]);
				
		font_name = JS_GetStringBytes(name);
		
		JS_ValueToNumber(cx, argv[1], &size);
				
		csRef<iFont> *fo = new csRef<iFont>(AwsMgr()->G2D()->GetFontServer()->LoadFont(font_name.GetData(), (float)size));
		
		if (!(fo->IsValid()))
		{
			msg.Format("Font: Could not load '%s'!", font_name.GetDataSafe());
			ScriptCon()->Message(msg);	
			
			return JS_FALSE;
		}		
		
		// Store this widget object with the new widget instance.
	  	CHECK("Font", JS_SetPrivate(cx, obj, (void *)fo)==JS_TRUE);  		
	}			
	  	  
	
	return JS_TRUE;
}


/** @brief Gets the baseline to baseline height of the font. */
static JSBool
GetTextHeight(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	csRef<iFont> *fo = (csRef<iFont> *)JS_GetPrivate(cx, obj);

	if (fo)	*rval = INT_TO_JSVAL((*fo)->GetTextHeight());
	
	return JS_TRUE;
}

/** @brief Determine how many characters from this string can be written without exceeding given width (in pixels).  */
static JSBool
GetLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	csRef<iFont> *fo = (csRef<iFont> *)JS_GetPrivate(cx, obj);		
	
	if (fo)	
	{
		int32 maxlength;
		JSString *name = JS_ValueToString(cx, argv[0]);
		
		JS_ValueToInt32(cx, argv[1], &maxlength);
		
		*rval = INT_TO_JSVAL((*fo)->GetLength(JS_GetStringBytes(name), maxlength));
	}
	
	return JS_TRUE;
}

/** @brief Used to return the dimensions a string of text in some font. */
JSClass font_dimensions_object_class = {
    "FontDimensions", 0,
    JS_PropertyStub,JS_PropertyStub,
    JS_PropertyStub,JS_PropertyStub,
    JS_EnumerateStub,JS_ResolveStub,
    JS_ConvertStub,JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS 
};

/** @brief Return the width, height, and descender of text written with this font.   */
static JSBool
GetDimensions(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	csRef<iFont> *fo = (csRef<iFont> *)JS_GetPrivate(cx, obj);		
	
	if (fo)	
	{		
		int w,h,desc;
		jsval fw, fh, fdesc;
		
		JSString *name = JS_ValueToString(cx, argv[0]);
		
		(*fo)->GetDimensions(JS_GetStringBytes(name), w, h, desc);
		
		JSObject *rv = JS_NewObject(cx, &font_dimensions_object_class, NULL, NULL);
		
		fw = INT_TO_JSVAL(w);
		fh = INT_TO_JSVAL(h);
		fdesc = INT_TO_JSVAL(desc);		
		
		JS_SetProperty(cx, rv, "width", &fw);
		JS_SetProperty(cx, rv, "height", &fh);
		JS_SetProperty(cx, rv, "descender", &fdesc);
		
		*rval = OBJECT_TO_JSVAL(rv);
	}
	
	return JS_TRUE;
}

/** @brief The constructor for color objects. */
static JSBool
Load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	return Font(cx, obj, argc, argv, rval);
}


JSClass font_object_class = {
    "Font", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,JS_PropertyStub,
    JS_PropertyStub,JS_PropertyStub,
    JS_EnumerateStub,JS_ResolveStub,
    JS_ConvertStub,JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS 
};


// static JSPropertySpec font_props[] =
// {
//         {"r",       COLOR_R,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"g",       COLOR_G,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"b",      	COLOR_B,   	   JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"alpha",   COLOR_ALPHA,   JSPROP_ENUMERATE | JSPROP_PERMANENT},        
//         {0,0,0}
// };

static JSFunctionSpec font_methods[] = {
    {"GetTextHeight",		GetTextHeight,		0, 0, 0},    
    {"GetLength",			GetLength,			2, 0, 0},    
    {"GetDimensions",		GetDimensions,		1, 0, 0}, 
    {0,0,0,0,0}
};    

static JSFunctionSpec font_static_methods[] = {
    {"Load",		Load,		2, 0, 0},    
    {0,0,0,0,0}
}; 


bool 
IsFontObject(JSObject *obj)
{
	return JS_InstanceOf(ScriptMgr()->GetContext(), obj, &font_object_class, NULL) == JS_TRUE;
}


void 
Font_SetupAutomation()
{
	if (font_proto_object==0)
	{
		csString msg;
						
	    CHECK("Font", font_proto_object = 
					   JS_InitClass(ScriptMgr()->GetContext(),
								    ScriptMgr()->GetGlobalObject(), 
									NULL /* no parent */, &font_object_class,

		                            /* native constructor function and min arg count */
		                            Font, 4,
		
		                            /* prototype object properties and methods -- these
		                               will be "inherited" by all instances through
		                               delegation up the instance's prototype link. */
		                            NULL /*font_props*/, font_methods,
		
		                            /* class constructor (static) properties and methods */
		                            NULL, font_static_methods));    
		                            
		                            
		 ScriptCon()->Message("Font builtin-object initialized.");
	 }
}

// newline
