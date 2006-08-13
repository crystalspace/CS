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
#include "texture.h"
#include "manager.h"

#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iutil/vfs.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "iutil/databuff.h"
#include "csgfx/imagememory.h"


#define CHECK(objname, check) \
  if (!(check)) { \
    msg.Format(objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
    ScriptCon ()->Message (msg); \
  }


/// The prototype object for textures.
static JSObject *texture_proto_object=0;

/// The constructor for color objects.
static JSBool Texture (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                       jsval *rval)	
{
  csString msg;

  if (JSVAL_IS_OBJECT(argv[0]))
  {
    JSObject *texture_object = JSVAL_TO_OBJECT(argv[0]);

    if (IsTextureObject (texture_object))
    {		
      // Copy constructor.	
      csRef<iTextureHandle> *fo = 
        (csRef<iTextureHandle> *)JS_GetPrivate (cx, texture_object);

      // Store this widget object with the new widget instance.
      CHECK("Texture", JS_SetPrivate (cx, obj, (void *)fo)==JS_TRUE);  	
    }
  }
  else if (JSVAL_IS_NUMBER(argv[0]))
  {
    int32 w, h;

    JS_ValueToInt32 (cx, argv[0], &w);
    JS_ValueToInt32 (cx, argv[1], &h);

    csImageMemory *img = new csImageMemory (w,h, 
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
      
    csRGBpixel cc(0,0,0,0);
    
    img->Clear(cc);

    csRef<iTextureHandle> *to = 
      new csRef<iTextureHandle>(AwsMgr ()->G3D ()->GetTextureManager ()->
        RegisterTexture (img, CS_TEXTURE_2D | CS_TEXTURE_3D));
        
    /* Disable compression on the texture.
     * \todo FIXME Really added so gradients look nice, but is this the right
     *   place?
     */
    (*to)->SetTextureClass ("nocompress");

    if (!to)
    {
      msg.Format ("Texture: Could not register raw texture with the texture manager.");
      ScriptCon ()->Message (msg);	

      return JS_FALSE;
    }		

    // Store this texture object with the new texture instance.
    CHECK("Texture", JS_SetPrivate (cx, obj, (void *)to)==JS_TRUE);  		
  }
  else
  {
    csString texture_name;
    csString ms;

    JSString *name = JS_ValueToString (cx, argv[0]);

    texture_name = JS_GetStringBytes (name);

    // Get a handle to the VFS.		
    csRef<iVFS> vfs (CS_QUERY_REGISTRY (AwsMgr ()->GetObjectRegistry (), iVFS));

    if (!vfs)
    {
      ScriptCon ()->Message ("Texture: error: Unable to load VFS plugin.");
      return JS_FALSE;
    }

    // Get a handle to the image loader.
    csRef<iImageIO> loader = CS_QUERY_REGISTRY (
      AwsMgr ()->GetObjectRegistry (), iImageIO);

    if (!loader)
    {
      ScriptCon ()->Message ("Texture: error: Unable to load ImageIO plugin.");
      return JS_FALSE;
    }

    // Load the texture rigamaroll						
    iTextureManager *txtmgr = AwsMgr ()->G3D ()->GetTextureManager ();
    int Format = txtmgr->GetTextureFormat ();

    // Get the file data		
    csRef<iDataBuffer> buf(vfs->ReadFile (texture_name, false));						
    if (!buf)
    {
      msg.Format ("Texture: Could not load '%s' because the file could not be read.", 
        texture_name.GetDataSafe ());
      ScriptCon ()->Message (msg);	

      return JS_FALSE;
    }		

    // Interpret the image data.
    csRef<iImage> ifile = loader->Load (buf, Format);
    if (!ifile)
    {
      msg.Format ("Texture: Could not load '%s', the image format is unknown.", 
        texture_name.GetDataSafe ());
      ScriptCon ()->Message (msg);	

      return JS_FALSE;
    }

    csRef<iTextureHandle> *to = 
      new csRef<iTextureHandle>(txtmgr->RegisterTexture (ifile, 
      CS_TEXTURE_2D | CS_TEXTURE_3D));

    if (!to)
    {
      msg.Format ("Texture: Could not register '%s' with the texture manager.", 
        texture_name.GetDataSafe ());
      ScriptCon ()->Message (msg);	

      return JS_FALSE;
    }

    // Store this widget object with the new widget instance.
    CHECK("Texture", JS_SetPrivate(cx, obj, (void *)to)==JS_TRUE);  		
  }			


  return JS_TRUE;
}


/// Used to return the dimensions a texture.
JSClass texture_dimensions_object_class = {
  "TextureDimensions", 
  0,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  JS_FinalizeStub,
  JSCLASS_NO_OPTIONAL_MEMBERS 
};

/// Return the width, height, and descender of text written with this font. 
static JSBool GetDimensions (JSContext *cx, JSObject *obj, uintN argc, 
                             jsval *argv, jsval *rval)	
{
  csRef<iTextureHandle> *to = (csRef<iTextureHandle> *)JS_GetPrivate (cx, obj);		

  if (to)	
  {		
    int w,h,d;
    jsval fw, fh,fd;

    (*to)->GetOriginalDimensions (w,h,d);

    JSObject *rv = JS_NewObject (cx, &texture_dimensions_object_class, NULL, NULL);

    fw = INT_TO_JSVAL(w);
    fh = INT_TO_JSVAL(h);
    fd = INT_TO_JSVAL(d);		

    JS_SetProperty (cx, rv, "width", &fw);
    JS_SetProperty (cx, rv, "height", &fh);
    JS_SetProperty (cx, rv, "depth", &fd);

    *rval = OBJECT_TO_JSVAL(rv);
  }

  return JS_TRUE;
}


/// Loads a texture object from disk.
static JSBool Load (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                    jsval *rval)	
{	
  return Texture (cx, obj, argc, argv, rval);
}

/// Creates a new texture object.
static JSBool Create (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  	
  return Texture (cx, obj, argc, argv, rval);
}

static void FinalizeTexture(JSContext *cx, JSObject *obj)
{
  csRef<iTextureHandle> *to = (csRef<iTextureHandle> *)JS_GetPrivate (cx, obj);		
  
  delete to;
}


JSClass texture_object_class = {
  "Texture", 
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  FinalizeTexture,
  JSCLASS_NO_OPTIONAL_MEMBERS 
};


// static JSPropertySpec texture_props[] =
// {
//         {"r",       COLOR_R,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"g",       COLOR_G,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"b",      	COLOR_B,   	   JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"alpha",   COLOR_ALPHA,   JSPROP_ENUMERATE | JSPROP_PERMANENT},        
//         {0,0,0}
// };

static JSFunctionSpec texture_methods[] = {
  {"GetDimensions",     GetDimensions,    0, 0, 0}, 
  {0,0,0,0,0}
};    

static JSFunctionSpec texture_static_methods[] = {
  {"Load",              Load,             1, 0, 0},    
  {"Create",            Create,           2, 0, 0},    
  {0,0,0,0,0}
}; 


bool IsTextureObject (JSObject *obj)
{
  return JS_InstanceOf (ScriptMgr ()->GetContext (), obj, 
    &texture_object_class, NULL) == JS_TRUE;
}


void Texture_SetupAutomation ()
{
  if (texture_proto_object==0)
  {
    csString msg;

    CHECK("Texture", texture_proto_object = 
      JS_InitClass (
      ScriptMgr ()->GetContext (),
      ScriptMgr ()->GetGlobalObject (), 
      NULL /* no parent */, &texture_object_class,

      /* native constructor function and min arg count */
      Texture, 4,

      /* prototype object properties and methods -- these
      will be "inherited" by all instances through
      delegation up the instance's prototype link. */
      NULL /*texture_props*/, texture_methods,

      /* class constructor (static) properties and methods */
      NULL, texture_static_methods));    


    ScriptCon ()->Message ("Texture builtin-object initialized.");
  }
}

// newline
