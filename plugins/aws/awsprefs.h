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
 
/// Abstract key interface
class awsKey
{
  /// The name of the key
  iString *name;

public:
  /// Simple constructor creates new key with name "n"
  awsKey(iString *n):name(n) {};

  /// Simple destructor does nothing
  virtual ~awsKey()         {};
  
  /// Pure virtual function Type returns the type of key
  virtual unsigned char Type()=0;

  /// Accessor function gets name of key
  iString              *Name() { return name; }
  
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

  /// Adds an item to the container
  void Add(awsKey *key) 
  { children.AddItem(key); }

  /// Removes an item from the container
  void Remove(iString *name)
  { children.RemoveItem(Find(name)); }

  /// Removes a specific item from the container
  void Remove(awsKey *key)
  { children.RemoveItem(key); }
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

//////////////////////////////////  Preference Manager ////////////////////////////////////////////////////////////////

class awsPrefManager : public iAwsPrefs
{
   

public:
    DECLARE_IBASE;

    awsPrefManager(iBase *iParent);
    virtual ~awsPrefManager();

    virtual void Load(char *def_file);

};
 
#endif
