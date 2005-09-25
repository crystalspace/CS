/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CSSYS_WIN32_REGISTRYCFG_H__
#define __CS_CSSYS_WIN32_REGISTRYCFG_H__

/**\file
 * Implementation for iConfigFile using the registry.
 */

#include "csextern.h"
#include "iutil/cfgfile.h"
#include "csutil/scf.h"
#include "csutil/strhash.h"
#include "csutil/array.h"

#include <windows.h>
#include "sanity.inc"

class csWin32RegistryIterator;

/**
 * An iConfigFile, storing the settings in the Windows registry.
 * \remarks This class provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the class and inclusion of the 
 *  header file should be surrounded by appropriate 
 *  '\#if defined(CS_PLATFORM_WIN32) ... \#endif' statements.
 */
class CS_CRYSTALSPACE_EXPORT csWin32RegistryConfig : public iConfigFile
{
private:
  friend class csWin32RegistryIterator;

  HKEY hKey;
  HKEY hKeyParent;
  char* Prefix;
  // whether this key is opened w/ write access.
  bool writeAccess;
  char* Key;

  typedef struct 
  {
    csStringHash strings;
  } rcStatus;
  rcStatus* status;

  csArray<csWin32RegistryIterator*> iters;

  // convert CS "x.y.z" keys to registry "x\y\z"
  void ReplaceSeparators (char* key) const;

  bool TryOpen (HKEY parent, HKEY& regKey, DWORD access, const char* keyName, 
    bool create);

  // convenience class, used to delete[] a buffer on function return
  struct Block_O_Mem
  {
    BYTE* data;
    size_t size;
    Block_O_Mem () : data(0), size(0) { }
    void Clear() { delete[] data; }
    void SetSize (size_t sz) { Clear(); size = sz; data = new BYTE[sz]; }
    ~Block_O_Mem() { Clear(); }
  };
  // To shorten calls to RegSetValueEx()
  bool InternalSetValue (const char* Key,
    DWORD type, const void* data, size_t datasize);
  // To shorten calls to RegQueryValueEx()
  bool InternalGetValue (const char* Key,
    DWORD& type, Block_O_Mem& data) const;

  /*
    Helper functions to convert the data from the registry to the 
    requested format
  */
  int RegToInt (DWORD type, Block_O_Mem& data, int Def) const;
  float RegToFloat (DWORD type, Block_O_Mem& data, float Def) const;
  const char* RegToStr (DWORD type, Block_O_Mem& data, const char* Def) const;
  bool RegToBool (DWORD type, Block_O_Mem& data, bool Def) const;

  // Check whether we have registry write access.
  bool WriteAccess();
public:
  SCF_DECLARE_IBASE;

  csWin32RegistryConfig ();
  virtual ~csWin32RegistryConfig();

  /**
   * Open a registry key.
   * This will open the key named \p Key as a subkey of \p parent.
   * \remarks The key must be the full path, e.g. "Software\CrystalSpace".
   * \remarks If \p parent is none of the default HKEY_ roots, the key must
   *   remain open as long as a registry config object isn't Close()d.
   */
  bool Open (const char* Key, HKEY parent = HKEY_CURRENT_USER);
  /**
   * Close the current registry key.
   * Use this if you want reuse a registry config object at a later time but
   * want to free it's resources for the time being.
   * \remarks This is called automatically on destruction or Open().
   */
  void Close ();

  virtual const char* GetFileName () const;
  virtual iVFS* GetVFS () const;
  virtual void SetFileName (const char*, iVFS*);
  virtual bool Load (const char* iFileName, iVFS* = 0, bool Merge = false,
    bool NewWins = true);
  virtual bool Save ();
  virtual bool Save (const char *iFileName, iVFS* = 0);
  
  virtual void Clear ();
  
  virtual csPtr<iConfigIterator> Enumerate (const char *Subsection = 0);
  virtual bool KeyExists (const char *Key) const;
  virtual bool SubsectionExists (const char *Subsection) const;
  
  virtual int GetInt (const char *Key, int Def = 0) const;
  virtual float GetFloat (const char *Key, float Def = 0.0) const;
  virtual const char *GetStr (const char *Key, const char *Def = "") const;
  virtual bool GetBool (const char *Key, bool Def = false) const;
  virtual const char *GetComment (const char *Key) const;

  virtual void SetStr (const char *Key, const char *Val);
  virtual void SetInt (const char *Key, int Value);
  virtual void SetFloat (const char *Key, float Value);
  virtual void SetBool (const char *Key, bool Value);
  virtual bool SetComment (const char *Key, const char *Text);
  virtual void DeleteKey (const char *Key);
  virtual const char *GetEOFComment () const;
  virtual void SetEOFComment (const char *Text);
};

/**
 * Iterates over a registry key subkeys and values.
 * \remarks This class provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the interface and inclusion of the 
 *  header file should be surrounded by appropriate 
 *  '\#if defined(CS_PLATFORM_WIN32) ... \#endif' statements.
 */
class CS_CRYSTALSPACE_EXPORT csWin32RegistryIterator : public iConfigIterator
{
  csRef<csWin32RegistryConfig> owner;

  typedef struct 
  {
    csStringHash strings;
  } riStatus;
  riStatus* status;

  DWORD EnumIndex;

  char* SubsectionName;

  // shortcut to RegEnumValue/RegQueryValueEx
  bool GetCurrentData (DWORD& type, 
    csWin32RegistryConfig::Block_O_Mem& data) const;
public:
  SCF_DECLARE_IBASE;

  csWin32RegistryIterator (csWin32RegistryConfig* Owner, 
    const char* Subsection);
  virtual ~csWin32RegistryIterator();

  virtual iConfigFile *GetConfigFile () const;
  virtual const char *GetSubsection () const;

  virtual void Rewind ();
  virtual bool Next();

  virtual const char *GetKey (bool Local = false) const;
  virtual int GetInt () const;
  virtual float GetFloat () const;
  virtual const char *GetStr () const;
  virtual bool GetBool () const;
  virtual const char *GetComment () const;
};

#endif

