/*
    Copyright (C) 2003 by Greg Block

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

#ifndef __CS_CSSYS_MACOSX_DEFAULTSCFG_H__
#define __CS_CSSYS_MACOSX_DEFAULTSCFG_H__

#include "iutil/cfgfile.h"
#include "csutil/scf.h"

#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>

/**
 * An iConfigFile which stores settings via Cocoa's NSUserDefaults facility.
 */
class csDefaultsConfig : public iConfigFile
{
private:
  friend class csDefaultsIterator;

  NSString* domain;
  NSUserDefaults* defaults;
  NSMutableDictionary* dict;

  bool KeyExists (NSString* Key) const;
  bool Writable(NSString* Key) const;

public:
  SCF_DECLARE_IBASE;

  csDefaultsConfig ();
  virtual ~csDefaultsConfig();

  bool Open (const char* Key);

  virtual const char* GetFileName () const;
  virtual iVFS* GetVFS () const;
  virtual void SetFileName (const char*, iVFS*);
  virtual bool Load (
    const char* iFileName, iVFS* = 0, bool Merge = false, bool NewWins = true);
  virtual bool Save ();
  virtual bool Save (const char* iFileName, iVFS* = 0);
  
  virtual void Clear ();
  
  virtual csPtr<iConfigIterator> Enumerate (const char* Subsection = 0);
  virtual bool KeyExists (const char* Key) const;
  virtual bool SubsectionExists (const char* Subsection) const;
  
  virtual int GetInt (const char* Key, int Def = 0) const;
  virtual float GetFloat (const char* Key, float Def = 0.0) const;
  virtual const char* GetStr (const char* Key, const char* Def = "") const;
  virtual bool GetBool (const char* Key, bool Def = false) const;
  virtual const char* GetComment (const char* Key) const;

  virtual void SetStr (const char* Key, const char* Val);
  virtual void SetInt (const char* Key, int Value);
  virtual void SetFloat (const char* Key, float Value);
  virtual void SetBool (const char* Key, bool Value);
  virtual bool SetComment (const char* Key, const char* Text);
  virtual void DeleteKey (const char* Key);
  virtual const char* GetEOFComment () const;
  virtual void SetEOFComment (const char* Text);
};

/**
 * Iterates over a Defaults key subkeys and values.
 */
class csDefaultsIterator : public iConfigIterator
{
  csRef<csDefaultsConfig> owner;
  NSString* name;
  NSString* domain;

  csDefaultsConfig* config;
  
  NSEnumerator* keyenum;
  NSString* currentkey;
public:
  SCF_DECLARE_IBASE;

  csDefaultsIterator (csDefaultsConfig* Owner, const char* Subsection);
  virtual ~csDefaultsIterator();

  virtual iConfigFile* GetConfigFile () const;
  virtual const char* GetSubsection () const;

  virtual void Rewind ();
  virtual bool Next();

  virtual const char* GetKey (bool Local = false) const;
  virtual int GetInt () const;
  virtual float GetFloat () const;
  virtual const char* GetStr () const;
  virtual bool GetBool () const;
  virtual const char* GetComment () const;
};

#endif
