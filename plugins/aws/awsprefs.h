#ifndef __CS_AWS_PREFERENCES_H__
#define __CS_AWS_PREFERENCES_H__

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
# include "iaws/aws.h"
# include "csgeom/csrect.h"
# include "csgeom/cspoint.h"
# include "csutil/csdllist.h"
# include "csutil/csvector.h"
# include "csutil/scfstr.h"
# include "ivideo/graph2d.h"

# include "awstex.h"

/****
 This is the pseudo-symbol table for the definitions keeper.  Windows and
 their sub-keys can be looked up from here. There are a few different types
 of values possible for keys:  Strings, Integers, and Rects.  They can be
 looked up using appropriate search methods in the main preferences.  Skins
 and Windows are containers which hold the keys, and the prefs manager
 contains those skin and window defintions.
 Windows can be filled in because components provide a factory service by
 registering, and then know how to get their settings from the window
 definition.



 Note that many of these classes use this in the base member initializer list.

 While the ISO C++ standard supports this in paragraph 8 of section 12.16.2, 

 some compilers do not particularly like it.  It can also be abused by having

 some class refer to objects that have not yet been initialized.  Be very careful

 in modifying the architecture of keys.


****/

//////////////////////////////////  Keys  ///////////////////////////////////

enum {
  KEY_INT = 0,
  KEY_STR = 1,
  KEY_RECT = 2,
  KEY_WIN = 3,
  KEY_SKIN = 4,
  KEY_COMPONENT = 5,
  KEY_RGB = 6,
  KEY_POINT = 7,
  KEY_CONNECTION = 8,
  KEY_CONNECTIONMAP = 9,
  KEY_CONTAINER = 10
};

/// Abstract key interface
class awsKey : public iAwsKey
{
  /// The name of the key
  unsigned long name;

  void ComputeKeyID (const char* name, size_t len);
public:
  SCF_DECLARE_IBASE;

  /// Simple constructor creates new key with name "n" (iString version)
  awsKey (iString *n)
  { 
    SCF_CONSTRUCT_IBASE(NULL);
    ComputeKeyID (n->GetData (), n->Length());
  }
  /// Simple constructor creates new key with name "n" (const char* version)
  awsKey (const char* n)
  {
    SCF_CONSTRUCT_IBASE(NULL);
    ComputeKeyID (n, strlen(n));
  }

  /// Simple destructor does nothing
  virtual ~awsKey ()
  { };

  /// Pure virtual function Type returns the type of key
  virtual uint8 Type () = 0;

  /// Accessor function gets name of key
  unsigned long Name () { return name; }
};

class awsIntKey : public awsKey, public iAwsIntKey
{
  /// The key's value
  int val;

public:

  SCF_DECLARE_IBASE_EXT(awsKey);
  /// Constructs an integer key with the given name
  awsIntKey (iString *name, int v)
    :  awsKey(name), val(v)
  { }
  /// Constructs an integer key with the given name
  awsIntKey (const char* name, int v)
    : awsKey(name), val(v)
  { }

  /// Destructor does nothing
  virtual ~awsIntKey ()
  { }

  /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  /// So that we know it's an int key
  virtual uint8 Type ()
  { return KEY_INT; }

  /// Gets the value of this key as an integer
  int Value ()
  { return val; }
};

class awsStringKey : public awsKey, public iAwsStringKey
{
  /// The key's value
  iString *val;

public:
  /// Constructs a string key with the given name (this function takes ref of
  //the iString
#if 0
  awsStringKey (iString *name, iString *v)
    : awsKey(name), val(v)
  { }
#endif

  SCF_DECLARE_IBASE_EXT(awsKey);

  /// Constructs a string key with the given name
  awsStringKey (const char* name, const char* v)
    : awsKey(name)
  { val = new scfString (v); }

  awsStringKey (const char* name, iString* v)
    : awsKey(name), val(v)
  { val->IncRef (); }

  awsStringKey (const awsStringKey& key)
    : awsKey(key), iAwsStringKey()
  { val = key.val; val->IncRef(); }

  /// Destructor does nothing
  virtual ~awsStringKey ()
  { SCF_DEC_REF(val); }

  /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  /// So that we know it's a string key.
  virtual uint8 Type ()
  { return KEY_STR; }

  /// Gets the value of this key as an iString
  iString* Value ()
  { return val; }
};

class awsRectKey : public awsKey, public iAwsRectKey
{
  /// The key's value
  csRect val;
  
public:
  SCF_DECLARE_IBASE_EXT(awsKey);

  /// Constructs an integer key with the given name
  awsRectKey (iString *name, csRect v)
    : awsKey(name), val(v)
  { }
  /// Constructs an integer key with the given name 
  awsRectKey (const char* name, csRect v)
    : awsKey(name), val(v)
  { }

  /// Destructor does nothing
  virtual ~awsRectKey ()
  { }

    /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  /// So that we know this is a rect key
  virtual uint8 Type ()
  { return KEY_RECT; }

  /// Gets the value of this key as a rectangle
  csRect Value ()
  { return val; }
};

class awsRGBKey : public awsKey, public iAwsRGBKey
{
public:
  
protected:
  /// The key's value
  iAwsRGBKey::RGB rgb;
  
public:
  SCF_DECLARE_IBASE_EXT(awsKey);

  /// Constructs an integer key with the given name
  awsRGBKey (
    iString *name,
    unsigned char r,
    unsigned char g,
    unsigned char b)
  : awsKey(name)
  {
    rgb.red = r;
    rgb.green = g;
    rgb.blue = b;
  }
  /// Constructs an integer key with the given name
  awsRGBKey (
    const char* name,
    unsigned char r,
    unsigned char g,
    unsigned char b)
  : awsKey(name)
  {
    rgb.red = r;
    rgb.green = g;
    rgb.blue = b;
  }

  /// Destructor does nothing
  virtual ~awsRGBKey ()
  { }

    /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  /// So that we know this is a rect key
  virtual uint8 Type ()
  { return KEY_RGB; }

  /// Gets the value of this key as a rectangle
  iAwsRGBKey::RGB &Value()
  { return rgb; }
};

class awsPointKey : public awsKey, public iAwsPointKey
{
  /// The key's value
  csPoint val;
  
public:
  SCF_DECLARE_IBASE_EXT(awsKey);

  /// Constructs an integer key with the given name
  awsPointKey (iString *name, csPoint v)
    : awsKey(name), val(v)
  { }
  /// Constructs an integer key with the given name
  awsPointKey (const char* name, csPoint v)
    : awsKey(name), val(v)
  { }

  /// Destructor does nothing
  virtual ~awsPointKey ()
  { }

    /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  /// So that we know this is a rect key
  virtual uint8 Type ()
  { return KEY_POINT; }

  /// Gets the value of this key as a rectangle
  csPoint Value ()
  { return val; }
};

class awsConnectionKey : public awsKey, public iAwsConnectionKey
{
  /// The sink that we want
  iAwsSink *sink;

  /// The trigger that we want
  unsigned long trigger;

  /// The signal that we want
  unsigned long signal;
public:
  SCF_DECLARE_IBASE_EXT(awsKey);

  /// Constructs an integer key with the given name
  awsConnectionKey (
    iString *name,
    iAwsSink *s,
    unsigned long t,
    unsigned long sig)
    
  : awsKey(name),
    sink(s),
    trigger(t),
    signal(sig)
  { }
  /// Constructs an integer key with the given name
  awsConnectionKey (
    const char* name,
    iAwsSink* s,
    unsigned long t,
    unsigned long sig)

  : awsKey(name),
    sink(s),
    trigger(t),
    signal(sig)
  { }

  /// Destructor does nothing
  virtual ~awsConnectionKey ()
  { }

    /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  /// So that we know this is a rect key
  virtual uint8 Type ()
  { return KEY_CONNECTION; }

  /// Gets the sink for this key
  iAwsSink *Sink ()
  { return sink; }

  /// Gets the trigger for this key
  unsigned long Trigger ()
  { return trigger; }

  /// Gets the signal for this key
  unsigned long Signal ()
  { return signal; }
};

//////////////////////////////////  Containers ////////////////////////////////////////////////////////////////////////
class awsKeyContainer : public awsKey, public iAwsKeyContainer
{
  /// list of children in container.
  csBasicVector children;

public:
  SCF_DECLARE_IBASE_EXT(awsKey);

  /// constructor that assigns a default name
  awsKeyContainer () : awsKey("Default")
  { }

  /// constructor that assigns name n
  awsKeyContainer (iString* n) : awsKey(n)
  { }

  /// constructor that assigns name n
  awsKeyContainer (const char* n) : awsKey(n)
  { }

  /// destructor that does nothing
  ~awsKeyContainer ()
  { 
    for (int i = Length()-1; i >= 0; i--)
      Remove(GetAt(i));
  }

  /// Looks up a key based on it's name.
  iAwsKey* Find (iString* name);

  /// Looks up a key based on it's name.
  iAwsKey* Find (const char* name);

  /// Looks up a key based on it's ID.
  iAwsKey *Find (unsigned long id);

  csBasicVector &Children ()
  { return children; }

  /// Adds an item to the container
  void Add (iAwsKey *key)
  { key->IncRef(); children.Push (key); }
  /// returns children number i
  iAwsKey* GetAt (int i)
  { return (iAwsKey*) children[i]; }

  /// returns number of childrens
  int Length ()
  { return children.Length (); }
    
  /// Removes an item from the container
  void Remove (iString *name);
  /// Removes an item from the container
  void Remove (const char* name);
  /// Removes a specific item from the container
  void Remove (iAwsKey *key);
  /// Removes all keys from the container.
  void RemoveAll ();

  /// Consumes an entire list by moving all of it's member's to this one, and removing them from it.
  void Consume (iAwsKeyContainer *c);

  /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  virtual uint8 Type ()
  { return KEY_CONTAINER; }
};

class awsSkinNode : public awsKeyContainer
{
public:

  awsSkinNode (iString *name)
    : awsKeyContainer (name)
  { }
  awsSkinNode (const char* name)
    : awsKeyContainer (name)
  { }
  virtual ~awsSkinNode ()
  { }

  /// So that we know this is a skin node
  virtual uint8 Type ()
  { return KEY_SKIN; }
};

class awsComponentNode : public awsKeyContainer, public iAwsComponentNode
{
  /// The type of component, like "Radio Button", "Check Box", etc.
  iString *comp_type;
public:
  SCF_DECLARE_IBASE_EXT(awsKeyContainer);

  awsComponentNode (iString *name, iString *component_type)
      : awsKeyContainer(name), comp_type(component_type)
  { }
  awsComponentNode (const char* name, const char* component_type)
      : awsKeyContainer(name)
  { comp_type = new scfString (component_type); }
  virtual ~awsComponentNode ()
  { comp_type->DecRef(); }

    /// Looks up a key based on it's name.
  iAwsKey* Find (iString* name) { return awsKeyContainer::Find(name); }

  /// Looks up a key based on it's name.
  iAwsKey* Find (const char* name) { return awsKeyContainer::Find(name); }

  /// Looks up a key based on it's ID.
  iAwsKey *Find (unsigned long id) { return awsKeyContainer::Find(id); }

  csBasicVector &Children ()
  { return awsKeyContainer::Children(); }

    /// Adds an item to the container
  void Add (iAwsKey *key)
  { awsKeyContainer::Add(key); }

    /// returns children number i
  iAwsKey* GetAt (int i)
  { return awsKeyContainer::GetAt(i); }

  /// returns number of childrens
  int Length ()
  { return awsKeyContainer::Length(); }
    
  /// Removes an item from the container
  void Remove (iString *name) { awsKeyContainer::Remove(name); }
  /// Removes an item from the container
  void Remove (const char* name) { awsKeyContainer::Remove(name); }
  /// Removes a specific item from the container
  void Remove (iAwsKey *key) { awsKeyContainer::Remove(key); }
  /// Removes all items from the container
  void RemoveAll () { awsKeyContainer::RemoveAll(); }

  /// Consumes an entire list by moving all of it's member's to this one, and removing them from it.
  void Consume (iAwsKeyContainer *c) { awsKeyContainer::Consume(c); }

  /// Accessor function gets name of key
  unsigned long Name () { return awsKey::Name(); }

  /// So that we know this is a component node
  virtual uint8 Type ()
  { return KEY_COMPONENT; }

  /// So that we can find out what sort of component type this should be
  iString *ComponentTypeName ()
  { return comp_type; }
};

class awsConnectionNode : public awsKeyContainer
{
public:
  awsConnectionNode ();
  virtual ~awsConnectionNode ();

  /// So that we know this is a component node
  virtual uint8 Type () { return KEY_CONNECTIONMAP; }
};

//////////////////////////////////  Preference Manager ////////////////////////////////////////////////////////////////
enum AWS_COLORS
{
  AC_HIGHLIGHT,
  AC_HIGHLIGHT2,
  AC_SHADOW,
  AC_SHADOW2,
  AC_FILL,
  AC_DARKFILL,
  AC_BACKFILL,
  AC_TEXTFORE,
  AC_TEXTBACK,
  AC_SELECTTEXTFORE,
  AC_SELECTTEXTBACK,
  AC_TEXTDISABLED,
  AC_BUTTONTEXT,
  AC_TRANSPARENT,
  AC_BLACK,
  AC_WHITE,
  AC_RED,
  AC_GREEN,
  AC_BLUE,
  AC_COLOR_COUNT
};

class awsPrefManager : public iAwsPrefManager
{
  csRef<iGraphics2D> g2d;

  /// list of window definitions
  csDLinkList win_defs;

  /// list of skin definitions
  csDLinkList skin_defs;

  /// count of window defintions loaded
  unsigned int n_win_defs;

  /// count of skin defintions loaded
  unsigned int n_skin_defs;

  /// currently selected skin
  iAwsKeyContainer *def_skin;

  /// color index
  int sys_colors[AC_COLOR_COUNT];

  /// aws texture manager
  awsTextureManager *awstxtmgr;

  /// font loader
  csRef<iFontServer> fontsvr;

  /// default font
  csRef<iFont> default_font;

  /// window manager
  iAws *wmgr;

  /// vfs plugin
  iObjectRegistry *objreg;

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

  awsPrefManager (iBase *iParent);
  virtual ~awsPrefManager ();

  /// Invokes the parser to load a definitions file.
  virtual bool Load (const char *def_file);

  /// Maps a name to an id
  virtual unsigned long NameToId (const char *name);

  /// Select which skin is the default for components, the skin must be loaded.  True on success, false otherwise.
  virtual bool SelectDefaultSkin (const char *skin_name);

  /// Lookup the value of an int key by name (from the skin def)
  virtual bool LookupIntKey (const char *name, int &val);

  /// Lookup the value of an int key by id (from the skin def)
  virtual bool LookupIntKey (unsigned long id, int &val);

  /// Lookup the value of a string key by name (from the skin def)
  virtual bool LookupStringKey (const char *name, iString * &val);

  /// Lookup the value of a string key by id (from the skin def)
  virtual bool LookupStringKey (unsigned long id, iString * &val);

  /// Lookup the value of a rect key by name (from the skin def)
  virtual bool LookupRectKey (const char *name, csRect &rect);

  /// Lookup the value of a rect key by id (from the skin def)
  virtual bool LookupRectKey (unsigned long id, csRect &rect);

  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey (
                const char *name,
                unsigned char &red,
                unsigned char &green,
                unsigned char &blue);

  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey (
                unsigned long id,
                unsigned char &red,
                unsigned char &green,
                unsigned char &blue);

  /// Lookup the value of a point key by name (from the skin def)
  virtual bool LookupPointKey (const char *name, csPoint &point);

  /// Lookup the value of a point key by id (from the skin def)
  virtual bool LookupPointKey (unsigned long id, csPoint &point);

  /// Get the value of an integer from a given component node
  virtual bool GetInt (iAwsComponentNode *node, const char *name, int &val);

  /// Get the a rect from a given component node
  virtual bool GetRect (iAwsComponentNode *node, const char *name, csRect &rect);

  /// Get the value of an integer from a given component node
  virtual bool GetString (iAwsComponentNode *node, const char *name, iString * &val);

  /// Get the a color from a given component node
  virtual bool GetRGB(iAwsComponentNode *node, const char *name, unsigned char& r, unsigned char& g, unsigned char& b);

  /// Find window definition and return the component node holding it, Null otherwise
  virtual iAwsComponentNode *FindWindowDef (const char *name);

  /// Find skin definition and return pointer, Null if not found
  virtual iAwsKeyContainer *FindSkinDef (const char *name);

  /// Completely remove a window definition from the list (false if doesn't exist)
  bool RemoveWindowDef (const char *name)
  {
    iAwsComponentNode *nd=FindWindowDef (name);
    if(nd) 
    {
      nd->RemoveAll ();
      win_defs.RemoveItem (nd);
      nd->DecRef();
      return true;
    }
    return false;
  }

  /// Removes all window definitions from the list
  void RemoveAllWindowDefs ()
  {
    iAwsComponentNode *nd=(iAwsComponentNode*)win_defs.GetFirstItem();
    while (nd)
    {
      nd->RemoveAll ();
      win_defs.RemoveItem ();
      nd->DecRef();
      nd=(iAwsComponentNode*)win_defs.GetNextItem();
    }
  }

  /// Completely remove a skin definition from the list (false if doesn't exist)
  bool RemoveSkinDef (const char *name)
  {
    iAwsKeyContainer *kc=FindSkinDef (name);
    if(kc)
    {
      kc->RemoveAll ();
      skin_defs.RemoveItem (kc);
      kc->DecRef();
      return true;
    }
    return false;
  }

  /// Removes all skin definitions from the list
  void RemoveAllSkinDefs ()
  {
    iAwsKeyContainer *sd=(iAwsKeyContainer*)skin_defs.GetFirstItem();
    while (sd)
    {
      sd->RemoveAll ();
      skin_defs.RemoveItem ();
      sd->DecRef();
      sd=(iAwsKeyContainer*)skin_defs.GetNextItem();
    }
  }

public:

  /// Called by internal code to add a parsed out tree of window components.
  void AddWindowDef (iAwsComponentNode *win)
  {
    if (!win) return ;
    win->IncRef();
    win_defs.AddItem (win);
    n_win_defs++;
  }

  /// Called by internal code to add a parsed out tree of skin defintions.
  void AddSkinDef (iAwsKeyContainer *skin)
  {
    if (!skin) return ;
    if( skin->Type() != KEY_SKIN) return;
    skin->IncRef();
    skin_defs.AddItem (skin);
    n_skin_defs++;
  }

public:
  /// Sets the value of a color in the global AWS palette.
  virtual void SetColor (int index, int color);

  /// Gets the value of a color from the global AWS palette.
  virtual int GetColor (int index);

  /// Finds the closest matching color
  virtual int FindColor(unsigned char r, unsigned char g, unsigned char b);

  /// Gets the current default font
  virtual iFont *GetDefaultFont ();

  /// Gets a font.  If it's not loaded, it will be.  Returns NULL on error.
  virtual iFont *GetFont (const char *filename);

  /// Gets a texture from the global AWS cache
  virtual iTextureHandle *GetTexture (const char *name, const char *filename = NULL);

  /// Gets a texture from the global AWS cache, if its loaded for the first time then
  /// the keycolor (key_r,key_g,key_b) is set
  virtual iTextureHandle *GetTexture (const char *name, const char *filename, 
                                      unsigned char key_r,
                                      unsigned char key_g,
                                      unsigned char key_b);

  /** Changes the texture manager: unregisters all current textures, and then re-registers them
     *  with the new manager */
  virtual void SetTextureManager (iTextureManager *txtmgr);

  /** Changes the font server.  This must be set during setup in awsManager. */
  virtual void SetFontServer (iFontServer *fntsvr);

  /** Changes the window manager.  This must be set during setup in awsManager. */
  virtual void SetWindowMgr (iAws *_wmgr);

  /** Sets up the AWS palette so that the colors are valid reflections of
       user preferences.  Although SetColor can be used, it's recommended
       that you do not.  Colors should always be a user preference, and
       should be read from the window and skin definition files (as
       happens automatically normally. */
  virtual void SetupPalette ();

  /** Performs whatever initialization is necessary.  For now, it simply
      * initializes the texture loader. */
  virtual bool Setup (iObjectRegistry *obj_reg);

  /** Allows a component to specify it's own constant values for parsing. */
  virtual void RegisterConstant (const char *name, int value);

  /** Returns true if the constant has been registered, false otherwise.  */
  virtual bool ConstantExists (const char *name);

  /** Allows a component to retrieve the value of a constant, or the parser as well. */
  virtual int GetConstantValue (const char *name);

  /** Creates a new key factory */
  virtual iAwsKeyFactory *CreateKeyFactory ();

  /** Creates a new connection node factory */
  virtual iAwsConnectionNodeFactory *CreateConnectionNodeFactory ();
};

#endif // __CS_AWS_PREFERENCES_H__

