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
#include "ivaria/aws.h"
#include "csgeom/csrect.h"
#include "csutil/csdllist.h"
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


//////////////////////////////////  Containers ////////////////////////////////////////////////////////////////////////

class awsKeyContainer 
{
  /// list of children in container.
  csDLinkList children;

public:
  awsKeyContainer() {};
  virtual ~awsKeyContainer() {};

public:
  /// Looks up a key based on it's name.
  awsKey *Find(iString *name);

  /// Looks up a key based on it's ID.
  awsKey *Find(unsigned long id);
    
  csDLinkList &Children()
  { return children; }
 
  /// Adds an item to the container
  void Add(awsKey *key) 
  { children.AddItem(key); }

  /// Removes an item from the container
  void Remove(iString *name)
  { children.RemoveItem(Find(name)); }

  /// Removes a specific item from the container
  void Remove(awsKey *key)
  { children.RemoveItem(key); }

  /// Consumes an entire list by moving all of it's member's to this one, and removing them from it.
  void Consume(awsKeyContainer *c)
  {
     void *p = c->children.GetFirstItem();

     while(p)
     {
        children.AddItem(p);
        c->children.RemoveItem();

        p=c->children.GetNextItem();
     }
  }
};

class awsSkinNode : public awsKey, awsKeyContainer
{
public:  
    awsSkinNode(iString *name):awsKey(name) {};
    virtual ~awsSkinNode() {};
    
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
    
  /// Exposes csDLinkList GetFirst for iteration initiation
  awsKey *GetFirst()
  { return (awsKey *)Children().GetFirstItem(); }
  
  /// Exposes csDLinkList GetNext for continuing iteration
  awsKey *GetNext()
  { return (awsKey *)Children().GetNextItem();  }
};

//////////////////////////////////  Preference Manager ////////////////////////////////////////////////////////////////

const unsigned int COLOR_HIGHLIGHT  = 0;
const unsigned int COLOR_SHADOW     = 1;
const unsigned int COLOR_FILL 	    = 2;
const unsigned int COLOR_DARKFILL   = 3;
const unsigned int COLOR_TEXTFORE   = 4;
const unsigned int COLOR_TEXTBACK   = 5;
const unsigned int COLOR_TEXTDISABLED = 6;


class awsPrefManager : public iAwsPrefs
{
   /// list of window definitions
  csDLinkList win_defs;

  /// list of skin definitions
  csDLinkList skin_defs;

  /// count of window defintions loaded
  unsigned int n_win_defs;

  /// count of skin defintions loaded
  unsigned int n_skin_defs;

  /// Currently selected skin 
  awsSkinNode *def_skin;

public:
    SCF_DECLARE_IBASE;

    awsPrefManager(iBase *iParent);
    virtual ~awsPrefManager();

    /// Invokes the parser to load a definitions file.
    virtual void Load(const char *def_file);

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

    
};
 
#endif
