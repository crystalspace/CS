/*
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
*/

#ifndef __CS_AWS_PREFS_H__
#define __CS_AWS_PREFS_H__

#include "csgeom/cspoint.h"
#include "csgeom/csrect.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/scfstr.h"
#include "iaws/aws.h"
#include "iutil/strset.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "awstex.h"

/**\file
 * This is the pseudo-symbol table for the definitions keeper.  Windows and
 * their sub-keys can be looked up from here. There are a few different types
 * of values possible for keys:  Strings, Integers, and Rects.  They can be
 * looked up using appropriate search methods in the main preferences.  Skins
 * and Windows are containers which hold the keys, and the prefs manager
 * contains those skin and window defintions.
 *<p>
 * Windows can be filled in because components provide a factory service by
 * registering, and then know how to get their settings from the window
 * definition.
 *<p>
 * Note that many of these classes use this in the base member
 * initializer list.
 *<p>
 * While the ISO C++ standard supports this in paragraph 8 of
 * section 12.16.2, some compilers do not particularly like it. It can also
 * be abused by having some class refer to objects that have not yet been
 * initialized.  Be very careful in modifying the architecture of keys.
 */

/// Keys.
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
  KEY_CONTAINER = 10,
  KEY_FLOAT = 11
};

/**
 * Abstract key interface.
 */
class awsKey : public iAwsKey
{
private:
  /// Shared string table.
  csRef<iStringSet> strset;
  /// The name of the key.
  unsigned long name;
protected:
  unsigned long ComputeKeyID (const char* name) const;
public:
  SCF_DECLARE_IBASE;

  /// Simple constructor creates new key with name "n" (iString version).
  awsKey (iAws* a, iString *n) : strset(a->GetStringTable())
  { 
    SCF_CONSTRUCT_IBASE (0);
    name = ComputeKeyID (n->GetData ());
  }

  /// Simple constructor creates new key with name "n" (const char* version).
  awsKey (iAws* a, const char* n) : strset(a->GetStringTable())
  {
    SCF_CONSTRUCT_IBASE (0);
    name = ComputeKeyID (n);
  }

  /// Simple destructor does nothing.
  virtual ~awsKey ()
  {
    SCF_DESTRUCT_IBASE ();
  };

  /// Pure virtual function Type returns the type of key.
  virtual uint8 Type () const = 0;

  /// Accessor function gets name of key.
  unsigned long Name () const { return name; }
};

class awsIntKey : public awsKey, public iAwsIntKey
{
private:
  /// The key's value.
  int val;
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructs an integer key with the given name.
  awsIntKey (iAws* a, iString *name, int v) : awsKey (a,name), val (v) { }

  /// Constructs an integer key with the given name.
  awsIntKey (iAws* a, const char* name, int v) : awsKey (a,name), val (v) { }

  /// Destructor does nothing.
  virtual ~awsIntKey () { }

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name(); }

  /// So that we know it's an int key.
  virtual uint8 Type () const { return KEY_INT; }

  /// Gets the value of this key as an integer.
  int Value () const { return val; }
};

class awsFloatKey : public awsKey, public iAwsFloatKey
{
private:
  /// The key's value.
  float val;
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructs a float key with the given name.
  awsFloatKey (iAws* a, iString *name, float v) : awsKey(a,name), val(v) { }

  /// Constructs a float key with the given name.
  awsFloatKey (iAws* a, const char* name, float v) : awsKey(a,name), val(v) { }

  /// Destructor does nothing.
  virtual ~awsFloatKey () { }

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name(); }

  /// So that we know it's a float key.
  virtual uint8 Type () const { return KEY_FLOAT; }

  /// Gets the value of this key as a float.
  float Value () const { return val; }
};

class awsStringKey : public awsKey, public iAwsStringKey
{
private:
  /// The key's value.
  csRef<iString> val;
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructs a string key with the given name.
  awsStringKey (iAws* a, iString *name, iString *v) : awsKey(a,name), val(v) {}

  /// Constructs a string key with the given name.
  awsStringKey (iAws* a, const char* name, const char* v) : awsKey (a,name)
  {
    val.AttachNew(new scfString (v));
  }

  /// Constructs a string key with the given name.
  awsStringKey(iAws* a, const char* name, iString* v) : awsKey(a,name),val(v){}

  /// Constructs a string key with the given name.
  awsStringKey (iAws* a, const awsStringKey& key) :
    awsKey (a,key.Value()), iAwsStringKey ()
  {
    val = key.val;
  }

  /// Destructor.
  virtual ~awsStringKey () {}

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name (); }

  /// So that we know it's a string key.
  virtual uint8 Type () const { return KEY_STR; }

  /// Gets the value of this key as an iString.
  iString* Value () const { return val; }
};

class awsRectKey : public awsKey, public iAwsRectKey
{
private:
  /// The key's value.
  csRect val;
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructs an integer key with the given name.
  awsRectKey (iAws* a, iString *name, csRect v) : awsKey (a,name), val (v) { }

  /// Constructs an integer key with the given name.
  awsRectKey (iAws* a, const char* name, csRect v) : awsKey(a,name), val(v) { }

  /// Destructor does nothing.
  virtual ~awsRectKey () { }

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name (); }

  /// So that we know this is a rect key.
  virtual uint8 Type () const { return KEY_RECT; }

  /// Gets the value of this key as a rectangle.
  csRect Value () const { return val; }
};

class awsRGBKey : public awsKey, public iAwsRGBKey
{
protected:
  /// The key's value.
  iAwsRGBKey::RGB rgb;
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructs an integer key with the given name.
  awsRGBKey (
    iAws* aws,
    iString *name,
    unsigned char r,
    unsigned char g,
    unsigned char b)
  : awsKey (aws,name)
  {
    rgb.red = r;
    rgb.green = g;
    rgb.blue = b;
  }

  /// Constructs an integer key with the given name.
  awsRGBKey (
    iAws* aws,
    const char* name,
    unsigned char r,
    unsigned char g,
    unsigned char b)
  : awsKey(aws,name)
  {
    rgb.red = r;
    rgb.green = g;
    rgb.blue = b;
  }

  /// Destructor does nothing.
  virtual ~awsRGBKey () { }

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name (); }

  /// So that we know this is a rect key.
  virtual uint8 Type () const { return KEY_RGB; }

  /// Gets the value of this key as a rectangle.
  const iAwsRGBKey::RGB &Value () const { return rgb; }
};

class awsPointKey : public awsKey, public iAwsPointKey
{
private:
  /// The key's value.
  csPoint val; 
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructs an integer key with the given name.
  awsPointKey(iAws* a, iString *name, csPoint v) : awsKey(a,name), val(v) {}

  /// Constructs an integer key with the given name.
  awsPointKey(iAws* a, const char* name, csPoint v) : awsKey(a,name), val(v) {}

  /// Destructor does nothing.
  virtual ~awsPointKey () { }

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name(); }

  /// So that we know this is a rect key.
  virtual uint8 Type () const { return KEY_POINT; }

  /// Gets the value of this key as a rectangle.
  csPoint Value () const { return val; }
};

class awsConnectionKey : public awsKey, public iAwsConnectionKey
{
private:
  /// The sink that we want.
  iAwsSink *sink;

  /// The trigger that we want.
  unsigned long trigger;

  /// The signal that we want.
  unsigned long signal;
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructs an integer key with the given name.
  awsConnectionKey (
    iAws* a,
    iString *name,
    iAwsSink *s,
    unsigned long t,
    unsigned long sig)
  : awsKey(a,name),
    sink(s),
    trigger(t),
    signal(sig)
  { }

  /// Constructs an integer key with the given name.
  awsConnectionKey (
    iAws* a,
    const char* name,
    iAwsSink* s,
    unsigned long t,
    unsigned long sig)
  : awsKey(a,name),
    sink(s),
    trigger(t),
    signal(sig)
  { }

  /// Destructor does nothing.
  virtual ~awsConnectionKey () { }

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name (); }

  /// So that we know this is a rect key.
  virtual uint8 Type () const { return KEY_CONNECTION; }

  /// Gets the sink for this key.
  iAwsSink *Sink () const { return sink; }

  /// Gets the trigger for this key.
  unsigned long Trigger () const { return trigger; }

  /// Gets the signal for this key.
  unsigned long Signal () const { return signal; }
};

class awsKeyContainer : public awsKey, public iAwsKeyContainer
{
private:
  /// List of children in container.
  csRefArray<iAwsKey> children;
public:
  SCF_DECLARE_IBASE_EXT (awsKey);

  /// Constructor that assigns a default name ("Default").
  awsKeyContainer (iAws* a) : awsKey (a,"Default") { }

  /// Constructor that assigns name n.
  awsKeyContainer (iAws* a, iString* n) : awsKey (a,n) { }

  /// Constructor that assigns name n.
  awsKeyContainer (iAws* a, const char* n) : awsKey (a,n) { }

  /// Destructor that does nothing.
  ~awsKeyContainer ()
  {
    int i;
    for (i = Length()-1; i >= 0; i--)
      Remove(GetAt (i));
  }

  /// Looks up a key based on it's name.
  iAwsKey* Find (iString* name) const;

  /// Looks up a key based on it's name.
  iAwsKey* Find (const char* name) const;

  /// Looks up a key based on it's ID.
  iAwsKey *Find (unsigned long id) const;

  const csRefArray<iAwsKey> &Children () const { return children; }

  /// Adds an item to the container.
  void Add (iAwsKey *key) { children.Push (key); }

  /// Returns children number i.
  iAwsKey* GetAt (int i) const { return children[i]; }

  /// Returns number of childrens.
  int Length () const { return children.Length (); }
    
  /// Removes an item from the container.
  void Remove (iString *name);
  
  /// Removes an item from the container.
  void Remove (const char* name);
  
  /// Removes a specific item from the container.
  void Remove (iAwsKey *key);
  
  /// Removes all keys from the container.
  void RemoveAll ();

  /**
   * Consumes an entire list by moving all of it's member's to this one,
   * and removing them from it.
   */
  void Consume (iAwsKeyContainer *c);

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name (); }

  virtual uint8 Type () const { return KEY_CONTAINER; }
};

class awsSkinNode : public awsKeyContainer
{
public:
  awsSkinNode (iAws* a, iString *name) : awsKeyContainer (a,name) { }
  
  awsSkinNode (iAws* a, const char* name) : awsKeyContainer (a,name) { }
  
  virtual ~awsSkinNode () { }

  /// So that we know this is a skin node.
  virtual uint8 Type () const { return KEY_SKIN; }
};

class awsComponentNode : public awsKeyContainer, public iAwsComponentNode
{
private:
  /// The type of component, like "Radio Button", "Check Box", etc.
  iString *comp_type;
public:
  SCF_DECLARE_IBASE_EXT (awsKeyContainer);

  awsComponentNode (iAws* a, iString *name, iString *component_type)
    : awsKeyContainer (a,name), comp_type (component_type) { }
      
  awsComponentNode (iAws* a, const char* name, const char* component_type)
    : awsKeyContainer (a,name) { comp_type = new scfString (component_type); }
    
  virtual ~awsComponentNode () { comp_type->DecRef (); }

  /// Looks up a key based on it's name.
  iAwsKey* Find (iString* name) const
  { return awsKeyContainer::Find (name); }

  /// Looks up a key based on it's name.
  iAwsKey* Find (const char* name) const
  { return awsKeyContainer::Find (name); }

  /// Looks up a key based on it's ID.
  iAwsKey *Find (unsigned long id) const
  { return awsKeyContainer::Find (id); }

  const csRefArray<iAwsKey> &Children () const
  { return awsKeyContainer::Children (); }

  /// Adds an item to the container.
  void Add (iAwsKey *key) { awsKeyContainer::Add (key); }

  /// returns children number i.
  iAwsKey* GetAt (int i) const { return awsKeyContainer::GetAt (i); }

  /// returns number of childrens.
  int Length () const { return awsKeyContainer::Length (); }
    
  /// Removes an item from the container.
  void Remove (iString *name) { awsKeyContainer::Remove (name); }
  
  /// Removes an item from the container.
  void Remove (const char* name) { awsKeyContainer::Remove (name); }
  
  /// Removes a specific item from the container.
  void Remove (iAwsKey *key) { awsKeyContainer::Remove (key); }
  
  /// Removes all items from the container.
  void RemoveAll () { awsKeyContainer::RemoveAll (); }

  /**
   * Consumes an entire list by moving all of it's member's to this one,
   * and removing them from it.
   */
  void Consume (iAwsKeyContainer *c) { awsKeyContainer::Consume (c); }

  /// Accessor function gets name of key.
  unsigned long Name () const { return awsKey::Name (); }

  /// So that we know this is a component node.
  virtual uint8 Type () const { return KEY_COMPONENT; }

  /// So that we can find out what sort of component type this should be.
  iString *ComponentTypeName () const { return comp_type; }
};

class awsConnectionNode : public awsKeyContainer
{
public:
  awsConnectionNode (iAws*);
  virtual ~awsConnectionNode ();

  /// So that we know this is a component node.
  virtual uint8 Type () const { return KEY_CONNECTIONMAP; }
};

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
private:
  csRef<iGraphics2D> g2d;

  /// List of window definitions.
  csRefArray<iAwsComponentNode> win_defs;

  /// List of skin definitions.
  csRefArray<iAwsKeyContainer> skin_defs;

  /// Currently selected skin.
  iAwsKeyContainer *def_skin;

  /// Color index.
  int sys_colors[AC_COLOR_COUNT];

  /// Aws texture manager.
  awsTextureManager *awstxtmgr;

  /// Font loader.
  csRef<iFontServer> fontsvr;

  /// Default font.
  csRef<iFont> default_font;

  /// Window manager.
  iAws *wmgr;

  /// VFS plugin.
  iObjectRegistry *objreg;

  /// Container of constants.
  struct constant_entry
  {
    /// Name of constant.
    unsigned int name;

    /// Integer value.
    int value;
  };

  /// constant value heap.
  csPDelArray<constant_entry> constants;
public:
  SCF_DECLARE_IBASE;

  awsPrefManager (iBase *iParent);
  virtual ~awsPrefManager ();

  /// Invokes the parser to load a definitions file.
  virtual bool Load (const char *def_file);

  /// Maps a name to an id
  virtual unsigned long NameToId (const char *name);

  /**
   * Select which skin is the default for components, the skin must be
   * loaded. True on success, false otherwise.
   */
  virtual bool SelectDefaultSkin (const char *skin_name);

  /// Lookup the value of an int key by name (from the skin def).
  virtual bool LookupIntKey (const char *name, int &val);

  /// Lookup the value of an int key by id (from the skin def).
  virtual bool LookupIntKey (unsigned long id, int &val);

  /// Lookup the value of a float key by name (from the skin def).
  virtual bool LookupFloatKey (const char *name, float &val);

  /// Lookup the value of a float key by id (from the skin def).
  virtual bool LookupFloatKey (unsigned long id, float &val);

  /// Lookup the value of a string key by name (from the skin def).
  virtual bool LookupStringKey (const char *name, iString * &val);

  /// Lookup the value of a string key by id (from the skin def).
  virtual bool LookupStringKey (unsigned long id, iString * &val);

  /// Lookup the value of a rect key by name (from the skin def).
  virtual bool LookupRectKey (const char *name, csRect &rect);

  /// Lookup the value of a rect key by id (from the skin def).
  virtual bool LookupRectKey (unsigned long id, csRect &rect);

  /// Lookup the value of an RGB key by name (from the skin def).
  virtual bool LookupRGBKey (
    const char *name,
    unsigned char &red,
    unsigned char &green,
    unsigned char &blue);

  /// Lookup the value of an RGB key by name (from the skin def).
  virtual bool LookupRGBKey (
    unsigned long id,
    unsigned char &red,
    unsigned char &green,
    unsigned char &blue);

  /// Lookup the value of a point key by name (from the skin def).
  virtual bool LookupPointKey (const char *name, csPoint &point);

  /// Lookup the value of a point key by id (from the skin def).
  virtual bool LookupPointKey (unsigned long id, csPoint &point);

  /// Get the value of an integer from a given component node.
  virtual bool GetInt (iAwsComponentNode *node, const char *name, int &val);

  /// Get the value of an integer from a given component node.
  virtual bool GetFloat (
    iAwsComponentNode *node,
    const char *name,
    float &val);

  /// Get the a rect from a given component node.
  virtual bool GetRect (
    iAwsComponentNode *node,
    const char *name,
    csRect &rect);

  /// Get the value of an integer from a given component node.
  virtual bool GetString (
    iAwsComponentNode *node,
    const char *name,
    iString * &val);

  /// Get the a color from a given component node.
  virtual bool GetRGB (
    iAwsComponentNode *node,
    const char *name,
    unsigned char& r,
    unsigned char& g,
    unsigned char& b);

  /**
   * Find window definition and return the component node holding it, Null
   * otherwise.
   */
  virtual iAwsComponentNode *FindWindowDef (const char *name);

  /// Find skin definition and return pointer, Null if not found.
  virtual iAwsKeyContainer *FindSkinDef (const char *name);

  /**
   * Completely remove a window definition from the list
   * (false if doesn't exist).
   */
  bool RemoveWindowDef (const char *name)
  {
    iAwsComponentNode *nd = FindWindowDef (name);
    if (!nd)
      return false;
    nd->RemoveAll ();
    win_defs.Delete (nd);
    return true;
  }

  /// Removes all window definitions from the list.
  void RemoveAllWindowDefs ()
  {
    size_t i;
    for (i = 0; i < win_defs.Length (); i++)
      win_defs[i]->RemoveAll ();
    win_defs.DeleteAll ();
  }

  /**
   * Completely remove a skin definition from the list
   * (false if doesn't exist).
   */
  bool RemoveSkinDef (const char *name)
  {
    iAwsKeyContainer *kc=FindSkinDef (name);
    if(!kc)
      return false;
    kc->RemoveAll ();
    skin_defs.Delete (kc);
    return true;
  }

  /// Removes all skin definitions from the list
  void RemoveAllSkinDefs ()
  {
    size_t i;
    for (i = 0; i < skin_defs.Length (); i++)
      skin_defs[i]->RemoveAll ();
    skin_defs.DeleteAll ();
  }
public:
  /// Called by internal code to add a parsed out tree of window components.
  void AddWindowDef (iAwsComponentNode *win)
  {
    if (!win)
      return;

    win_defs.Push (win);
  }

  /// Called by internal code to add a parsed out tree of skin defintions.
  void AddSkinDef (iAwsKeyContainer *skin)
  {
    if (!skin || skin->Type() != KEY_SKIN)
      return;
    skin_defs.Push (skin);
  }

  /// Sets the value of a color in the global AWS palette.
  virtual void SetColor (int index, int color);

  /// Gets the value of a color from the global AWS palette.
  virtual int GetColor (int index);

  /// Finds the closest matching color
  virtual int FindColor (unsigned char r, unsigned char g, unsigned char b);

  /// Gets the current default font
  virtual iFont *GetDefaultFont ();

  /// Gets a font.  If it's not loaded, it will be.  Returns 0 on error.
  virtual iFont *GetFont (const char *filename);

  /// Gets a texture from the global AWS cache
  virtual iTextureHandle *GetTexture (
    const char *name,
    const char *filename = 0);

  /**
   * Gets a texture from the global AWS cache, if its loaded for the
   * first time then the keycolor (key_r,key_g,key_b) is set.
   */
  virtual iTextureHandle *GetTexture (
    const char *name,
    const char *filename,
    unsigned char key_r,
    unsigned char key_g,
    unsigned char key_b);

  /**
   * Changes the texture manager: unregisters all current textures,
   * and then re-registers them with the new manager.
   */
  virtual void SetTextureManager (iTextureManager *txtmgr);

  /// Changes the font server.  This must be set during setup in awsManager.
  virtual void SetFontServer (iFontServer *fntsvr);

  /// Changes the default font. This must be set during setup in awsManager.
  virtual void SetDefaultFont (iFont *font);

  /// Changes the window manager.  This must be set during setup in awsManager.
  virtual void SetWindowMgr (iAws *_wmgr);

  /**
   * Sets up the AWS palette so that the colors are valid reflections of
   * user preferences.  Although SetColor can be used, it's recommended
   * that you do not.  Colors should always be a user preference, and
   * should be read from the window and skin definition files (as
   * happens automatically normally.
   */
  virtual void SetupPalette ();

  /**
   * Performs whatever initialization is necessary.  For now, it simply
   * initializes the texture loader.
   */
  virtual bool Setup (iObjectRegistry *obj_reg);

  /// Allows a component to specify it's own constant values for parsing.
  virtual void RegisterConstant (const char *name, int value);

  /// Returns true if the constant has been registered, false otherwise.
  virtual bool ConstantExists (const char *name);

  /**
   * Allows a component to retrieve the value of a constant, or the
   * parser as well.
   */
  virtual int GetConstantValue (const char *name);

  /// Creates a new key factory.
  virtual iAwsKeyFactory *CreateKeyFactory ();

  ///reates a new connection node factory.
  virtual iAwsConnectionNodeFactory *CreateConnectionNodeFactory ();
};

#endif // __CS_AWS_PREFS_H__
