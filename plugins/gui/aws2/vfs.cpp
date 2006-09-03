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
#include "vfs.h"
#include "manager.h"

#include "iutil/vfs.h"
#include "iutil/databuff.h"
#include "iutil/stringarray.h"


#define CHECK(objname, check) \
  if (!(check)) { \
    msg.Format(objname ": failed on " #check " in\n   %s:%s:%d", __FILE__, __FUNCTION__, __LINE__); \
    ScriptCon ()->Message (msg); \
  }


/// The prototype object for vfss.
static JSObject *vfs_proto_object=0;

/// The constructor for vfs objects.
static JSBool Vfs (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                       jsval *rval)	
{
	csString msg;
	
	csString vfs_name;
	csString ms;
	
	JSString *name = JS_ValueToString (cx, argv[0]);
	
	vfs_name = JS_GetStringBytes (name);
	
	// Get a handle to the VFS.		
	csRef<iVFS> *to = new csRef<iVFS>((CS_QUERY_REGISTRY (AwsMgr ()->GetObjectRegistry (), iVFS)));
	
	if (!(to->IsValid()))
	{
	  ScriptCon ()->Message ("Vfs: error: Unable to load VFS plugin.");
	  return JS_FALSE;
	}
		
	// Store this widget object with the new widget instance.
	CHECK("Vfs", JS_SetPrivate(cx, obj, (void *)to)==JS_TRUE);  		
	
	return JS_TRUE;
}

/// Change to the given directory
static JSBool ChDir (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  		
   
     csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
        
     JSString *name = JS_ValueToString (cx, argv[0]);
        
     return (*vfso)->ChDir(JS_GetStringBytes(name)) ? JS_TRUE : JS_FALSE;     
}

/// Convenience macro to set a value to a string.
#define SET_STRING(source_str, dest) \
{  \
     size_t len = strlen(source_str);\
     char *bytes = (char *)JS_malloc(cx, len);\
     strcpy(bytes, source_str); \
     JSString *_tmp_ = JS_NewString(cx, bytes, len);     \
     (dest) = STRING_TO_JSVAL(_tmp_);     \
  }
    
/// Get the current working directory
static JSBool GetCwd (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  		
   
     csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
        
     const char *_tmp =  (*vfso)->GetCwd();
     
     SET_STRING(_tmp, *rval);     
          
     return JS_TRUE;    
}

/// Pops the path from the stack
static JSBool PopDir (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  		
   
     csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
        
     (*vfso)->PopDir();
                    
     return JS_TRUE;    
}

/// Pushes the path onto the stack
static JSBool PushDir (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  		
   
     csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
       
     JSString *name = JS_ValueToString (cx, argv[0]);
     
     (*vfso)->PushDir(JS_GetStringBytes(name));
                    
     return JS_TRUE;    
}

/// Returns true if the path/file exists, false otherwise
static JSBool Exists (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  		   
     csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
        
     JSString *name = JS_ValueToString (cx, argv[0]);
        
     *rval = BOOLEAN_TO_JSVAL(((*vfso)->Exists(JS_GetStringBytes(name)) ? JS_TRUE : JS_FALSE));
     
	return JS_TRUE;
}

/// Finds all the files in a given path.
static JSBool FindFiles (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  		   
     csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
        
     JSString *name = JS_ValueToString (cx, argv[0]);
        
     csRef<iStringArray> files = (*vfso)->FindFiles(JS_GetStringBytes(name));
       
     jsval *entries = new jsval[files->GetSize()];
     
     // Set each of the entries to the string.
     for(size_t i=0; i<files->GetSize(); ++i)
     {
	     SET_STRING(files->Get(i), entries[i]);	     
     }
     
     // Create a new array object
     JSObject *array = JS_NewArrayObject(cx, (jsint)files->GetSize(), entries);
     
     // Return the object
     *rval = OBJECT_TO_JSVAL(array);

     // Free the storage
	 delete [] entries;
     
	return JS_TRUE;
}

/// Returns true if the path/file exists, false otherwise
static JSBool GetFileSize (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                      jsval *rval)	
{	  	 	  		   
     csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
        
     JSString *name = JS_ValueToString (cx, argv[0]);
     size_t size=0;
        
     (*vfso)->GetFileSize(JS_GetStringBytes(name), size);
     
     *rval = INT_TO_JSVAL(size);
     
	return JS_TRUE;
}

static void FinalizeVfs(JSContext *cx, JSObject *obj)
{
    csRef<iVFS> *vfso = 
        (csRef<iVFS> *)JS_GetPrivate (cx, obj);
        
    delete vfso; 	
}


JSClass vfs_object_class = {
  "Vfs", 
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  FinalizeVfs,
  JSCLASS_NO_OPTIONAL_MEMBERS 
};


// static JSPropertySpec vfs_props[] =
// {
//         {"r",       COLOR_R,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"g",       COLOR_G,       JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"b",      	COLOR_B,   	   JSPROP_ENUMERATE | JSPROP_PERMANENT},
//         {"alpha",   COLOR_ALPHA,   JSPROP_ENUMERATE | JSPROP_PERMANENT},        
//         {0,0,0}
// };

static JSFunctionSpec vfs_methods[] = {
  {"ChDir",      ChDir,     1, 0, 0}, 
  {"GetCwd",     GetCwd,    0, 0, 0}, 
  {"PopDir",     PopDir, 	0, 0, 0},  
  {"PushDir",    PushDir,   1, 0, 0},
  {"Exists",     Exists,    1, 0, 0}, 
  {"FindFiles",  FindFiles, 1, 0, 0},  
  {"GetFileSize",  GetFileSize, 1, 0, 0},
  
  {0,0,0,0,0}
};    

// static JSFunctionSpec vfs_static_methods[] = {
//   {"Load",              Load,             1, 0, 0},    
//   {"Create",            Create,           2, 0, 0},    
//   {0,0,0,0,0}
// }; 


bool IsVfsObject (JSObject *obj)
{
  return JS_InstanceOf (ScriptMgr ()->GetContext (), obj, 
    &vfs_object_class, NULL) == JS_TRUE;
}


void Vfs_SetupAutomation ()
{
  if (vfs_proto_object==0)
  {
    csString msg;

    CHECK("Vfs", vfs_proto_object = 
      JS_InitClass (
      ScriptMgr ()->GetContext (),
      ScriptMgr ()->GetGlobalObject (), 
      NULL /* no parent */, &vfs_object_class,

      /* native constructor function and min arg count */
      Vfs, 4,

      /* prototype object properties and methods -- these
      will be "inherited" by all instances through
      delegation up the instance's prototype link. */
      NULL /*vfs_props*/, vfs_methods,

      /* class constructor (static) properties and methods */
      NULL, NULL /*vfs_static_methods*/));    


    ScriptCon ()->Message ("Vfs builtin-object initialized.");
  }
}

// newline
