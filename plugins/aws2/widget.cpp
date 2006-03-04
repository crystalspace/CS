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
#include "widget.h"

/** @brief The prototype object for widgets. */
static JSObject *widget_proto_object=0;

enum { WIDGET_XMIN, WIDGET_YMIN, WIDGET_XMAX, WIDGET_YMAX, WIDGET_WIDTH, WIDGET_HEIGHT, WIDGET_DIRTY,

		/* Static properties */
       WIDGET_STATIC_START, WIDGET_DOCK_NORTH, WIDGET_DOCK_SOUTH, WIDGET_DOCK_EAST, WIDGET_DOCK_WEST
	   };


/** @brief Forwards a widget GetProperty call. */
static JSBool
widget_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
    
	if (wo) wo->GetProperty(cx, obj, id, vp);		
	
    return JS_TRUE;
}

/** @brief Forwards a widget GetProperty call. */
static JSBool
widget_get_staticProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	
	// Try static properties first.  They can't be handled in the class because
	// They're STATIC properties.
	if (JSVAL_IS_INT(id)) 
   	{			  
	   	   	
		   	
		    switch (JSVAL_TO_INT(id)) 
			{
				case WIDGET_DOCK_NORTH: *vp =  INT_TO_JSVAL(W_DOCK_NORTH); break;					
				case WIDGET_DOCK_SOUTH: *vp =  INT_TO_JSVAL(W_DOCK_SOUTH); break;
				case WIDGET_DOCK_EAST:  *vp =  INT_TO_JSVAL(W_DOCK_EAST);  break;
				case WIDGET_DOCK_WEST:  *vp =  INT_TO_JSVAL(W_DOCK_WEST);  break;
				default:
					return JS_FALSE;				
			}
			
			return JS_TRUE;
		
	}	
	
    return JS_FALSE;
}



/** @brief Forwards a widget SetProperty call. */
static JSBool     
widget_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
    
	if (wo) wo->SetProperty(cx, obj, id, vp);		
	
    return JS_TRUE;
}

///// Widget JS Class //////////////////////////////////////////////////
JSClass widget_object_class = {
    "Widget", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,JS_PropertyStub,
    widget_getProperty,widget_setProperty,
    JS_EnumerateStub,JS_ResolveStub,
    JS_ConvertStub,JS_FinalizeStub 
};

/** @brief The constructor for widget objects. */
static JSBool
Widget(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	aws::widget *wo = new aws::widget();
	
	// Store this widget object with the new widget instance.
  	JS_SetPrivate(cx, obj, (void *)wo);  
  	
  	// Store the object inside the widget class too.
  	wo->SetWidgetObject(obj); 
  	
  	// Save the widget.
  	aws::widgets.Push(wo);
	
	return JS_TRUE;
}

/** @brief Move a widget by (x,y) */
static JSBool
Move(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<2) return JS_FALSE;
	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	int32 x,y;
		
	JS_ValueToInt32(cx,  argv[0], &x);
	JS_ValueToInt32(cx,  argv[1], &y);
	
	wo->Bounds().Move(x, y);
	wo->MoveDocked();
	
	return JS_TRUE;
}

/** @brief Move a widget to (x,y) */
static JSBool
MoveTo(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<2) return JS_FALSE;
	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	int32 x,y;
		
	JS_ValueToInt32(cx,  argv[0], &x);
	JS_ValueToInt32(cx,  argv[1], &y);
	
	wo->Bounds().SetPos(x, y);
	wo->MoveDocked();
	
	return JS_TRUE;
}

/** @brief Resize a widget by (w,h) */
static JSBool
Resize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<2) return JS_FALSE;
	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	int32 w, h;
			
	JS_ValueToInt32(cx,  argv[0], &w);
	JS_ValueToInt32(cx,  argv[1], &h);
	
	wo->Bounds().xmax+=w;
	wo->Bounds().ymax+=h;
	
	wo->MoveDocked();
	wo->AdjustChildrenForStickiness();
	
	return JS_TRUE;
}

/** @brief Resize a widget to (w,h) */
static JSBool
ResizeTo(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<2) return JS_FALSE;
	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	int32 w, h;
			
	JS_ValueToInt32(cx,  argv[0], &w);
	JS_ValueToInt32(cx,  argv[1], &h);
	
	wo->Bounds().SetSize(w, h);
	wo->MoveDocked();
	wo->AdjustChildrenForStickiness();
		
	return JS_TRUE;
}

/** @brief Invalidate a widget. */
static JSBool
Invalidate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{		
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	wo->Invalidate();
	
	return JS_TRUE;
}

/** @brief Gets the pen for a widget. */
static JSBool
GetPen(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{		
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	aws::pen *wpen = wo->GetPen();
	
	if (wpen==0) *rval = JSVAL_NULL;
	else
	{
		*rval = OBJECT_TO_JSVAL(wpen->PenObject());
	}
	
	return JS_TRUE;
}

/** @brief Sets the pen for a widget. */
static JSBool
SetPen(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{		
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	JSObject *pen_object = JSVAL_TO_OBJECT(argv[0]);
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, pen_object);
	
	wo->SetPen(po);
		
	return JS_TRUE;
}

/** @brief Adds a child object. */
static JSBool
AddChild(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{		
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	JSObject *child_object = JSVAL_TO_OBJECT(argv[0]);
	
	if (JS_InstanceOf(cx, child_object, &widget_object_class, NULL) == JS_FALSE)
	{
		JS_ReportError(cx, "Trying to add an object that is not a Widget as a child is prohibited.");
		return JS_FALSE;	
	} 
	
	aws::widget *child_wo = (aws::widget *)JS_GetPrivate(cx, child_object);
	
	wo->AddChild(child_wo);
	
	// Remove from global widget set.
	aws::widgets.Delete(child_wo);
		
	return JS_TRUE;
}

/** @brief Removes a child object. */
static JSBool
RemoveChild(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{		
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	JSObject *child_object = JSVAL_TO_OBJECT(argv[0]);
	
	if (JS_InstanceOf(cx, child_object, &widget_object_class, NULL) == JS_FALSE)
	{
		JS_ReportError(cx, "Trying to remove an object that is not a Widget as a child is prohibited.");
		return JS_FALSE;	
	} 
	
	aws::widget *child_wo = (aws::widget *)JS_GetPrivate(cx, child_object);
	
	wo->RemoveChild(child_wo);
		
	return JS_TRUE;
}

/** @brief Adds a child object. */
static JSBool
Dock(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	int32 where;	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	JSObject *sib_object = JSVAL_TO_OBJECT(argv[0]);
	JS_ValueToInt32(cx, argv[1], &where);
	
	if (JS_InstanceOf(cx, sib_object, &widget_object_class, NULL) == JS_FALSE)
	{
		JS_ReportError(cx, "Trying to dock an object that is not a Widget is prohibited.");
		return JS_FALSE;	
	} 
	
	aws::widget *sib_wo = (aws::widget *)JS_GetPrivate(cx, sib_object);
	
	wo->Dock(sib_wo, where);
			
	return JS_TRUE;
}

/** @brief Adds a child object. */
static JSBool
SetFrameAnchor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	int32 where;	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	JS_ValueToInt32(cx, argv[0], &where);
	
	wo->SetFrameAnchor(where);
	wo->AdjustForStickiness();
			
	return JS_TRUE;
}

/** @brief Adds a child object. */
static JSBool
ClearFrameAnchor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	int32 where;	
	aws::widget *wo = (aws::widget *)JS_GetPrivate(cx, obj);
	
	JS_ValueToInt32(cx, argv[0], &where);
	
	wo->ClearFrameAnchor(where);
			
	return JS_TRUE;
}


static JSPropertySpec widget_props[] =
{
        {"xmin",       	WIDGET_XMIN,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {"ymin",       	WIDGET_YMIN,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {"xmax",       	WIDGET_XMAX,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {"ymax",       	WIDGET_YMAX,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {"width",       WIDGET_WIDTH,   JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {"height",      WIDGET_HEIGHT,  JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {"dirty",       WIDGET_DIRTY,   JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {0,0,0}
};


static JSPropertySpec widget_static_props[] =
{
        {"DOCK_NORTH",     WIDGET_DOCK_NORTH,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        {"DOCK_SOUTH",     WIDGET_DOCK_SOUTH,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        {"DOCK_EAST",      WIDGET_DOCK_EAST,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        {"DOCK_WEST",      WIDGET_DOCK_WEST,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        
        {"STICK_NORTH",     WIDGET_DOCK_NORTH,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        {"STICK_SOUTH",     WIDGET_DOCK_SOUTH,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        {"STICK_EAST",      WIDGET_DOCK_EAST,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        {"STICK_WEST",      WIDGET_DOCK_WEST,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, widget_get_staticProperty},
        
        {0,0,0}
};

static JSFunctionSpec widget_methods[] = {
    {"Move",		Move,		2, 0, 0},
    {"MoveTo",		MoveTo,		2, 0, 0},    
    {"Resize",		Resize,		2, 0, 0},
    {"ResizeTo",	ResizeTo,	2, 0, 0},    
    {"Invalidate",	Invalidate,	0, 0, 0},    
    {"GetPen",		GetPen,		0, 0, 0}, 
    {"SetPen",		SetPen,		1, 0, 0},
    
    {"AddChild",	AddChild,	 1, 0, 0},
    {"RemoveChild",	RemoveChild, 1, 0, 0},
    {"Dock",		Dock, 		 2, 0, 0},
    
    {"SetFrameAnchor",		SetFrameAnchor,		 1, 0, 0},
    {"ClearFrameAnchor",	ClearFrameAnchor,	 1, 0, 0},
    
    
    {0,0,0,0,0}
};    
    
void 
Widget_SetupAutomation()
{
	if (widget_proto_object==0)
	{				
		widget_proto_object = 
					   JS_InitClass(ScriptMgr()->GetContext(),
								    ScriptMgr()->GetGlobalObject(), 
									NULL /* no parent */, &widget_object_class,

		                            /* native constructor function and min arg count */
		                            Widget, 0,
		
		                            /* prototype object properties and methods -- these
		                               will be "inherited" by all instances through
		                               delegation up the instance's prototype link. */
		                            widget_props, widget_methods,
		
		                            /* class constructor (static) properties and methods */
		                            widget_static_props, NULL); 
		                            
		 ScriptCon()->Message("Widget builtin-object initialized.");   
	 }
}


namespace aws
{
	
/** @brief The list of all widgets. */
widget_list_t widgets;

bool widget::SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  if (JSVAL_IS_INT(id)) 
  {	
	JSBool b;
	  
    switch (JSVAL_TO_INT(id)) 
    {
      case WIDGET_XMIN:  JS_ValueToInt32(cx, *vp, &Bounds().xmin); break;
      case WIDGET_YMIN:  JS_ValueToInt32(cx, *vp, &Bounds().ymin); break;
      case WIDGET_XMAX:  JS_ValueToInt32(cx, *vp, &Bounds().xmax); break;
      case WIDGET_YMAX:  JS_ValueToInt32(cx, *vp, &Bounds().ymax); break;
      case WIDGET_WIDTH:  { int32 w; JS_ValueToInt32(cx, *vp, &w);  Bounds().SetSize(w, Bounds().Height()); } break;
      case WIDGET_HEIGHT: { int32 h; JS_ValueToInt32(cx, *vp, &h);  Bounds().SetSize(Bounds().Width(), h); } break;
      case WIDGET_DIRTY:  JS_ValueToBoolean(cx, *vp, &b); dirty = b; break;          
      default: return false;
    }  
    
   }
   
   return true;
}

bool widget::GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  if (JSVAL_IS_INT(id)) 
  {	
// 	csString msg;
// 	
// 	msg.Format("widget:GetProperty=%d,%d,%d,%d", Bounds().xmin, Bounds().ymin, Bounds().xmax, Bounds().ymax);
// 	ScriptCon()->Message(msg);
		  
    switch (JSVAL_TO_INT(id)) 
    {
      case WIDGET_XMIN:   *vp = INT_TO_JSVAL(Bounds().xmin); break;
      case WIDGET_YMIN:   *vp = INT_TO_JSVAL(Bounds().ymin); break;
      case WIDGET_XMAX:   *vp = INT_TO_JSVAL(Bounds().xmax); break;
      case WIDGET_YMAX:   *vp = INT_TO_JSVAL(Bounds().ymax); break;
      case WIDGET_WIDTH:  *vp = INT_TO_JSVAL(Bounds().Width());  break;
      case WIDGET_HEIGHT: *vp = INT_TO_JSVAL(Bounds().Height()); break;
      case WIDGET_DIRTY:  *vp = (dirty ? JSVAL_TRUE : JSVAL_FALSE);  break; 
      default: return false;
    }  
    
    
   }
   
   return true;
}

void 
widget::Draw(iPen *output_pen)
{
	if (wpen!=0)
	{
		csString msg;
		jsval argv[1], rval;
		
		// Prepare for the drawing.
		fr.Prepare(output_pen);		
		
		output_pen->SetColor(1,1,1,1);
		output_pen->DrawRect(0,0,Bounds().Width(),Bounds().Height());
		//msg.Format("(%d,%d,%d,%d)", Bounds().xmin, Bounds().ymin, Bounds().Width(), Bounds().Height());
		//output_pen->Write(0,-10, msg.GetData());
		
		
		if (IsDirty())
		{			
			// Prepare the pen object
			argv[0] = OBJECT_TO_JSVAL(wpen->PenObject());
			
			// Call the Automation drawing function.
			JS_CallFunctionName(ScriptMgr()->GetContext(), w_object, "onDraw", 1, argv, &rval);
			
			// Clear the dirty flag
			dirty = JS_FALSE;
		}		

		
		wpen->Draw(output_pen);	
	
		// Draw all children.
		for(size_t i=0; i<children.Length(); ++i)
		{
			children[i]->Draw(output_pen);						
		}		

		// Finish the drawing
		fr.Finish(output_pen);
	}
}


} // end of namespace
