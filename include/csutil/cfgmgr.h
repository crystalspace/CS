/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CFGMGR_H__
#define __CFGMGR_H__

#include "iutil/cfgmgr.h"
#include "csutil/csvector.h"

/**
 * A configuration manager makes a number of individual configuration files
 * appear to be a single configuration object.  See the description of the SCF
 * interface iConfigManager for full details.
 */
class csConfigManager : public iConfigManager
{
public:
  SCF_DECLARE_IBASE;

  /**
   * Create a new config manager object. If 'Optimize' is set to 'true', then
   * the config manager will enable some optimizations, which you may or may
   * not want: <ul>
   * <li> When a config file is added via Add(name, vfs), the config manager
   * first looks through all registered config files. If a file with the same
   * name and vfs pointer are found, it is added a second time, so the file is
   * not loaded twice.
   * <li> When a config file is removed, the config manager keeps a reference
   * to it until Flush() is called. If you add the file again in the meantime
   * with Add(name, vfs), this reference is used instead.
   * </ul>
   */
  csConfigManager(iConfigFile *DynamicDomain, bool Optimize);
  /// Delete this config manager.
  virtual ~csConfigManager();

  /// add a configuration domain
  virtual void AddDomain(iConfigFile*, int priority);
  /// add a configuration domain
  virtual iConfigFile *AddDomain(char const* path, iVFS*, int priority);
  /// remove a configuration domain
  virtual void RemoveDomain(iConfigFile*);
  /// remove a configuration domain
  virtual void RemoveDomain(char const* path);
  /// return a pointer to a single config domain
  virtual iConfigFile* LookupDomain(char const* path) const;
  /// set the priority of a config domain
  virtual void SetDomainPriority(char const* path, int priority);
  /// set the priority of a config domain
  virtual void SetDomainPriority(iConfigFile*, int priority);
  /// return the priority of a config domain
  virtual int GetDomainPriority(char const* path) const;
  /// return the priority of a config domain
  virtual int GetDomainPriority(iConfigFile*) const;

  /**
   * Change the dynamic domain. The given config object must already be
   * registered with AddDomain(). Returns false if this is not the case.
   */
  virtual bool SetDynamicDomain(iConfigFile*);
  /// return a pointer to the dynamic config domain
  virtual iConfigFile *GetDynamicDomain() const;
  /// set the priority of the dynamic config domain
  virtual void SetDynamicDomainPriority(int priority);
  /// return the priority of the dynamic config domain
  virtual int GetDynamicDomainPriority() const;

  /// flush all removed config files (only required in optimize mode)
  virtual void FlushRemoved();

  /**
   * Get configuration file name.  Also consult GetVFS() to determine which
   * (if any) VFS object was used for the file's storage.
   */
  virtual const char* GetFileName () const;

  /**
   * Get the VFS object on which this file is stored (if any).  Returns NULL
   * if this file resides within the real (non-VFS) filesystem.
   */
  virtual iVFS* GetVFS () const;

  /**
   * Set config file name. You can use this if you want Save()
   * to write to another file. This will set the dirty flag.
   */
  virtual void SetFileName (const char*, iVFS*);

  /**
   * Load a configuration file.
   * <p>
   * If the file resides in a real filesystem, rather than a VFS filesystem,
   * then pass NULL for the VFS argument.  This will clear all options before
   * loading the new options, even if the file cannot be opened.
   * <p>
   * You can set the Merge flag to merge the newly loaded configuration
   * information into the existing information.  If you do so, nothing will
   * happen if the named file doesn't exist.  The NewWins flag determines
   * the behavior in case of configuration key conflicts.  If true, then the
   * new configuration value replaces the old for that key.  If false, then the
   * old value is kept, and the new value is ignored.  The recorded file name
   * will be set to the name of the newly loaded file if the Merge flag is
   * false; otherwise it will retain the old name.
   */
  virtual bool Load (const char* iFileName, iVFS* = NULL, bool Merge = false,
    bool NewWins = true);

  /**
   * Save configuration to the same place from which it was loaded.  Returns
   * true if the save operation succeeded.
   */
  virtual bool Save ();

  /**
   * Save configuration into the given file (on VFS or on the physical
   * filesystem).  If the second parameter is skipped, the file will be written
   * to the physical filesystem, otherwise it is stored on given VFS
   * filesystem.  This method does not change the internally stored file name.
   */
  virtual bool Save (const char *iFileName, iVFS* = NULL);

  /// Delete all options and rewind all iterators.
  virtual void Clear ();

  /**
   * FlushRemoved() and delete all domains.
   */
  virtual void CleanUp ();

  /**
   * Enumerate selected keys.  If a subsection is given, only those keys which
   * are prefixed by the subsection string will be enumerated.  The returned
   * iterator does not yet point to a valid key.  You must call Next() to set
   * it to the first key.
   */
  virtual iConfigIterator *Enumerate(const char *Subsection = NULL);

  /// Test if a key exists.
  virtual bool KeyExists(const char *Key) const;
  /// Test if at least one key exists with the given Subsection prefix.
  virtual bool SubsectionExists(const char *Subsection) const;

  /// Get an integer value from the configuration.
  virtual int GetInt(const char *Key, int Def = 0) const;
  /// Get a float value from the configuration.
  virtual float GetFloat(const char *Key, float Def = 0.0) const;
  /// Get a string value from the configuration.
  virtual const char *GetStr(const char *Key, const char *Def = "") const;
  /// Get a boolean value from the configuration.
  virtual bool GetBool(const char *Key, bool Def = false) const;
  /// Get the comment of the given key, or NULL if no comment exists.
  virtual const char *GetComment(const char *Key) const;

  /// Set an null-terminated string value.
  virtual void SetStr (const char *Key, const char *Val);
  /// Set an integer value.
  virtual void SetInt (const char *Key, int Value);
  /// Set a floating-point value.
  virtual void SetFloat (const char *Key, float Value);
  /// Set a boolean value.
  virtual void SetBool (const char *Key, bool Value);
  /**
   * Set the comment for given key.  In addition to an actual comment, you can
   * use "" for Text to place an empty comment line before this key, or NULL to
   * remove the comment entirely.  The comment may contain newline characters.
   * Returns false if the key does not exist.
   */
  virtual bool SetComment (const char *Key, const char *Text);
  /// Delete a key and its value and comment.
  virtual void DeleteKey(const char *Key);
  /// return the final comment at the end of the configuration file
  virtual const char *GetEOFComment() const;
  /// set the final comment at the end of the configuration file
  virtual void SetEOFComment(const char *Text);

private:
  friend class csConfigManagerIterator;
  // optimize mode?
  bool Optimize;
  // pointer to the dynamic config domain
  class csConfigDomain *DynamicDomain;
  // list of all domains (including the dynamic domain)
  class csConfigDomain *FirstDomain, *LastDomain;
  // list of all removed config files (only used in optimize mode)
  csVector Removed;
  // list of all iterators
  csVector Iterators;

  csConfigDomain *FindConfig(iConfigFile *cfg) const;
  csConfigDomain *FindConfig(const char *name) const;
  void ClearKeyAboveDynamic(const char *Key);
  void RemoveIterator(csConfigManagerIterator *it);
  void FlushRemoved(int n);
  int FindRemoved(const char *Filename) const;
  void RemoveDomain(class csConfigDomain *cfg);
};

#endif // __CFGMGR_H__
