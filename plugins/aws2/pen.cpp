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
#include "manager.h"
#include "script_manager.h"
#include "script_console.h"
#include "pen.h"
#include "color.h"
#include "font.h"
#include <string.h>

/** @brief The prototype object for pens. */
static JSObject *pen_proto_object=0;

/** @brief The constructor for pen objects. */
static JSBool
Pen(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	aws::pen *po = new aws::pen();
			
	// Store this pen object with the new pen instance.
  	JS_SetPrivate(cx, obj, (void *)po);  
  	
  	// Store the object inside the pen class too.
  	po->SetPenObject(obj); 
  		
	return JS_TRUE;
}

/** @brief Clears out the contents of the pen. */
static JSBool
Clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	if (po) po->Clear();		
		
	return JS_TRUE;
}

/** @brief Set the color. */
static JSBool
SetColor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{			
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
		
	jsdouble r=0.0,g=0.0,b=0.0,a=1.0;
	
	if (JSVAL_IS_OBJECT(argv[0]))
	{
		JSObject *color_object = JSVAL_TO_OBJECT(argv[0]);
				
		if (IsColorObject(color_object))
		{	
			csColor4 *co = (csColor4 *)JS_GetPrivate(cx, color_object); 
			
			r=co->red;
			g=co->green;
			b=co->blue;
			a=co->alpha;		
		}		
	}
	else
	{		
		JS_ValueToNumber(cx,  argv[0], &r);
		JS_ValueToNumber(cx,  argv[1], &g);
		JS_ValueToNumber(cx,  argv[2], &b);
		JS_ValueToNumber(cx,  argv[3], &a);
	}
		
	if (po) po->SetColor(r,g,b,a);	
		
	return JS_TRUE;
}

/** @brief Swaps the color of the pen with the alternate color. */
static JSBool
SwapColors(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	if (po) po->SwapColors();		
		
	return JS_TRUE;
}

/** @brief Swaps the color of the pen with the alternate color. */
static JSBool
PushTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	if (po) po->PushTransform();		
		
	return JS_TRUE;
}


/** @brief Swaps the color of the pen with the alternate color. */
static JSBool
PopTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	if (po) po->PopTransform();		
		
	return JS_TRUE;
}

/** @brief Swaps the color of the pen with the alternate color. */
static JSBool
ClearTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	if (po) po->ClearTransform();		
		
	return JS_TRUE;
}

/** @brief Draw a line from (x1,y1) to (x2,y2) */
static JSBool
DrawLine(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<4) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	int32 x1,y1,x2,y2;
		
	JS_ValueToInt32(cx,  argv[0], &x1);
	JS_ValueToInt32(cx,  argv[1], &y1);
	JS_ValueToInt32(cx,  argv[2], &x2);
	JS_ValueToInt32(cx,  argv[3], &y2);	

	if (po) po->DrawLine(x1,y1,x2,y2);	
		
	return JS_TRUE;
}

/** @brief Draw a rect from (x1,y1) to (x2,y2), with optional fill */
static JSBool
DrawRect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<4) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	int32 x1,y1,x2,y2;
	JSBool fill, swap_colors;
		
	JS_ValueToInt32(cx,  argv[0], &x1);
	JS_ValueToInt32(cx,  argv[1], &y1);
	JS_ValueToInt32(cx,  argv[2], &x2);
	JS_ValueToInt32(cx,  argv[3], &y2);
	JS_ValueToBoolean(cx, argv[4], &fill);
	JS_ValueToBoolean(cx, argv[5], &swap_colors);
		
	if (po) po->DrawRect(x1,y1,x2,y2,swap_colors==JS_TRUE, fill==JS_TRUE);	
		
	return JS_TRUE;
}

/** @brief Draw a mitered rect from (x1,y1) to (x2,y2), miter, with optional fill */
static JSBool
DrawMiteredRect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<5) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	int32 x1,y1,x2,y2;
	jsdouble miter;
	JSBool fill, swap_colors;
		
	JS_ValueToInt32(cx,  argv[0], &x1);
	JS_ValueToInt32(cx,  argv[1], &y1);
	JS_ValueToInt32(cx,  argv[2], &x2);
	JS_ValueToInt32(cx,  argv[3], &y2);
	JS_ValueToNumber(cx,  argv[4], &miter);
	JS_ValueToBoolean(cx, argv[5], &fill);
	JS_ValueToBoolean(cx, argv[6], &swap_colors);
		
	if (po) po->DrawMiteredRect(x1,y1,x2,y2,(float)miter, swap_colors==JS_TRUE, fill==JS_TRUE);	
		
	return JS_TRUE;
}

/** @brief Draw a rounded rect from (x1,y1) to (x2,y2), roundness, with optional fill */
static JSBool
DrawRoundedRect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<5) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	int32 x1,y1,x2,y2;
	jsdouble roundness;
	JSBool fill, swap_colors;
		
	JS_ValueToInt32(cx,  argv[0], &x1);
	JS_ValueToInt32(cx,  argv[1], &y1);
	JS_ValueToInt32(cx,  argv[2], &x2);
	JS_ValueToInt32(cx,  argv[3], &y2);
	JS_ValueToNumber(cx,  argv[4], &roundness);
	JS_ValueToBoolean(cx, argv[5], &fill);
	JS_ValueToBoolean(cx, argv[6], &swap_colors);
		
	if (po) po->DrawRoundedRect(x1,y1,x2,y2,(float)roundness, swap_colors==JS_TRUE, fill==JS_TRUE);	
		
	return JS_TRUE;
}

/** @brief Draw an arc from (x1,y1) to (x2,y2), (start_angle,end_angle), with optional fill */
static JSBool
DrawArc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<6) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	int32 x1,y1,x2,y2;
	jsdouble start_angle, end_angle;
	JSBool fill, swap_colors;
		
	JS_ValueToInt32(cx,  argv[0], &x1);
	JS_ValueToInt32(cx,  argv[1], &y1);
	JS_ValueToInt32(cx,  argv[2], &x2);
	JS_ValueToInt32(cx,  argv[3], &y2);
	JS_ValueToNumber(cx,  argv[4], &start_angle);
	JS_ValueToNumber(cx,  argv[5], &end_angle);
	JS_ValueToBoolean(cx, argv[6], &fill);
	JS_ValueToBoolean(cx, argv[7], &swap_colors);
		
	if (po) po->DrawArc(x1,y1,x2,y2,(float)start_angle, (float)end_angle, swap_colors==JS_TRUE, fill==JS_TRUE);	
		
	return JS_TRUE;
}

/** @brief Draw a triangle from (x1,y1) to (x2,y2) to (x3, y3), with optional fill */
static JSBool
DrawTriangle(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<6) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	int32 x1,y1,x2,y2,x3,y3;
	JSBool fill;
		
	JS_ValueToInt32(cx,  argv[0], &x1);
	JS_ValueToInt32(cx,  argv[1], &y1);
	JS_ValueToInt32(cx,  argv[2], &x2);
	JS_ValueToInt32(cx,  argv[3], &y2);
	JS_ValueToInt32(cx,  argv[4], &x3);
	JS_ValueToInt32(cx,  argv[5], &y3);	
	JS_ValueToBoolean(cx, argv[6], &fill);
		
	if (po) po->DrawTriangle(x1,y1,x2,y2,x3,y3, fill==JS_TRUE);	
		
	return JS_TRUE;
}

/** @brief Draw a triangle from (x1,y1) to (x2,y2) to (x3, y3), with optional fill */
static JSBool
Write(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	//csString msg;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	int32 x1,y1;	
		
	JSObject *fnt = JSVAL_TO_OBJECT(argv[0]);
	
	if (argv[0] == JSVAL_VOID || !IsFontObject(fnt))
	{
		ScriptCon()->Message("Pen: write: Object passed as parameter 0 is not a font object!");	
		return JS_FALSE;
	}
		
	csRef<iFont> *fo = (csRef<iFont> *)JS_GetPrivate(cx, fnt);
		
	JS_ValueToInt32(cx,  argv[1], &x1);
	JS_ValueToInt32(cx,  argv[2], &y1);
	JSString *text = JS_ValueToString(cx,  argv[3]);
	
// 	msg.Format("Pen: write %d, %d, %s, %p, font valid: %s", x1, y1, JS_GetStringBytes(text), (iFont *)(*fo), fo->IsValid() ? "yes" : "no");
// 	ScriptCon()->Message(msg);
// 		
	if (po && fo->IsValid()) po->Write((iFont *)(*fo), x1,y1,JS_GetStringBytes(text));	
		
	return JS_TRUE;
}

/** Draw this pen into another pen. */
static JSBool
Draw(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{			
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	if (JSVAL_IS_OBJECT(argv[0]))
	{
		JSObject *pen_object = JSVAL_TO_OBJECT(argv[0]);
				
		if (IsPenObject(pen_object))
		{	
			aws::pen *other_pen = (aws::pen *)JS_GetPrivate(cx, pen_object); 					
			
			if (po) po->Draw(other_pen);	
		}		
	}
			
	
		
	return JS_TRUE;
}


/** @brief Move a pen by (x,y) */
static JSBool
Translate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<2) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	jsdouble x,y,z;
		
	JS_ValueToNumber(cx,  argv[0], &x);
	JS_ValueToNumber(cx,  argv[1], &y);
	JS_ValueToNumber(cx,  argv[2], &z);
		
	if (po) po->Translate(csVector3 ((float)x, (float)y, (float)z));	
		
	return JS_TRUE;
}

/** @brief Rotate a pen by (a) */
static JSBool
Rotate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)	
{
	if (argc<1) return JS_FALSE;
	
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
	
	jsdouble a;
		
	JS_ValueToNumber(cx,  argv[0], &a);	
	
	if (po) po->Rotate((float)a);	
		
	return JS_TRUE;
}


/** @brief Forewards a pen GetProperty call. */
static JSBool
pen_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
    
	if (po) po->GetProperty(cx, obj, id, vp);		
	
    return JS_TRUE;
}

/** @brief Forewards a pen SetProperty call. */
static JSBool     
pen_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	aws::pen *po = (aws::pen *)JS_GetPrivate(cx, obj);
    
	if (po) po->SetProperty(cx, obj, id, vp);		
	
    return JS_TRUE;
}

JSClass pen_object_class = {
    "Pen", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,JS_PropertyStub,
    pen_getProperty,pen_setProperty,
    JS_EnumerateStub,JS_ResolveStub,
    JS_ConvertStub,JS_FinalizeStub 
};

// enum { WIDGET_XMIN, WIDGET_YMIN, WIDGET_XMAX, WIDGET_YMAX, WIDGET_WIDTH, WIDGET_HEIGHT, WIDGET_DIRTY };

// static JSPropertySpec pen_props[] =
// {
//         {"xmin",       	WIDGET_XMIN,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"ymin",       	WIDGET_YMIN,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"xmax",       	WIDGET_XMAX,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"ymax",       	WIDGET_YMAX,    JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"width",       WIDGET_WIDTH,   JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"height",      WIDGET_HEIGHT,  JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"dirty",       WIDGET_DIRTY,   JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {0,0,0}
// };

static JSFunctionSpec pen_methods[] = {
	{"Clear",		Clear,		0, 0, 0},
	{"SetColor",	SetColor,	4, 0, 0},
	{"SwapColors",  SwapColors, 0, 0, 0}, 
	
	{"PushTransform",  PushTransform,  0, 0, 0}, 
	{"PopTransform",   PopTransform,   0, 0, 0}, 
	{"ClearTransform", ClearTransform, 0, 0, 0}, 
	
    {"Translate",	Translate,	3, 0, 0},
    {"Rotate",		Rotate,		1, 0, 0},   
     
    {"DrawLine",	DrawLine,	4, 0, 0},
    {"DrawRect",	DrawRect,	6, 0, 0},    
    {"DrawMiteredRect",	DrawMiteredRect,	7, 0, 0},
    {"DrawRoundedRect",	DrawRoundedRect,	7, 0, 0},
    {"DrawArc",			DrawArc,			8, 0, 0},
    {"DrawTriangle",	DrawTriangle,		7, 0, 0},
    
    {"Draw",		    Draw,				1, 0, 0},
    
    {"Write",		    Write,				4, 0, 0},    
    
    
    //{"Invalidate",	Invalidate,	0, 0, 0},    
    {0,0,0,0,0}
};    

bool 
IsPenObject(JSObject *obj)
{
	return JS_InstanceOf(ScriptMgr()->GetContext(), obj, &pen_object_class, NULL) == JS_TRUE;
}
    
void 
Pen_SetupAutomation()
{
	if (pen_proto_object==0)
	{				
		pen_proto_object = 
					   JS_InitClass(ScriptMgr()->GetContext(),
								    ScriptMgr()->GetGlobalObject(), 
									NULL /* no parent */, &pen_object_class,

		                            /* native constructor function and min arg count */
		                            Pen, 0,
		
		                            /* prototype object properties and methods -- these
		                               will be "inherited" by all instances through
		                               delegation up the instance's prototype link. */
		                            NULL /*pen_props*/, pen_methods,
		
		                            /* class constructor (static) properties and methods */
		                            NULL, NULL); 
		                            
		 ScriptCon()->Message("Pen builtin-object initialized.");   
	 }
}


namespace aws
{

bool pen::SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
//   if (JSVAL_IS_INT(id)) 
//   {	
// 	JSBool b;
// 	  
//     switch (JSVAL_TO_INT(id)) 
//     {
//       case WIDGET_XMIN:  JS_ValueToInt32(cx, *vp, &Bounds().xmin); break;
//       case WIDGET_YMIN:  JS_ValueToInt32(cx, *vp, &Bounds().ymin); break;
//       case WIDGET_XMAX:  JS_ValueToInt32(cx, *vp, &Bounds().xmax); break;
//       case WIDGET_YMAX:  JS_ValueToInt32(cx, *vp, &Bounds().ymax); break;
//       case WIDGET_WIDTH:  { int32 w; JS_ValueToInt32(cx, *vp, &w);  Bounds().SetSize(w, Bounds().Height()); } break;
//       case WIDGET_HEIGHT: { int32 h; JS_ValueToInt32(cx, *vp, &h);  Bounds().SetSize(Bounds().Width(), h); } break;
//       case WIDGET_DIRTY:  JS_ValueToBoolean(cx, *vp, &b); dirty = b; break;          
//       default: return false;
//     }  
//     
//    }
   
   return true;
}

bool pen::GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  if (JSVAL_IS_INT(id)) 
  {	
// 	csString msg;
// 	
// 	msg.Format("pen:GetProperty=%d,%d,%d,%d", Bounds().xmin, Bounds().ymin, Bounds().xmax, Bounds().ymax);
// 	ScriptCon()->Message(msg);
		  
//     switch (JSVAL_TO_INT(id)) 
//     {
//       case WIDGET_XMIN:   *vp = INT_TO_JSVAL(Bounds().xmin); break;
//       case WIDGET_YMIN:   *vp = INT_TO_JSVAL(Bounds().ymin); break;
//       case WIDGET_XMAX:   *vp = INT_TO_JSVAL(Bounds().xmax); break;
//       case WIDGET_YMAX:   *vp = INT_TO_JSVAL(Bounds().ymax); break;
//       case WIDGET_WIDTH:  *vp = INT_TO_JSVAL(Bounds().Width());  break;
//       case WIDGET_HEIGHT: *vp = INT_TO_JSVAL(Bounds().Height()); break;
//       case WIDGET_DIRTY:  *vp = (dirty ? JSVAL_TRUE : JSVAL_FALSE);  break; 
//       default: return false;
//     }  
    
    
   }
   
   return true;
}

enum { PEN_OP_NOP = 0, PEN_OP_SETCOLOR, PEN_OP_SWAPCOLORS, PEN_OP_CLEARTRANSFORM, PEN_OP_PUSHTRANSFORM, PEN_OP_POPTRANSFORM,
       PEN_OP_SETORIGIN, PEN_OP_TRANSLATE, PEN_OP_ROTATE, PEN_OP_DRAWLINE, PEN_OP_DRAWPOINT,
       PEN_OP_DRAWRECT, PEN_OP_DRAWMITEREDRECT, PEN_OP_DRAWROUNDRECT,
       PEN_OP_DRAWARC, PEN_OP_DRAWTRIANGLE, PEN_OP_WRITE, PEN_OP_WRITEBOXED };


void  pen::Draw(iPen *_pen) 
{
// 	csString msg;
	
	buf->SetPos(0);
	
// 	msg.Format("pen::Draw(): enter: buf=%p, size=%d", buf, (buf ? buf->GetSize() : 0));
// 	ScriptCon()->Message(msg);
	
	while(!(buf->AtEOF()))
	{
		uint8 op;
		
		buf->Read((char *)&op, sizeof(uint8));
		
		switch(op)
		{
			case PEN_OP_NOP:
				ScriptCon()->Message("Pen NOP (pen buffer error!)");
			break;
			
			case PEN_OP_SETCOLOR:
			{
				float r,g,b,a;
				
				buf->Read((char *)&r, sizeof(float));
				buf->Read((char *)&g, sizeof(float));	
				buf->Read((char *)&b, sizeof(float));	
				buf->Read((char *)&a, sizeof(float));
				
				_pen->SetColor(r,g,b,a);				
			} break;
			
			case PEN_OP_SWAPCOLORS:
				_pen->SwapColors();
				break;
				
			case PEN_OP_CLEARTRANSFORM:
				_pen->ClearTransform();
				break;
				
			case PEN_OP_PUSHTRANSFORM:
				_pen->PushTransform();
				break;
				
			case PEN_OP_POPTRANSFORM:
				_pen->PopTransform();
				break;
				
			case PEN_OP_SETORIGIN:
			{
				float x,y,z;
				
				buf->Read((char *)&x, sizeof(float));
				buf->Read((char *)&y, sizeof(float));	
				buf->Read((char *)&z, sizeof(float));	
				
				_pen->SetOrigin(csVector3(x,y,z));				
			} break;
			
			case PEN_OP_TRANSLATE:
			{
				float x,y,z;
				
				buf->Read((char *)&x, sizeof(float));
				buf->Read((char *)&y, sizeof(float));	
				buf->Read((char *)&z, sizeof(float));	
				
				_pen->Translate(csVector3(x,y,z));				
			} break;
			
			case PEN_OP_ROTATE:
			{
				float a;
				
				buf->Read((char *)&a, sizeof(float));
				
				_pen->Rotate(a);				
			} break;
			
			case PEN_OP_DRAWLINE:
			{
				uint x1, y1, x2, y2;
						
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));
								
								
				_pen->DrawLine(x1,y1,x2,y2);					
		    } break;
		    
		    case PEN_OP_DRAWRECT:
			{
				uint x1, y1, x2, y2;
				bool swap_colors, fill;
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				
				buf->Read((char *)&swap_colors, sizeof(bool));	
				buf->Read((char *)&fill, sizeof(bool));	
				
				_pen->DrawRect(x1,y1,x2,y2,swap_colors,fill);
		    } break;
		    
		    case PEN_OP_DRAWMITEREDRECT:
			{
				uint x1, y1, x2, y2;
				float miter;
				bool swap_colors, fill;
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&miter, sizeof(float));
				
				buf->Read((char *)&swap_colors, sizeof(bool));	
				buf->Read((char *)&fill, sizeof(bool));	
				
				_pen->DrawMiteredRect(x1,y1,x2,y2,miter,swap_colors,fill);
		    } break;
		    
		    case PEN_OP_DRAWROUNDRECT:
			{
				uint x1, y1, x2, y2;
				float roundness;
				bool swap_colors, fill;
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&roundness, sizeof(float));
				
				buf->Read((char *)&swap_colors, sizeof(bool));	
				buf->Read((char *)&fill, sizeof(bool));	
				
				_pen->DrawRoundedRect(x1,y1,x2,y2,roundness,swap_colors,fill);
		    } break;
		    
		    case PEN_OP_DRAWARC:
			{
				uint x1, y1, x2, y2;
				float start_angle, end_angle;
				bool swap_colors, fill;
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&start_angle, sizeof(float));
				buf->Read((char *)&end_angle, sizeof(float));
				
				buf->Read((char *)&swap_colors, sizeof(bool));	
				buf->Read((char *)&fill, sizeof(bool));	
				
				_pen->DrawArc(x1,y1,x2,y2,start_angle,end_angle,swap_colors,fill);
		    } break;
		    
		    case PEN_OP_DRAWTRIANGLE:
			{
				uint x1, y1, x2, y2, x3, y3;
				bool fill;
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&x3, sizeof(uint));
				buf->Read((char *)&y3, sizeof(uint));									
				
				buf->Read((char *)&fill, sizeof(bool));	
				
				_pen->DrawTriangle(x1,y1,x2,y2,x3,y3,fill);
		    } break;
		    
		    case PEN_OP_WRITE:
			{
				uint x1, y1, len;
				iFont *font;				
				const char *text;
	
				buf->Read((char *)&font, sizeof(iFont *));
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));				
				buf->Read((char *)&len, sizeof(uint));						
				
				// Read the text right out of the buffer.
				text = buf->GetData() + buf->GetPos();
				
				// Seek forward, skipping the null.
				buf->SetPos(buf->GetPos()+len+1);
				
				_pen->Write(font,x1,y1,(char *)text);
		    } break;
		    
		    case PEN_OP_WRITEBOXED:
			{
				uint x1, y1, x2, y2, h_align, v_align, len;
				iFont *font;				
				const char *text;
	
				buf->Read((char *)&font, sizeof(iFont *));
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));				
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));				
				buf->Read((char *)&h_align, sizeof(uint));									
				buf->Read((char *)&v_align, sizeof(uint));				
				buf->Read((char *)&len, sizeof(uint));									
				
				// Read the text right out of the buffer.
				text = buf->GetData() + buf->GetPos();
				
				// Seek forward, skipping the null.
				buf->SetPos(buf->GetPos()+len+1);
				
				_pen->WriteBoxed(font,x1,y1,x2,y2,h_align, v_align,(char *)text);
		    } break;
		    
		    default:
		    	// An unrecognized op is bad, so abort.
		    	return;
		    				
		} // end switch op		
		
	} // end while not eof.	
}


/** 
* Sets the current color. 
*/
void pen::SetColor (float r, float g, float b, float a) 
{ 
	uint8 op = PEN_OP_SETCOLOR;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&r, sizeof(float));
	buf->Write((const char *)&g, sizeof(float));	
	buf->Write((const char *)&b, sizeof(float));	
	buf->Write((const char *)&a, sizeof(float));	
}

/** 
* Sets the current color. 
*/
void pen::SetColor(const csColor4 &color)
{ 
	uint8 op = PEN_OP_SETCOLOR;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&color.red, sizeof(float));	
	buf->Write((const char *)&color.green, sizeof(float));	
	buf->Write((const char *)&color.blue, sizeof(float));	
	buf->Write((const char *)&color.alpha, sizeof(float));		
}

/**
* Swaps the current color and the alternate color. 
*/
void pen::SwapColors() 
{ 
	uint8 op = PEN_OP_SWAPCOLORS;
	
	buf->Write((const char *)&op, sizeof(uint8));	
}

/**    
* Clears the current transform, resets to identity.
*/
void pen::ClearTransform() 
{ 
	uint8 op = PEN_OP_CLEARTRANSFORM;
	
	buf->Write((const char *)&op, sizeof(uint8));		
}

/** 
* Pushes the current transform onto the stack. *
*/
void pen::PushTransform() 
{ 
	uint8 op = PEN_OP_PUSHTRANSFORM;
	
	buf->Write((const char *)&op, sizeof(uint8));	
}

/**
* Pops the transform stack. The top of the stack becomes the current
* transform. 
*/
void pen::PopTransform() 
{ 
	uint8 op = PEN_OP_POPTRANSFORM;
	
	buf->Write((const char *)&op, sizeof(uint8));	
}

/** 
* Sets the origin of the coordinate system. 
*/
void pen::SetOrigin(const csVector3 &o) 
{ 
	uint8 op = PEN_OP_SETORIGIN;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&o.x, sizeof(float));	
	buf->Write((const char *)&o.y, sizeof(float));	
	buf->Write((const char *)&o.z, sizeof(float));		
}

/** 
* Translates by the given vector
*/
void pen::Translate(const csVector3 &t) 
{ 
	uint8 op = PEN_OP_TRANSLATE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&t.x, sizeof(float));	
	buf->Write((const char *)&t.y, sizeof(float));	
	buf->Write((const char *)&t.z, sizeof(float));	
}

/**
* Rotates by the given angle.
*/
void pen::Rotate(const float &a) 
{ 
	uint8 op = PEN_OP_ROTATE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&a, sizeof(float));		
}

/** 
* Draws a single line. 
*/
void pen::DrawLine (uint x1, uint y1, uint x2, uint y2) 
{ 
	uint8 op = PEN_OP_DRAWLINE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));		
}

/** 
* Draws a single point. 
*/
void pen::DrawPoint (uint x1, uint y2)
{
	uint8 op = PEN_OP_DRAWPOINT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));		
}

/** 
* Draws a rectangle. 
*/
void pen::DrawRect (uint x1, uint y1, uint x2, uint y2,
	bool swap_colors, bool fill) 
{ 
	uint8 op = PEN_OP_DRAWRECT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	
	buf->Write((const char *)&swap_colors, sizeof(bool));	
	buf->Write((const char *)&fill, sizeof(bool));
}

/** 
* Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
* and determines how much of the corner is mitered off and beveled. 
*/
void pen::DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
	float miter, bool swap_colors, bool fill)
{
	uint8 op = PEN_OP_DRAWMITEREDRECT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&miter, sizeof(float));
	
	buf->Write((const char *)&swap_colors, sizeof(bool));	
	buf->Write((const char *)&fill, sizeof(bool)); 
}

/** 
* Draws a rounded rectangle. The roundness value should be between
* 0.0 and 1.0, and determines how much of the corner is rounded off. 
*/
void pen::DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
	float roundness, bool swap_colors, bool fill) 
{ 
	uint8 op = PEN_OP_DRAWROUNDRECT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&roundness, sizeof(float));
	
	buf->Write((const char *)&swap_colors, sizeof(bool));	
	buf->Write((const char *)&fill, sizeof(bool));	
} 

/** 
* Draws an elliptical arc from start angle to end angle.  Angle must be
* specified in radians. The arc will be made to fit in the given box.
* If you want a circular arc, make sure the box is a square.  If you want
* a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
* angle.
*/
void pen::DrawArc(uint x1, uint y1, uint x2, uint y2, float start_angle,
	float end_angle, bool swap_colors, bool fill) 	
{ 
	uint8 op = PEN_OP_DRAWARC;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&start_angle, sizeof(float));
	buf->Write((const char *)&end_angle, sizeof(float));
	
	buf->Write((const char *)&swap_colors, sizeof(bool));	
	buf->Write((const char *)&fill, sizeof(bool));
}

/**
* Draws a triangle around the given vertices. 
*/
void pen::DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3, bool fill) 
{ 
	uint8 op = PEN_OP_DRAWTRIANGLE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&x3, sizeof(uint));
	buf->Write((const char *)&y3, sizeof(uint));	
	
	buf->Write((const char *)&fill, sizeof(bool));	
}

/**
* Writes text in the given font at the given location.
*/
void pen::Write(iFont *font, uint x1, uint y1, char *text) 
{ 
	uint8 op = PEN_OP_WRITE;
	uint len = strlen(text);
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&font, sizeof(iFont *));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&len, sizeof(uint));
	buf->Write((const char *)text, len+1); // include the zero terminator
}

/**
* Writes text in the given font, in the given box.  The alignment
* specified in h_align and v_align determine how it should be aligned.  
*/
void pen::WriteBoxed(iFont *font, uint x1, uint y1, uint x2, uint y2, 
	uint h_align, uint v_align, char *text) 
{ 
	uint8 op = PEN_OP_WRITEBOXED;
	uint len = strlen(text);
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&font, sizeof(iFont *));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&h_align, sizeof(uint));
	buf->Write((const char *)&v_align, sizeof(uint));
	buf->Write((const char *)&len, sizeof(uint));
	buf->Write((const char *)text, len+1); // include the zero terminator
}



} // end of namespace
