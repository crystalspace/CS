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
#include <cstool/debugimagewriter.h>
#include <igraphic/image.h>
#include "manager.h"
#include "script_manager.h"
#include "script_console.h"
#include "pen.h"
#include "color.h"
#include "font.h"
#include "texture.h"

#include <string.h>

enum { MIX_ADD, MIX_ALPHA, MIX_COPY, MIX_DSTALPHAADD, MIX_FLAT, MIX_MASK_ALPHA, MIX_MASK_MIXMODE, 
	   MIX_MULTIPLY, MIX_MULTIPLY2, MIX_PREMULTALPHA, MIX_SRCALPHAADD, MIX_TRANSPARENT, MIX_TRANSPARENTTEST,
	   MIX_DSTALPHA, MIX_DSTALPHAMASK };


/// The prototype object for pens.
static JSObject *pen_proto_object=0;

/// The constructor for pen objects.
static JSBool Pen (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                   jsval *rval)	
{
  aws::pen *po = new aws::pen;

  // Store this pen object with the new pen instance.
  JS_SetPrivate (cx, obj, (void *)po);  

  // Store the object inside the pen class too.
  po->SetPenObject (obj); 

  return JS_TRUE;
}

/// Clears out the contents of the pen. 
static JSBool Clear (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                     jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) { po->Clear (); }

  return JS_TRUE;
}

/// Sets a flag
static JSBool SetFlag (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                       jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) 
  { 
    int32 flag;		

    JS_ValueToInt32 (cx,  argv[0], &flag);

    po->SetFlag ((uint)flag); 
  }

  return JS_TRUE;
}

/// Clears a flag
static JSBool ClearFlag (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                         jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) 
  { 
    int32 flag;		

    JS_ValueToInt32 (cx,  argv[0], &flag);

    po->ClearFlag ((uint)flag); 
  }

  return JS_TRUE;
}

/// Sets the mix (blending) mode
static JSBool SetMixMode (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                       jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) 
  { 
    uint32 mode;
    uint rm;

    JS_ValueToECMAUint32(cx,  argv[0], &mode);
  	   	   			  
    switch (mode) 
    {				
	    case MIX_ADD:       	rm = CS_FX_ADD; break;
	    case MIX_ALPHA:       	rm = CS_FX_ALPHA; break;
	    case MIX_COPY:       	rm = CS_FX_COPY; break;
	    case MIX_DSTALPHAADD:   rm = CS_FX_DESTALPHAADD; break;
	    case MIX_FLAT:       	  rm = CS_FX_FLAT; break;
	    case MIX_MASK_ALPHA:      rm = CS_FX_MASK_ALPHA; break;
	    case MIX_MASK_MIXMODE:    rm = CS_FX_MASK_MIXMODE; break;
	    case MIX_MULTIPLY:        rm = CS_FX_MULTIPLY; break;
	    case MIX_MULTIPLY2:       rm = CS_FX_MULTIPLY2; break;
	    case MIX_PREMULTALPHA:    rm = CS_FX_PREMULTALPHA; break;
	    case MIX_SRCALPHAADD:     rm = CS_FX_SRCALPHAADD; break;
	    case MIX_TRANSPARENT:     rm = CS_FX_TRANSPARENT; break;    
	    case MIX_TRANSPARENTTEST: rm = CS_MIXMODE_BLEND(ZERO, ONE) | CS_MIXMODE_ALPHATEST_ENABLE; break;    
	    case MIX_DSTALPHA:	  	  rm = CS_MIXMODE_BLEND(DSTALPHA, ZERO) | CS_MIXMODE_ALPHATEST_DISABLE; break;    
	    case MIX_DSTALPHAMASK:    rm = CS_MIXMODE_BLEND(DSTALPHA, SRCCOLOR)  | CS_MIXMODE_ALPHATEST_DISABLE; break;  
	   
	    default:
	      return JS_FALSE;				
    }    

    po->SetMixMode (rm); 
  }

  return JS_TRUE;
}

/// Set the color. 
static JSBool SetColor (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                        jsval *rval)	
{			
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  jsdouble r=0.0,g=0.0,b=0.0,a=1.0;

  if (JSVAL_IS_OBJECT(argv[0]))
  {
    JSObject *color_object = JSVAL_TO_OBJECT(argv[0]);

    if (IsColorObject (color_object))
    {	
      csColor4 *co = (csColor4 *)JS_GetPrivate (cx, 
        color_object); 

      r=co->red;
      g=co->green;
      b=co->blue;
      a=co->alpha;		
    }		
  }
  else
  {		
    JS_ValueToNumber (cx, argv[0], &r);
    JS_ValueToNumber (cx, argv[1], &g);
    JS_ValueToNumber (cx, argv[2], &b);
    JS_ValueToNumber (cx, argv[3], &a);
  }

  if (po) po->SetColor (r,g,b,a);	

  return JS_TRUE;
}

/// Set the texture.
static JSBool SetTexture (JSContext *cx, JSObject *obj, uintN argc, 
                          jsval *argv, jsval *rval)	
{			
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (JSVAL_IS_OBJECT(argv[0]))
  {
    JSObject *tex_object = JSVAL_TO_OBJECT(argv[0]);

    if (IsTextureObject (tex_object))
    {		
      csRef<iTextureHandle> *to = (csRef<iTextureHandle> *)
        JS_GetPrivate (cx, tex_object);	 						
      iTextureHandle *th = *to;

      if (po) po->SetTexture (th);	

      return JS_TRUE;
    }		
  }			

  return JS_FALSE;
}

/// Swaps the color of the pen with the alternate color.
static JSBool SwapColors (JSContext *cx, JSObject *obj, uintN argc, 
                          jsval *argv, jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) po->SwapColors ();		

  return JS_TRUE;
}

/// Sets width of the pen.
static JSBool SetWidth (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                        jsval *rval)	
{			
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  jsdouble width=0.0;

  JS_ValueToNumber (cx,  argv[0], &width);

  if (po) po->SetPenWidth (width);	

  return JS_TRUE;
}

/// Swaps the color of the pen with the alternate color.
static JSBool PushTransform (JSContext *cx, JSObject *obj, uintN argc, 
                             jsval *argv, jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) po->PushTransform ();		

  return JS_TRUE;
}


/// Swaps the color of the pen with the alternate color.
static JSBool PopTransform (JSContext *cx, JSObject *obj, uintN argc, 
                            jsval *argv, jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) po->PopTransform ();		

  return JS_TRUE;
}

/// Swaps the color of the pen with the alternate color.
static JSBool ClearTransform (JSContext *cx, JSObject *obj, uintN argc, 
                              jsval *argv, jsval *rval)	
{	
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) po->ClearTransform ();		

  return JS_TRUE;
}

/// Draw a line from (x1,y1) to (x2,y2)
static JSBool DrawLine (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                        jsval *rval)	
{
  if (argc<4) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1,x2,y2;

  JS_ValueToInt32 (cx, argv[0], &x1);
  JS_ValueToInt32 (cx, argv[1], &y1);
  JS_ValueToInt32 (cx, argv[2], &x2);
  JS_ValueToInt32 (cx, argv[3], &y2);	

  if (po) po->DrawLine (x1,y1,x2,y2);	

  return JS_TRUE;
}

/// Draw a rect from (x1,y1) to (x2,y2), with optional fill
static JSBool DrawRect (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                        jsval *rval)	
{
  if (argc<4) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1,x2,y2;
  //JSBool fill, swap_colors;

  JS_ValueToInt32 (cx, argv[0], &x1);
  JS_ValueToInt32 (cx, argv[1], &y1);
  JS_ValueToInt32 (cx, argv[2], &x2);
  JS_ValueToInt32 (cx, argv[3], &y2);
  //JS_ValueToBoolean(cx, argv[4], &fill);
  //JS_ValueToBoolean(cx, argv[5], &swap_colors);

  if (po) po->DrawRect (x1,y1,x2,y2);	

  return JS_TRUE;
}

/// Draw a mitered rect from (x1,y1) to (x2,y2), miter, with optional fill
static JSBool DrawMiteredRect (JSContext *cx, JSObject *obj, uintN argc, 
                               jsval *argv, jsval *rval)	
{
  if (argc<5) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1,x2,y2;
  int32 miter;
  //JSBool fill, swap_colors;

  JS_ValueToInt32 (cx, argv[0], &x1);
  JS_ValueToInt32 (cx, argv[1], &y1);
  JS_ValueToInt32 (cx, argv[2], &x2);
  JS_ValueToInt32 (cx, argv[3], &y2);
  JS_ValueToInt32 (cx, argv[4], &miter);
  // 	JS_ValueToBoolean(cx, argv[5], &fill);
  // 	JS_ValueToBoolean(cx, argv[6], &swap_colors);

  if (po) po->DrawMiteredRect (x1,y1,x2,y2,miter);	

  return JS_TRUE;
}

/// Draw a rounded rect from (x1,y1) to (x2,y2), roundness, with optional fill
static JSBool DrawRoundedRect (JSContext *cx, JSObject *obj, uintN argc, 
                               jsval *argv, jsval *rval)	
{
  if (argc<5) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1,x2,y2;
  int32 roundness;
  //JSBool fill, swap_colors;

  JS_ValueToInt32 (cx, argv[0], &x1);
  JS_ValueToInt32 (cx, argv[1], &y1);
  JS_ValueToInt32 (cx, argv[2], &x2);
  JS_ValueToInt32 (cx, argv[3], &y2);
  JS_ValueToInt32 (cx, argv[4], &roundness);
  // 	JS_ValueToBoolean(cx, argv[5], &fill);
  // 	JS_ValueToBoolean(cx, argv[6], &swap_colors);

  if (po) po->DrawRoundedRect (x1,y1,x2,y2,roundness);	

  return JS_TRUE;
}

/// Draw an arc from (x1,y1) to (x2,y2), (start_angle,end_angle), with optional fill
static JSBool DrawArc (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                       jsval *rval)	
{
  if (argc<6) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1,x2,y2;
  jsdouble start_angle, end_angle;
  //JSBool fill, swap_colors;

  JS_ValueToInt32 (cx, argv[0], &x1);
  JS_ValueToInt32 (cx, argv[1], &y1);
  JS_ValueToInt32 (cx, argv[2], &x2);
  JS_ValueToInt32 (cx, argv[3], &y2);
  JS_ValueToNumber (cx, argv[4], &start_angle);
  JS_ValueToNumber (cx, argv[5], &end_angle);
  // 	JS_ValueToBoolean(cx, argv[6], &fill);
  // 	JS_ValueToBoolean(cx, argv[7], &swap_colors);

  if (po) po->DrawArc (x1,y1,x2,y2,(float)start_angle, (float)end_angle);	

  return JS_TRUE;
}

/// Draw a triangle from (x1,y1) to (x2,y2) to (x3, y3), with optional fill
static JSBool DrawTriangle (JSContext *cx, JSObject *obj, uintN argc, 
                            jsval *argv, jsval *rval)	
{
  if (argc<6) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1,x2,y2,x3,y3;
  //JSBool fill;

  JS_ValueToInt32 (cx, argv[0], &x1);
  JS_ValueToInt32 (cx, argv[1], &y1);
  JS_ValueToInt32 (cx, argv[2], &x2);
  JS_ValueToInt32 (cx, argv[3], &y2);
  JS_ValueToInt32 (cx, argv[4], &x3);
  JS_ValueToInt32 (cx, argv[5], &y3);	
  //JS_ValueToBoolean(cx, argv[6], &fill);

  if (po) po->DrawTriangle (x1,y1,x2,y2,x3,y3);	

  return JS_TRUE;
}

/// Writes text in the given font at the given position.
static JSBool Write (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                     jsval *rval)	
{
  //csString msg;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1;	

  JSObject *fnt = JSVAL_TO_OBJECT(argv[0]);

  if (argv[0] == JSVAL_VOID || !IsFontObject (fnt))
  {
    ScriptCon ()->
      Message ("Pen: write: Object passed as parameter 0 is not a font object!");	
    return JS_FALSE;
  }

  csRef<iFont> *fo = (csRef<iFont> *)JS_GetPrivate (cx, fnt);

  JS_ValueToInt32 (cx, argv[1], &x1);
  JS_ValueToInt32 (cx, argv[2], &y1);
  JSString *text = JS_ValueToString (cx,  argv[3]);

  // 	msg.Format("Pen: write %d, %d, %s, %p, font valid: %s", x1, y1, JS_GetStringBytes(text), (iFont *)(*fo), fo->IsValid() ? "yes" : "no");
  // 	ScriptCon()->Message(msg);
  // 		
  if (po && fo->IsValid ()) 
    po->Write ((iFont *)(*fo), x1,y1,JS_GetStringBytes (text));	

  return JS_TRUE;
}

/// Writes text in the given font aligned in the given box.
static JSBool WriteBoxed (JSContext *cx, JSObject *obj, uintN argc,
                          jsval *argv, jsval *rval)	
{
  //csString msg;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  int32 x1,y1,x2,y2,h_align,v_align;	

  JSObject *fnt = JSVAL_TO_OBJECT(argv[0]);

  if (argv[0] == JSVAL_VOID || !IsFontObject (fnt))
  {
    ScriptCon ()->
      Message ("Pen: write: Object passed as parameter 0 is not a font object!");	
    return JS_FALSE;
  }

  csRef<iFont> *fo = (csRef<iFont> *)JS_GetPrivate (cx, fnt);

  JS_ValueToInt32 (cx, argv[1], &x1);
  JS_ValueToInt32 (cx, argv[2], &y1);
  JS_ValueToInt32 (cx, argv[3], &x2);
  JS_ValueToInt32 (cx, argv[4], &y2);
  JS_ValueToInt32 (cx, argv[5], &h_align);
  JS_ValueToInt32 (cx, argv[6], &v_align);

  JSString *text = JS_ValueToString (cx,  argv[7]);

  // 	msg.Format("Pen: write %d, %d, %s, %p, font valid: %s", x1, y1, JS_GetStringBytes(text), (iFont *)(*fo), fo->IsValid() ? "yes" : "no");
  // 	ScriptCon()->Message(msg);
  // 		
  if (po && fo->IsValid ()) 
    po->WriteBoxed ((iFont *)(*fo), x1,y1,x2,y2,h_align,v_align,
    JS_GetStringBytes (text));	

  return JS_TRUE;
}

/// Draw this pen into another pen.
static JSBool Draw (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                    jsval *rval)	
{			
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (JSVAL_IS_OBJECT(argv[0]))
  {
    JSObject *pen_object = JSVAL_TO_OBJECT(argv[0]);

    if (IsPenObject (pen_object))
    {	
      aws::pen *other_pen = (aws::pen *)JS_GetPrivate (cx, pen_object); 					

      if (po) po->Draw (other_pen);	
    }		
  }



  return JS_TRUE;
}


/// Move a pen by (x,y)
static JSBool Translate (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                         jsval *rval)	
{
  if (argc<2) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  jsdouble x,y,z;

  JS_ValueToNumber (cx, argv[0], &x);
  JS_ValueToNumber (cx, argv[1], &y);
  JS_ValueToNumber (cx, argv[2], &z);

  if (po) po->Translate (csVector3 ((float)x, (float)y, (float)z));	

  return JS_TRUE;
}

/// Move a pen by (x,y)
static JSBool SetOrigin (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                         jsval *rval)	
{
  if (argc<2) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  jsdouble x,y,z;

  JS_ValueToNumber (cx, argv[0], &x);
  JS_ValueToNumber (cx, argv[1], &y);
  JS_ValueToNumber (cx, argv[2], &z);

  if (po) po->SetOrigin (csVector3 ((float)x, (float)y, (float)z));	

  return JS_TRUE;
}

/// Rotate a pen by (a)
static JSBool Rotate (JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
                      jsval *rval)	
{
  if (argc<1) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  jsdouble a;

  JS_ValueToNumber (cx, argv[0], &a);	

  if (po) po->Rotate ((float)a);	

  return JS_TRUE;
}

/// Rotate a pen by (a)
static JSBool Render (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{
  if (argc<1) return JS_FALSE;

  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po && JS_TypeOfValue (cx, argv[0])!=JSTYPE_VOID)
  {
    JSObject *o = JSVAL_TO_OBJECT(argv[0]);		

    if (IsTextureObject (o))
    {
      csRef<iTextureHandle> *to = 
        (csRef<iTextureHandle> *)JS_GetPrivate (cx, o);	
        
      int w,h,d;		

      (*to)->GetOriginalDimensions (w,h,d);

      csPen pen(AwsMgr ()->G2D (), AwsMgr ()->G3D ()); 

      AwsMgr ()->G3D ()->SetRenderTarget (*to, true);
      AwsMgr ()->G3D ()->BeginDraw (CSDRAW_2DGRAPHICS);
      // Clear it out to make sure that we get good, clean backgroundess.
      AwsMgr ()->G2D ()->Clear (AwsMgr ()->G2D ()->FindRGB (0,0,0,0));
      
//    csRef<iImage> shot = AwsMgr ()->G2D ()->ScreenShot();
// 	  csDebugImageWriter::DebugImageWrite(shot, "post-clear.png");      
            
      po->Draw (&pen);			
      
//       shot = AwsMgr ()->G2D ()->ScreenShot();
//       csDebugImageWriter::DebugImageWrite(shot, "post-draw.png");
            
      AwsMgr ()->G3D ()->FinishDraw ();			
      
      return JS_TRUE;
    }	
    
    // @todo Throw a JS error here so they know that the object isn't a texture!    
  }

  return JS_FALSE;
}


/// Forewards a pen GetProperty call.
static JSBool pen_getProperty (JSContext *cx, JSObject *obj, jsval id, 
                               jsval *vp)
{
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) po->GetProperty (cx, obj, id, vp);		

  return JS_TRUE;
}

/// Forewards a pen SetProperty call.
static JSBool pen_setProperty (JSContext *cx, JSObject *obj, jsval id, 
                               jsval *vp)
{
  aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);

  if (po) po->SetProperty (cx, obj, id, vp);		

  return JS_TRUE;
}

/// Returns static properties
static JSBool pen_get_staticProperty (JSContext *cx, JSObject *obj, jsval id, 
                                      jsval *vp)
{	
  // Try static properties first.  They can't be handled in the class because
  // They're STATIC properties.
  if (JSVAL_IS_INT(id)) 
  {				   	   	

    switch (JSVAL_TO_INT(id)) 
    {
    case CS_PEN_TA_TOP: *vp =  INT_TO_JSVAL(CS_PEN_TA_TOP); break;					
    case CS_PEN_TA_BOT: *vp =  INT_TO_JSVAL(CS_PEN_TA_BOT); break;					
    case CS_PEN_TA_LEFT: *vp =  INT_TO_JSVAL(CS_PEN_TA_LEFT); break;					
    case CS_PEN_TA_RIGHT: *vp =  INT_TO_JSVAL(CS_PEN_TA_RIGHT); break;					
    case CS_PEN_TA_CENTER: *vp =  INT_TO_JSVAL(CS_PEN_TA_CENTER); break;													

    default:
      return JS_FALSE;				
    }

    return JS_TRUE;		
  }	

  return JS_FALSE;
}

/// Returns static properties
static JSBool pen_get_staticFlagsProperty (JSContext *cx, JSObject *obj, 
                                           jsval id, jsval *vp)
{	
  // Try static properties first.  They can't be handled in the class because
  // They're STATIC properties.
  if (JSVAL_IS_INT(id)) 
  {				   	   	

    switch (JSVAL_TO_INT(id)) 
    {				
    case CS_PEN_FILL:       *vp = INT_TO_JSVAL(CS_PEN_FILL); break;
    case CS_PEN_SWAPCOLORS: *vp = INT_TO_JSVAL(CS_PEN_SWAPCOLORS); break;
    case CS_PEN_TEXTURE:    *vp = INT_TO_JSVAL(CS_PEN_TEXTURE); break;
    default:
      return JS_FALSE;				
    }

    return JS_TRUE;		
  }	

  return JS_FALSE;
}


/// Returns static properties
static JSBool pen_get_staticMixProperty (JSContext *cx, JSObject *obj, 
                                           jsval id, jsval *vp)
{	
  // Try static properties first.  They can't be handled in the class because
  // They're STATIC properties.
  if (JSVAL_IS_INT(id)) 
  {			
	  
	*vp = id;  
	  	   	   			  
//     switch (JSVAL_TO_INT(id)) 
//     {				
//     case MIX_ADD:       	*vp = INT_TO_JSVAL(CS_FX_ADD); break;
//     case MIX_ALPHA:       	*vp = INT_TO_JSVAL(CS_FX_ALPHA); break;
//     case MIX_COPY:       	*vp = INT_TO_JSVAL(CS_FX_COPY); break;
//     case MIX_DSTALPHAADD:   *vp = INT_TO_JSVAL(CS_FX_DESTALPHAADD); break;
//     case MIX_FLAT:       	  *vp = INT_TO_JSVAL(CS_FX_FLAT); break;
//     case MIX_MASK_ALPHA:      *vp = INT_TO_JSVAL(CS_FX_MASK_ALPHA); break;
//     case MIX_MASK_MIXMODE:    *vp = INT_TO_JSVAL(CS_FX_MASK_MIXMODE); break;
//     case MIX_MULTIPLY:        *vp = INT_TO_JSVAL(CS_FX_MULTIPLY); break;
//     case MIX_MULTIPLY2:       *vp = INT_TO_JSVAL(CS_FX_MULTIPLY2); break;
//     case MIX_PREMULTALPHA:    *vp = INT_TO_JSVAL(CS_FX_PREMULTALPHA); break;
//     case MIX_SRCALPHAADD:     *vp = INT_TO_JSVAL(CS_FX_SRCALPHAADD); break;
//     case MIX_TRANSPARENT:     *vp = INT_TO_JSVAL(CS_FX_TRANSPARENT); break;    
//     case MIX_TRANSPARENTTEST: *vp = INT_TO_JSVAL(CS_MIXMODE_BLEND(ZERO, ONE) | CS_MIXMODE_ALPHATEST_ENABLE); break;    
//     case MIX_DSTALPHATEST:	  *vp = INT_TO_JSVAL(CS_MIXMODE_BLEND(DSTALPHA, ZERO) | CS_MIXMODE_ALPHATEST_ENABLE); break;    
//     case MIX_DSTALPHAADDTEST: *vp = INT_TO_JSVAL(CS_MIXMODE_BLEND(DSTALPHA, ONE)  | CS_MIXMODE_ALPHATEST_ENABLE); break;  
//    
//     default:
//       return JS_FALSE;				
//     }

    return JS_TRUE;		
  }	

  return JS_FALSE;
}

static void FinalizePen(JSContext *cx, JSObject *obj)
{
    aws::pen *po = (aws::pen *)JS_GetPrivate (cx, obj);
                
    delete po; 	
}


JSClass pen_object_class = {
  "Pen", 
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
  JS_PropertyStub,
  pen_getProperty,
  pen_setProperty,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  FinalizePen 
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


static JSPropertySpec pen_static_props[] =
{
  {"ALIGN_TOP",       CS_PEN_TA_TOP,      JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticProperty},       
  {"ALIGN_BOT",       CS_PEN_TA_BOT,      JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticProperty},
  {"ALIGN_LEFT",      CS_PEN_TA_LEFT,     JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticProperty},
  {"ALIGN_RIGHT",     CS_PEN_TA_RIGHT,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticProperty},
  {"ALIGN_CENTER",    CS_PEN_TA_CENTER,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticProperty},

  {"FLAG_FILL",       CS_PEN_FILL,        JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticFlagsProperty},
  {"FLAG_SWAPCOLORS", CS_PEN_SWAPCOLORS,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticFlagsProperty},
  {"FLAG_TEXTURE",    CS_PEN_TEXTURE,     JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticFlagsProperty},
  
  {"MIX_ADD",         MIX_ADD,     JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_ALPHA",       MIX_ALPHA,     JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_COPY",        MIX_COPY,     JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},  
  {"MIX_DST_ALPHA_ADD",  MIX_DSTALPHAADD,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_FLAT",           MIX_FLAT,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_MASK_ALPHA",     MIX_MASK_ALPHA,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_MASK_MIXMODE",   MIX_MASK_MIXMODE,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_MULTIPLY",       MIX_MULTIPLY,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_MULTIPLY2",      MIX_MULTIPLY2,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_PRE_MULT_ALPHA", MIX_PREMULTALPHA, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_SRC_ALPHA_ADD",  MIX_SRCALPHAADD, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_TRANSPARENT",    MIX_TRANSPARENT, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_TRANSPARENT_TEST",    MIX_TRANSPARENTTEST, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_DST_ALPHA",      	  MIX_DSTALPHA, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},
  {"MIX_DST_ALPHA_MASK",  MIX_DSTALPHAMASK, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, pen_get_staticMixProperty},

  {0,0,0}
};

static JSFunctionSpec pen_methods[] = {
  {"Clear",           Clear,            0, 0, 0},
  {"SetFlag",         SetFlag,          1, 0, 0},
  {"ClearFlag",       ClearFlag,        1, 0, 0},
  {"SetMixMode",      SetMixMode,       1, 0, 0},
  {"SetColor",        SetColor,         4, 0, 0},
  {"SetTexture",      SetTexture,       1, 0, 0},
  {"SwapColors",      SwapColors,       0, 0, 0}, 
  {"SetWidth",        SetWidth,         1, 0, 0}, 

  {"PushTransform",   PushTransform,    0, 0, 0}, 
  {"PopTransform",    PopTransform,     0, 0, 0}, 
  {"ClearTransform",  ClearTransform,   0, 0, 0}, 

  {"SetOrigin",       SetOrigin,        3, 0, 0},
  {"Translate",       Translate,        3, 0, 0},
  {"Rotate",          Rotate,           1, 0, 0},   

  {"DrawLine",        DrawLine,         4, 0, 0},
  {"DrawRect",        DrawRect,         6, 0, 0},    
  {"DrawMiteredRect", DrawMiteredRect,  7, 0, 0},
  {"DrawRoundedRect", DrawRoundedRect,  7, 0, 0},
  {"DrawArc",         DrawArc,          8, 0, 0},
  {"DrawTriangle",    DrawTriangle,     7, 0, 0},

  {"Draw",            Draw,             1, 0, 0},
  {"Render",          Render,           1, 0, 0},

  {"Write",           Write,            4, 0, 0},    
  {"WriteBoxed",      WriteBoxed,       8, 0, 0},


  //{"Invalidate",	Invalidate,	0, 0, 0},    
  {0,0,0,0,0}
};    

bool IsPenObject (JSObject *obj)
{
  return JS_InstanceOf (ScriptMgr ()->GetContext (), obj, 
    &pen_object_class, NULL) == JS_TRUE;
}

void Pen_SetupAutomation ()
{
  if (pen_proto_object==0)
  {				
    pen_proto_object = 
      JS_InitClass (
      ScriptMgr ()->GetContext (),
      ScriptMgr ()->GetGlobalObject (), 
      NULL /* no parent */, &pen_object_class,

      /* native constructor function and min arg count */
      Pen, 0,

      /* prototype object properties and methods -- these
      will be "inherited" by all instances through
      delegation up the instance's prototype link. */
      NULL /*pen_props*/, pen_methods,

      /* class constructor (static) properties and methods */
      pen_static_props, NULL); 

    ScriptCon ()->Message ("Pen builtin-object initialized.");   
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




} // end of namespace
