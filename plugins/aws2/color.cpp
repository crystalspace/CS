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
#include "csutil/cscolor.h"
#include "script_manager.h"
#include "script_console.h"
#include "color.h"


#define CHECK(objname, check) \
  if (!(check)) { \
    msg.Format (objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
    ScriptCon ()->Message (msg); \
  }

static JSBool Set (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


/// The prototype object for colors.
static JSObject *color_proto_object=0;


enum 
{ 
  COLOR_R, 
  COLOR_G, 
  COLOR_B, 
  COLOR_ALPHA 
};

/// The constructor for color objects.
static JSBool Color (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                     jsval *rval)	
{
  csString msg;
  csColor4 *co = new csColor4();

  // Store this widget object with the new widget instance.
  CHECK("Color", JS_SetPrivate (cx, obj, (void *)co)==JS_TRUE);

  if (argc>0)
  {
    Set (cx, obj, argc, argv, rval);
  }  

  return JS_TRUE;
}


/// Sets a color. Any or all parameters may be included.
static JSBool Set(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                  jsval *rval)	
{
  csColor4 *co = (csColor4 *)JS_GetPrivate (cx, obj);

  for (uintN i=0; i<argc; ++i)
  {	
    jsdouble c;

    if (JS_TypeOfValue (cx, argv[i])!=JSTYPE_VOID)
    {
      JS_ValueToNumber (cx,  argv[i], &c);	

      // Adjust the number to be between 0 and 1 
      while (c>1) { c/=255.0; }

      switch(i)
      {
      case 0: co->red = (float)c; break;
      case 1: co->green = (float)c; break;
      case 2: co->blue = (float)c; break;
      case 3: co->alpha = (float)c; break;
      default:
        return JS_TRUE;
      }
    }
    // Abort the loop if we get an undefined value, because
    // we have probably reached the end of the passed in values.
    else 
      break;
  }

  return JS_TRUE;
}

/// Resolves a Color GetProperty.
static JSBool color_getProperty(JSContext *cx, JSObject *obj, jsval id, 
                                jsval *vp)
{

  csColor4 *co = (csColor4 *)JS_GetPrivate (cx, obj);    

  if (JSVAL_IS_INT(id)) 
  {
    jsdouble c;

    switch (JSVAL_TO_INT(id)) 
    {
    case COLOR_R: c=co->red;  break;
    case COLOR_G: c=co->green; break;
    case COLOR_B: c=co->blue; break;
    case COLOR_ALPHA: c=co->alpha; break;
    default:
      return JS_FALSE;
    }

    *vp = DOUBLE_TO_JSVAL(&c);

    return JS_TRUE;
  }

  return JS_FALSE;
}

/// Resolves a Color SetProperty call.
static JSBool color_setProperty(JSContext *cx, JSObject *obj, jsval id, 
                                jsval *vp)
{
  csColor4 *co = (csColor4 *)JS_GetPrivate (cx, obj);    
  jsdouble c;

  if (JSVAL_IS_INT(id)) 
  {
    JS_ValueToNumber (cx, *vp, &c);

    switch (JSVAL_TO_INT(id)) 
    {
    case COLOR_R: co->red = (float)c; break;
    case COLOR_G: co->green = (float)c; break;
    case COLOR_B: co->blue = (float)c; break;
    case COLOR_ALPHA: co->alpha = (float)c; break;
    default:
      return JS_FALSE;
    }

    return JS_TRUE;
  }

  return JS_FALSE;
}

JSClass color_object_class = 
{
  "Color", 
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
  JS_PropertyStub,
  color_getProperty,
  color_setProperty,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  JS_FinalizeStub,
  JSCLASS_NO_OPTIONAL_MEMBERS 
};


static JSPropertySpec color_props[] =
{
  {"r",       COLOR_R,        JSPROP_ENUMERATE | JSPROP_PERMANENT},
  {"g",       COLOR_G,        JSPROP_ENUMERATE | JSPROP_PERMANENT},
  {"b",       COLOR_B,        JSPROP_ENUMERATE | JSPROP_PERMANENT},
  {"alpha",   COLOR_ALPHA,    JSPROP_ENUMERATE | JSPROP_PERMANENT},        
  {0,0,0}
};

static JSFunctionSpec color_methods[] = 
{
  {"Set",     Set,            4, 0, 0},    
  {0,0,0,0,0}
};    


bool IsColorObject (JSObject *obj)
{
  return JS_InstanceOf (ScriptMgr()->GetContext(), obj, 
    &color_object_class, NULL) == JS_TRUE;
}


void Color_SetupAutomation ()
{
  if (color_proto_object==0)
  {
    csString msg;

    CHECK("Color", color_proto_object = 
      JS_InitClass (
      ScriptMgr ()->GetContext (),
      ScriptMgr ()->GetGlobalObject (), 
      NULL /* no parent*/, 
      &color_object_class,

      /* native constructor function and min arg count */
      Color, 4,

      /* prototype object properties and methods -- these
      will be "inherited" by all instances through
      delegation up the instance's prototype link.*/
      color_props, color_methods,

      /* class constructor (static) properties and methods */
      NULL, NULL));    


    ScriptCon ()->Message ("Color builtin-object initialized.");
  }
}

// newline
