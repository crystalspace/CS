 #ifndef __AWS_PREFERENCES_H__
 #define __AWS_PREFERENCES_H__
/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson 
    
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
*****************************************************************************/
#include "iaws/aws.h"
#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "csutil/csdllist.h"
#include "csutil/csvector.h"

#include "awstex.h"

/** global variables for the scanner (XXX: This is ugly a c++ scanner would be
 * nicer
 */
struct iVFS;
struct iFile;
extern iFile *aws_fileinputvfs;

struct iString;

/****

 This is the pseudo-symbol table for the definitions keeper.  Windows and their sub-keys can be looked up from here.
There are a few different types of values possible for keys:  Strings, Integers, and Rects.  They can be looked up
using appropriate search methods in the main preferences.  Skins and Windows are containers which hold the keys, and
the prefs manager contains those skin and window defintions.  
 
 Windows can be filled in because components provide a factory service by registering, and then know how to get their
settings from the window definition.
                               
 ****/

//////////////////////////////////  Keys  //////////////////////////////////////////////////////////////////////////////

const unsigned char KEY_INT  = 0;
const unsigned char KEY_STR  = 1;
const unsigned char KEY_RECT = 2;
const unsigned char KEY_WIN  = 3;
const unsigned char KEY_SKIN = 4;
const unsigned char KEY_COMPONENT = 5;
const unsigned char KEY_RGB  = 6;
const unsigned char KEY_POINT= 7;
const unsigned char KEY_CONNECTION = 8;
const unsigned char KEY_CONNECTIONMAP = 9;


/// Abstract key interface
class awsKey
{
  /// The name of the key
  unsigned long name; 
  //iString *name;

public:
  /// Simple constructor creates new key with name "n"
  awsKey(iString *n);

  /// Simple destructor does nothing
  virtual ~awsKey()         {};
  
  /// Pure virtual function Type returns the type of key
  virtual unsigned char Type()=0;

  /// Accessor function gets name of key
  unsigned long Name() { return name; }
  
};

class awsIntKey : public awsKey
{
  /// The key's value
  int val;

public:
  /// Constructs an integer key with the given name
  awsIntKey(iString *name, int v):awsKey(name), val(v) {};

  /// Destructor does nothing
  virtual ~awsIntKey() {};

  /// So that we know it's an int key
  virtual unsigned char Type() 
  { return KEY_INT; }

  /// Gets the value of this key as an integer
  int Value() { return val; }
};

class awsStringKey : public awsKey
{
  /// The key's value
  iString *val;

public:
  /// Constructs a string key with the given name
  awsStringKey(iString *name, iString *v):awsKey(name), val(v) {};

  /// Destructor does nothing
  virtual ~awsStringKey() {};

  /// So that we know it's a string key.
  virtual unsigned char Type() 
  { return KEY_STR; }

  /// Gets the value of this key as an iString
  iString *Value() { return val; }
};

class awsRectKey : public awsKey
{
  /// The key's value
  csRect val;

public:
  /// Constructs an integer key with the given name
  awsRectKey(iString *name, csRect v):awsKey(name), val(v) {};

  /// Destructor does nothing
  virtual ~awsRectKey() {};

  /// So that we know this is a rect key 
  virtual unsigned char Type() 
  { return KEY_RECT; }

  /// Gets the value of this key as a rectangle
  csRect Value() { return val; }
};

class awsRGBKey : public awsKey
{
public:

  /// The key's value
  struct RGB {
  	unsigned char red, green, blue;
  } rgb;
  
	/// Constructs an integer key with the given name
  awsRGBKey(iString *name, unsigned char r, unsigned char g, unsigned char b):awsKey(name)
  { rgb.red=r; rgb.green=g; rgb.blue=b; }

  /// Destructor does nothing
  virtual ~awsRGBKey() {};

  /// So that we know this is a rect key 
  virtual unsigned char Type() 
  { return KEY_RGB; }

  /// Gets the value of this key as a rectangle
  awsRGBKey::RGB& Value() { return rgb; }
};

class awsPointKey : public awsKey
{
  /// The key's value
  csPoint val;

public:
  /// Constructs an integer key with the given name
  awsPointKey(iString *name, csPoint v):awsKey(name), val(v) {};

  /// Destructor does nothing
  virtual ~awsPointKey() {};

  /// So that we know this is a rect key 
  virtual unsigned char Type() 
  { return KEY_POINT; }

  /// Gets the value of this key as a rectangle
  csPoint Value() { return val; }
};

class awsConnectionKey : public awsKey
{
  /// The sink that we want
  iAwsSink *sink;
  /// The trigger that we want
  unsigned long trigger;
  /// The signal that we want
  unsigned long signal;
  
public:
  /// Constructs an integer key with the given name
  awsConnectionKey(iString *name, iAwsSink *s, unsigned long t, unsigned long sig):
      awsKey(name), sink(s), trigger(t), signal(sig) {};

  /// Destructor does nothing
  virtual ~awsConnectionKey() {};

  /// So that we know this is a rect key 
  virtual unsigned char Type() 
  { return KEY_CONNECTION; }

  /// Gets the sink for this key
  iAwsSink *Sink() { return sink; }
  
  /// Gets the trigger for this key
  unsigned long Trigger() { return trigger; }

  /// Gets the signal for this key
  unsigned long Signal()  { return signal;  }
};


//////////////////////////////////  Containers ////////////////////////////////////////////////////////////////////////

class awsKeyContainer 
{
  /// list of children in container.
  csBasicVector children;

public:
  awsKeyContainer() {};
  virtual ~awsKeyContainer() {};

public:
  /// Looks up a key based on it's name.
  awsKey *Find(iString *name);

  /// Looks up a key based on it's ID.
  awsKey *Find(unsigned long id);
   
  csBasicVector &Children()
  { return children; }
 
  /// Adds an item to the container
  void Add(awsKey *key) 
  { children.Push(key); }

  /// Removes an item from the container
  void Remove(iString *name)
  { children.Delete(children.Find(Find(name))); }

  /// Removes a specific item from the container
  void Remove(awsKey *key)
  { children.Delete(children.Find(key)); }

  /// Consumes an entire list by moving all of it's member's to this one, and removing them from it.
  void Consume(awsKeyContainer *c);
};

class awsSkinNode : public awsKey, awsKeyContainer
{
public:  
    awsSkinNode(iString *name):awsKey(name) {};
    virtual ~awsSkinNode() {};

    int Length()
    { return Children().Length(); }

    awsKey *GetAt(int i)
    { return (awsKey *)Children()[i]; }
    
    /// So that we know this is a skin node
    virtual unsigned char Type() 
    { return KEY_SKIN; }
};

class awsComponentNode : public awsKey, awsKeyContainer
{
  /// The type of component, like "Radio Button", "Check Box", etc.
  iString *comp_type;

public:  
  awsComponentNode(iString *name, iString *component_type):awsKey(name), comp_type(component_type) {};
  virtual ~awsComponentNode() {};
    
  /// So that we know this is a component node
  virtual unsigned char Type() 
  { return KEY_COMPONENT; }
    
  /// So that we can find out what sort of component type this should be
  iString *ComponentTypeName()
  { return comp_type; }
    
  /// Exposes length of child list for iteration
  int GetLength()
  { return Children().Length(); }
  
  /// Exposes [] for index access
  awsKey *GetItemAt(int i)
  { return (awsKey *)Children()[i];  }
};

class awsConnectionNode : public awsKey, awsKeyContainer
{
  
public:  
  awsConnectionNode();
  virtual ~awsConnectionNode();
    
  /// So that we know this is a component node
  virtual unsigned char Type() 
  { return KEY_CONNECTIONMAP; }
      
  /// Exposes length of child list for iteration
  int GetLength()
  { return Children().Length(); }
  
  /// Exposes [] for index access
  awsKey *GetItemAt(int i)
  { return (awsKey *)Children()[i];  }
};



//////////////////////////////////  Preference Manager ////////////////////////////////////////////////////////////////


enum AWS_COLORS { AC_HIGHLIGHT, AC_HIGHLIGHT2, AC_SHADOW, AC_SHADOW2, AC_FILL, AC_DARKFILL, 
		  AC_TEXTFORE, AC_TEXTBACK, AC_TEXTDISABLED, 
		  AC_BUTTONTEXT, AC_TRANSPARENT, 
                  AC_BLACK, AC_WHITE, AC_RED, AC_GREEN, AC_BLUE,
		  AC_COLOR_COUNT };

class awsPrefManager : public iAwsPrefManager
{
   /// list of window definitions
  csDLinkList win_defs;

  /// list of skin definitions
  csDLinkList skin_defs;

  /// count of window defintions loaded
  unsigned int n_win_defs;

  /// count of skin defintions loaded
  unsigned int n_skin_defs;

  /// currently selected skin 
  awsSkinNode *def_skin;
  
  /// color index
  int sys_colors[AC_COLOR_COUNT];

  /// aws texture manager
  awsTextureManager *awstxtmgr;

  /// font loader
  iFontServer *fontsvr;

  /// default font
  iFont *default_font;

  /// window manager
  iAws  *wmgr;

  /// vfs plugin
  iVFS *vfs;

  /// constant value heap
  csBasicVector constants;

  /// container of constants
  struct constant_entry
  {
    /// Name of constant
    unsigned int name;

    /// Integer value
    int value;
  };

public:
    SCF_DECLARE_IBASE;

    awsPrefManager(iBase *iParent);
    virtual ~awsPrefManager();

    /// Invokes the parser to load a definitions file.
    virtual bool Load(const char *def_file);

    /// Maps a name to an id
    virtual unsigned long NameToId(char *name);
    
    /// Select which skin is the default for components, the skin must be loaded.  True on success, false otherwise.
    virtual bool SelectDefaultSkin(char *skin_name);

    /// Lookup the value of an int key by name (from the skin def)
    virtual bool LookupIntKey(char *name, int &val); 

    /// Lookup the value of an int key by id (from the skin def)
    virtual bool LookupIntKey(unsigned long id, int &val); 

    /// Lookup the value of a string key by name (from the skin def)
    virtual bool LookupStringKey(char *name, iString *&val); 

    /// Lookup the value of a string key by id (from the skin def)
    virtual bool LookupStringKey(unsigned long id, iString *&val); 

    /// Lookup the value of a rect key by name (from the skin def)
    virtual bool LookupRectKey(char *name, csRect &rect); 

    /// Lookup the value of a rect key by id (from the skin def)
    virtual bool LookupRectKey(unsigned long id, csRect &rect); 
    
    /// Lookup the value of an RGB key by name (from the skin def)
    virtual bool LookupRGBKey(char *name, unsigned char &red, unsigned char &green, unsigned char &blue);
    
    /// Lookup the value of an RGB key by name (from the skin def)
    virtual bool LookupRGBKey(unsigned long id, unsigned char &red, unsigned char &green, unsigned char &blue);

    /// Lookup the value of a point key by name (from the skin def)
    virtual bool LookupPointKey(char *name, csPoint &point); 

    /// Lookup the value of a point key by id (from the skin def)
    virtual bool LookupPointKey(unsigned long id, csPoint &point); 
            
    /// Get the value of an integer from a given component node
    virtual bool GetInt(awsComponentNode *node, char *name, int &val);

    /// Get the a rect from a given component node
    virtual bool GetRect(awsComponentNode *node, char *name, csRect &rect);

    /// Get the value of an integer from a given component node
    virtual bool GetString(awsComponentNode *node, char *name, iString *&val);
    
    /// Find window definition and return the component node holding it, Null otherwise
    virtual awsComponentNode *FindWindowDef(char *name);

public:
    /// Called by internal code to add a parsed out tree of window components.
    void AddWindowDef(awsComponentNode *win)
    { win_defs.AddItem(win); n_win_defs++; }

    /// Called by internal code to add a parsed out tree of skin defintions.
    void AddSkinDef(awsSkinNode *skin)
    { skin_defs.AddItem(skin); n_skin_defs++; }

public:
    /// Sets the value of a color in the global AWS palette.
    virtual void SetColor(int index, int color); 
    
    /// Gets the value of a color from the global AWS palette.
    virtual int  GetColor(int index);

    /// Gets the current default font
    virtual iFont *GetDefaultFont();

    /// Gets a font.  If it's not loaded, it will be.  Returns NULL on error.
    virtual iFont *GetFont(char *filename);
    
    /// Gets a texture from the global AWS cache
    virtual iTextureHandle *GetTexture(char *name, char *filename=NULL);

    /** Changes the texture manager: unregisters all current textures, and then re-registers them
     *  with the new manager */
    virtual void SetTextureManager(iTextureManager *txtmgr);

    /** Changes the font server.  This must be set during setup in awsManager. */
    virtual void SetFontServer(iFontServer *fntsvr);

    /** Changes the window manager.  This must be set during setup in awsManager. */
    virtual void SetWindowMgr(iAws *_wmgr);
        
    /** Sets up the AWS palette so that the colors are valid reflections of
       user preferences.  Although SetColor can be used, it's recommended 
       that you do not.  Colors should always be a user preference, and 
       should be read from the window and skin definition files (as
       happens automatically normally. */
    virtual void SetupPalette();

    /** Performs whatever initialization is necessary.  For now, it simply
      * initializes the texture loader. */
    virtual bool Setup(iObjectRegistry *obj_reg);

    /** Allows a component to specify it's own constant values for parsing. */
    virtual void RegisterConstant(char *name, int value);

    /** Returns true if the constant has been registered, false otherwise.  */
    virtual bool ConstantExists(char *name);

    /** Allows a component to retrieve the value of a constant, or the parser as well. */
    virtual int  GetConstantValue(char *name);
    
};
 
#endif

