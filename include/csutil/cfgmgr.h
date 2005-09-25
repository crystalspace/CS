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

#ifndef __CS_CFGMGR_H__
#define __CS_CFGMGR_H__

/**\file
 * Implementation for iConfigManager 
 */

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/refarr.h"
#include "iutil/cfgmgr.h"


class csConfigManagerIterator;

/**
 * A configuration manager makes a number of individual iConfigFile objects
 * appear to be a single configuration object.  See the description of the
 * iConfigManager interface for full details.
 */
class CS_CRYSTALSPACE_EXPORT csConfigManager : public iConfigManager
{
public:
  SCF_DECLARE_IBASE;

  /**
   * Create a new config manager object. If 'Optimize' is set to 'true', then
   * the configuration manager will enable some optimizations, which you may or
   * may not want:
   * <ul>
   * <li> When an iConfigFile is added via AddDomain(name, vfs), the
   *      configuration manager first looks through all registered iConfigFile
   *      objects. If an object with the same name and VFS pointer are found,
   *      it is added a second time, so the file is not loaded twice.
   * <li> When an iConfigFile is removed, the configuration manager keeps a
   *      reference to it until Flush() is called. If you add the iConfigFile
   *      again in the meantime with AddDomain(name, vfs), this reference is
   *      used instead.
   * </ul>
   */
  csConfigManager(iConfigFile *DynamicDomain = 0, bool Optimize = false);
  /// Destroy configuration manager.
  virtual ~csConfigManager();

  /**
   * Add a configuration domain.  The configuration manager invokes IncRef()
   * upon the incoming iConfigFile.
   */
  virtual void AddDomain(iConfigFile*, int priority);
  /**
   * Add a configuration domain by loading it from a file.  The new iConfigFile
   * object which represents the loaded file is also returned.  If you want to
   * hold onto the iConfigFile even after it is removed from this object or
   * after the configuration manager is destroyed, be sure to invoke IncRef()
   * or assign it to a csRef<>.  The incoming iVFS* may be null, in which case
   * the path is assumed to point at a file in the pyhysical filesystem, rather
   * than at a file in the virtual filesystem.
   */
  virtual iConfigFile* AddDomain(char const* path, iVFS*, int priority);
  /**
   * Remove a configuration domain.  If registered, the configuration manager
   * will relinquish its reference to the domain by invoking DecRef() on it to
   * balance the IncRef() it performed when the domain was added.  If the
   * domain is not registered, the RemoveDomain() request is ignored.  It is
   * not legal to remove the dynamic domain.
   */
  virtual void RemoveDomain(iConfigFile*);
  /// Remove a configuration domain.
  virtual void RemoveDomain(char const* path);
  /**
   * Find the iConfigFile object for a registered domain.  Returns null if the
   * domain is not registered.
   */
  virtual iConfigFile* LookupDomain(char const* path) const;
  /// Set the priority of a configuration domain.
  virtual void SetDomainPriority(char const* path, int priority);
  /**
   * Set the priority of a registered configuration domain.  If the domain is
   * not registered, the request is ignored.
   */
  virtual void SetDomainPriority(iConfigFile*, int priority);
  /**
   * Return the priority of a configuration domain.  If the domain is not
   * registered, PriorityMedium is returned.
   */
  virtual int GetDomainPriority(char const* path) const;
  /**
   * Return the priority of a configuration domain.  If the domain is not
   * registered, PriorityMedium is returned.
   */
  virtual int GetDomainPriority(iConfigFile*) const;

  /**
   * Change the dynamic domain.  The domain must already have been registered
   * with AddDomain() before calling this method.  If the domain is not
   * registered, then false is returned.
   */
  virtual bool SetDynamicDomain(iConfigFile*);
  /** 
   * Return a pointer to the dynamic configuration domain.  The returned
   * pointer will remain valid as long as the domain is registered with the
   * configuration manager.
   */
  virtual iConfigFile *GetDynamicDomain() const;
  /// Set the priority of the dynamic configuration domain.
  virtual void SetDynamicDomainPriority(int priority);
  /// Return the priority of the dynamic configuration domain.
  virtual int GetDynamicDomainPriority() const;

  /// flush all removed configuration files (only required in optimize mode)
  virtual void FlushRemoved();

  /**
   * Get configuration file name.  Also consult GetVFS() to determine which
   * (if any) VFS object was used for the file's storage.
   */
  virtual const char* GetFileName () const;

  /**
   * Get the VFS object on which this file is stored (if any).  Returns 0
   * if this file resides within the real (non-VFS) filesystem.
   */
  virtual iVFS* GetVFS () const;

  /**
   * Set config file name. You can use this if you want Save() to write to
   * another file. The incoming iVFS can be null, in which case the filename
   * represents a file in the physical fileysstem. This will set the dirty
   * flag.
   */
  virtual void SetFileName (const char*, iVFS*);

  /**
   * Load a configuration file.
   * <p>
   * If the file resides in a real filesystem, rather than a VFS filesystem,
   * then pass 0 for the VFS argument.  This will clear all options before
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
  virtual bool Load (const char* iFileName, iVFS* = 0, bool Merge = false,
    bool NewWins = true);

  /**
   * Save configuration to the same place from which it was loaded.  Returns
   * true if the save operation succeeded, else false.
   */
  virtual bool Save ();

  /**
   * Save configuration into the given file (on VFS or on the physical
   * filesystem).  If the iVFS parameter is null, the file will be written
   * to the physical filesystem, otherwise it is stored on given VFS
   * filesystem.  This method does not change the internally stored file name.
   */
  virtual bool Save (const char *iFileName, iVFS* = 0);

  /// Delete all options and rewind all iterators.
  virtual void Clear ();

  /// FlushRemoved() and delete all domains.
  void CleanUp ();

  /**
   * Enumerate selected keys.  If a subsection is given, only those keys which
   * are prefixed by the subsection string will be enumerated.  The returned
   * iterator does not yet point to a valid key.  You must call Next() to set
   * it to the first key.
   */
  virtual csPtr<iConfigIterator> Enumerate(const char *Subsection = 0);

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
  /// Get the comment of the given key, or 0 if no comment exists.
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
   * use "" for Text to place an empty comment line before this key, or 0 to
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
  /// optimize mode?
  bool Optimize;
  /// pointer to the dynamic config domain
  class csConfigDomain *DynamicDomain;
  /// list of all domains (including the dynamic domain)
  class csConfigDomain *FirstDomain, *LastDomain;
  /// list of all removed config files (only used in optimize mode)
  csRefArray<iConfigFile> Removed;
  /// list of all iterators
  csArray<iConfigIterator*> Iterators;

  csConfigDomain *FindConfig(iConfigFile *cfg) const;
  csConfigDomain *FindConfig(const char *name) const;
  void ClearKeyAboveDynamic(const char *Key);
  void RemoveIterator(csConfigManagerIterator *it);
  void FlushRemoved(size_t n);
  size_t FindRemoved(const char *Filename) const;
  void RemoveDomain(class csConfigDomain *cfg);
};

#endif // __CS_CFGMGR_H__
