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

#ifndef __CS_IUTIL_CFGFILE_H__
#define __CS_IUTIL_CFGFILE_H__

/**\file
 * Configuration file interface.
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf_interface.h"
#include "csutil/ref.h"
struct iConfigIterator;
struct iVFS;


/**
 * Configuration file interface.
 */
struct iConfigFile : public virtual iBase
{
  SCF_INTERFACE(iConfigFile, 2,0,0);
  /// Get configuration file name.
  virtual const char* GetFileName () const = 0;

  /**
   * Get the VFS object on which this file is stored (if any).  Returns 0
   * if this file resides within the real (non-VFS) filesystem.
   */
  virtual iVFS* GetVFS () const = 0;

  /**
   * Set config file name. You can use this if you want Save()
   * to write to another file. This will set the dirty flag.
   */
  virtual void SetFileName (const char*, iVFS*) = 0;

  /**
   * Load a configuration file.
   * <p>
   * If the file resides in a real filesystem, rather than a VFS filesystem,
   * then pass 0 for the VFS argument.
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
    bool NewWins = true) = 0;

  /**
   * Save configuration to the same place from which it was loaded.  Returns
   * true if the save operation succeeded.
   */
  virtual bool Save () = 0;

  /**
   * Save configuration into the given file (on VFS or on the physical
   * filesystem).  If the second parameter is skipped, the file will be written
   * to the physical filesystem, otherwise it is stored on given VFS
   * filesystem.  This method does not change the internally stored file name.
   */
  virtual bool Save (const char *iFileName, iVFS* = 0) = 0;

  /// Delete all options and rewind all iterators.
  virtual void Clear () = 0;

  /**
   * Enumerate selected keys.  If a subsection is given, only those keys which
   * are prefixed by the subsection string will be enumerated.  The returned
   * iterator does not yet point to a valid key.  You must call Next() to set
   * it to the first key.
   */
  virtual csPtr<iConfigIterator> Enumerate (const char *Subsection = 0) = 0;

  /// Test if a key exists.
  virtual bool KeyExists (const char *Key) const = 0;
  /// Test if at least one key exists with the given Subsection prefix.
  virtual bool SubsectionExists (const char *Subsection) const = 0;

  /**
   * Get an integer value from the configuration. The optional default
   * value (Def parameter) will be used if the key was not found.
   */
  virtual int GetInt (const char *Key, int Def = 0) const = 0;
  /**
   * Get a float value from the configuration. The optional default
   * value (Def parameter) will be used if the key was not found.
   */
  virtual float GetFloat (const char *Key, float Def = 0.0) const = 0;
  /**
   * Get a string value from the configuration. The optional default
   * value (Def parameter) will be used if the key was not found.
   */
  virtual const char *GetStr (const char *Key, const char *Def = "") const = 0;
  /**
   * Get a boolean value from the configuration. The optional default
   * value (Def parameter) will be used if the key was not found.
   */
  virtual bool GetBool (const char *Key, bool Def = false) const = 0;
  /// Get the comment of the given key, or 0 if no comment exists.
  virtual const char *GetComment (const char *Key) const = 0;

  /// Set an null-terminated string value.
  virtual void SetStr (const char *Key, const char *Val) = 0;
  /// Set an integer value.
  virtual void SetInt (const char *Key, int Value) = 0;
  /// Set a floating-point value.
  virtual void SetFloat (const char *Key, float Value) = 0;
  /// Set a boolean value.
  virtual void SetBool (const char *Key, bool Value) = 0;
  /**
   * Set the comment for given key.  In addition to an actual comment, you can
   * use "" for Text to place an empty comment line before this key, or 0 to
   * remove the comment entirely.  The comment may contain newline characters.
   * Returns false if the key does not exist.
   */
  virtual bool SetComment (const char *Key, const char *Text) = 0;
  /// Delete a key and its value and comment.
  virtual void DeleteKey (const char *Key) = 0;
  /// return the final comment at the end of the configuration file
  virtual const char *GetEOFComment () const = 0;
  /// set the final comment at the end of the configuration file
  virtual void SetEOFComment (const char *Text) = 0;
};


/**
 * Iterator which allows sequential access to configuration information
 * contained in an iConfigFile object.
 */
struct iConfigIterator : public virtual iBase
{
  SCF_INTERFACE(iConfigIterator, 2,0,0);
  /// Returns the configuration object for this iterator.
  virtual iConfigFile *GetConfigFile () const = 0;
  /// Returns the subsection in the configuruation.
  virtual const char *GetSubsection () const = 0;

  /// Rewind the iterator (points to nowhere after this)
  virtual void Rewind () = 0;
  /// Move to the next valid key. Returns false if no more keys exist.
  virtual bool Next() = 0;

  /**
   * Get the current key name.  Set Local to true to return only the local name
   * inside the iterated subsection.  This is the portion of the key string
   * which follows the subsection prefix which was used to create this
   * iterator.
   */
  virtual const char *GetKey (bool Local = false) const = 0;
  /// Get an integer value from the configuration.
  virtual int GetInt () const = 0;
  /// Get a float value from the configuration.
  virtual float GetFloat () const = 0;
  /// Get a string value from the configuration.
  virtual const char *GetStr () const = 0;
  /// Get a boolean value from the configuration.
  virtual bool GetBool () const = 0;
  /// Get the comment of the given key, or 0 if no comment exists.
  virtual const char *GetComment () const = 0;
};
/** @} */

#endif // __CS_IUTIL_CFGFILE_H__
