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
#include "ivideo/texture.h"
#include "script_manager.h"
#include "script_console.h"
#include "manager.h"
#include "color.h"
#include "gradient.h"
#include "texture.h"

#include <math.h>

#define CHECK(objname, check) \
  if (!(check)) { \
    msg.Format (objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
    ScriptCon ()->Message (msg); \
  }

enum 
{ 
  GRAD_HORZ = 0, 
  GRAD_VERT, 
  GRAD_RADIAL, 
  GRAD_LDIAG, 
  GRAD_RDIAG 
};


/// The prototype object for gradients.
static JSObject *gradient_proto_object=0;

/// The constructor for gradient objects.
static JSBool Gradient (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                        jsval *rval)	
{
  csString msg;
  csGradient *go = new csGradient;

  // Store this widget object with the new widget instance.
  CHECK("Gradient", JS_SetPrivate (cx, obj, (void *)go)==JS_TRUE);


  return JS_TRUE;
}


/// Sets a gradient. Any or all parameters may be included.
static JSBool AddColor (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                        jsval *rval)	
{
  csGradient *go = (csGradient *)JS_GetPrivate (cx, obj);

  if (JS_TypeOfValue (cx, argv[0])!=JSTYPE_VOID)
  {
    JSObject *o = JSVAL_TO_OBJECT(argv[0]);		

    if (IsColorObject (o))
    {
      csColor4 *co = (csColor4 *)JS_GetPrivate (cx, o);
      jsdouble pos;

      JS_ValueToNumber (cx, argv[1], &pos);			

      go->AddShade (csGradientShade (*co, (float)pos)); 
    }
    else
    {
      return JS_FALSE;	
    }
  }

  return JS_TRUE;
}

/// Sets a gradient. Any or all parameters may be included.
static JSBool RenderToTexture (JSContext *cx, JSObject *obj, uintN argc, 
                               jsval *argv, jsval *rval)	
{
  csGradient *go = (csGradient *)JS_GetPrivate (cx, obj);
  

  if (JS_TypeOfValue(cx, argv[0])!=JSTYPE_VOID)
  {
    JSObject *o = JSVAL_TO_OBJECT(argv[0]);		
    int style = JSVAL_TO_INT(argv[1]);
    
    if (IsTextureObject (o))
    {
      csRef<iTextureHandle> *to = (csRef<iTextureHandle> *)JS_GetPrivate (cx, o);	

      int w, h;

      // Find out how big the texture is.
      (*to)->GetOriginalDimensions (w,h);

      switch(style)
      {				
      case GRAD_HORZ: 
        { 
          // Horizontal linear					
          csRGBpixel *pbuf = new csRGBpixel[w];

          // Create the gradient
          go->Render (pbuf, w);

	     // Blit it to the texture.
          for(int i=0; i<h; ++i)
          {
            (*to)->Blit (0,i,w,1, (const unsigned char *)pbuf);							
          }				

          // Free pixel buffer
          delete [] pbuf;                
        } 
        break;

      case GRAD_VERT: 
        { 
          // Vertical linear				
          csRGBpixel *pbuf = new csRGBpixel[h];

          // Create the gradient
          go->Render (pbuf, h);					
	          
          // Blit it to the texture.
          for(int i=0; i<w; ++i)
          {
            (*to)->Blit (i,0,1,h, (const unsigned char *)pbuf);							
          }

          // Free the pixel buffer
          delete [] pbuf;              
        } 
        break;

      case GRAD_RADIAL: 
        { 
          // Radial				
          int mw=w>>1, mh=h>>1;
          int rl = ((mw+mh)>>1)+1;
          csRGBpixel *pbuf = new csRGBpixel[rl+1];

          // Create the gradient
          go->Render (pbuf, rl+1);					

          // Temporary buffer for rendering the gradient.
          csRGBpixel *tmp = new csRGBpixel[w*h];

          // Render the gradient.  
          for(int x=0; x<w; ++x)
          {
            for(int y=0; y<h; ++y)
            {
              int dx = mw - x,
                dy = mh - y;

              int dist = (int)sqrtf ((dx*dx) + (dy*dy));

              if (dist>rl) dist=rl;

              tmp[x+(y*w)] = pbuf[dist];
            }
          }

          (*to)->Blit (0,0,w,h, (const unsigned char *)tmp);

          // Free the pixel buffer
          delete [] pbuf;
          delete [] tmp;
                        
        } 
        break;

      case GRAD_LDIAG: 
        { 
          // Diagonal upper left to lower right.					
          csRGBpixel *pbuf = new csRGBpixel[w+1];
          float fw = w;

          // Create the gradient
          go->Render (pbuf, w+1);					

          // Temporary buffer for rendering the gradient.
          csRGBpixel *tmp = new csRGBpixel[w*h];

          // Render the gradient.  
          for(int x=0; x<w; ++x)
          {
            float xr = (1.0 - ((float)x / (float)w)) * 0.5f;

            for(int y=0; y<h; ++y)
            {
              float yr = ((float)y / (float)h) * 0.5f;							
              float pos = (yr + xr);

              if (pos>1.0) pos=1.0;
              if (pos<0.0) pos=0.0;

              tmp[x+(y*w)] = pbuf[(int)(pos*fw)];
            }
          }

          (*to)->Blit (0,0,w,h, (const unsigned char *)tmp);

          // Free the pixel buffer
          delete [] pbuf;
          delete [] tmp;               
        } 
        break;

      case GRAD_RDIAG: 
        { 
          // Diagonal upper right to lower left.					
          csRGBpixel *pbuf = new csRGBpixel[w+1];
          float fw = w;

          // Create the gradient
          go->Render (pbuf, w+1);					

          // Temporary buffer for rendering the gradient.
          csRGBpixel *tmp = new csRGBpixel[w*h];

          // Render the gradient.  
          for(int x=0; x<w; ++x)
          {
            float xr = ((float)x / (float)w) * 0.5f;

            for(int y=0; y<h; ++y)
            {
              float yr = ((float)y / (float)h) * 0.5f;							
              float pos = 1.0 - (yr + xr);

              if (pos>1.0) pos=1.0;
              if (pos<0.0) pos=0.0;

              tmp[x+(y*w)] = pbuf[(int)(pos*fw)];
            }
          }

          (*to)->Blit (0,0,w,h, (const unsigned char *)tmp);

          // Free the pixel buffer
          delete [] pbuf;
          delete [] tmp;
        } 
        break;

      } // end switch style			
    }
    else
    {
      return JS_FALSE;	
    }
  }

  return JS_TRUE;
}

/// Returns static properties
static JSBool gradient_get_staticProperty (JSContext *cx, JSObject *obj, 
                                           jsval id, jsval *vp)
{	
  // Try static properties first.  They can't be handled in the class because
  // They're STATIC properties.
  if (JSVAL_IS_INT(id)) 
  {				   	   	

    switch (JSVAL_TO_INT(id)) 
    {
    case GRAD_HORZ: *vp =  INT_TO_JSVAL(GRAD_HORZ); break;					
    case GRAD_VERT: *vp =  INT_TO_JSVAL(GRAD_VERT); break;									
    case GRAD_RADIAL: *vp =  INT_TO_JSVAL(GRAD_RADIAL); break;
    case GRAD_LDIAG: *vp =  INT_TO_JSVAL(GRAD_LDIAG); break;
    case GRAD_RDIAG: *vp =  INT_TO_JSVAL(GRAD_RDIAG); break;
    default:
      return JS_FALSE;				
    }

    return JS_TRUE;		
  }	

  return JS_FALSE;
}

static void FinalizeGradient(JSContext *cx, JSObject *obj)
{
    csGradient *go = (csGradient *)JS_GetPrivate (cx, obj);
        
    delete go; 	
}


JSClass gradient_object_class = {
  "Gradient", 
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  FinalizeGradient,
  JSCLASS_NO_OPTIONAL_MEMBERS 
};


static JSFunctionSpec gradient_methods[] = {
  {"AddColor",    AddColor,             2, 0, 0},    
  {"Render",      RenderToTexture,      2, 0, 0},
  {0,0,0,0,0}
};    

static JSPropertySpec gradient_static_props[] =
{
  {"HORIZONTAL",  GRAD_HORZ,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, gradient_get_staticProperty},       
  {"VERTICAL",    GRAD_VERT,    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, gradient_get_staticProperty},       
  {"RADIAL",      GRAD_RADIAL,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, gradient_get_staticProperty},       
  {"LDIAG",       GRAD_LDIAG,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, gradient_get_staticProperty},  
  {"RDIAG",       GRAD_RDIAG,   JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, gradient_get_staticProperty},  

  {0,0,0}
};


bool IsGradientObject (JSObject *obj)
{
  return JS_InstanceOf (ScriptMgr ()->GetContext (), obj,
    &gradient_object_class, NULL) == JS_TRUE;
}


void Gradient_SetupAutomation ()
{
  if (gradient_proto_object==0)
  {
    csString msg;

    CHECK("Gradient", gradient_proto_object = 
      JS_InitClass (
      ScriptMgr ()->GetContext (),
      ScriptMgr ()->GetGlobalObject (), 
      NULL /* no parent */, &gradient_object_class,

      /* native constructor function and min arg count */
      Gradient, 4,

      /* prototype object properties and methods -- these
      will be "inherited" by all instances through
      delegation up the instance's prototype link. */
      NULL, gradient_methods,

      /* class constructor (static) properties and methods */
      gradient_static_props, NULL));    


    ScriptCon ()->Message ("Gradient builtin-object initialized.");
  }
}

// newline
