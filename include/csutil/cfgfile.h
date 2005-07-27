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

#ifndef __CS_UTIL_CFGFILE_H__
#define __CS_UTIL_CFGFILE_H__

/**\file
 * iConfigFile implementation for configurations stored in VFS files.
 */

#include "csextern.h"
#include "csutil/array.h"
#include "iutil/cfgfile.h"
#include "iutil/vfs.h"

class csConfigNode;
class csConfigIterator;

/**
 * Configuration file which implements the iConfigFile SCF interface.
 */
class CS_CRYSTALSPACE_EXPORT csConfigFile : public iConfigFile
{
public:
  SCF_DECLARE_IBASE;

  /// Create a new configuration object from the given file.
  csConfigFile(const char *Filename = 0, iVFS* = 0);
  /// Create a new empty configuration object.
  csConfigFile (iBase*);
  /// Delete this configuration.
  virtual ~csConfigFile();

  /// Is the configuration object empty?
  virtual bool IsEmpty() const;

  /// Get configuration file name.
  virtual const char *GetFileName () const;

  /**
   * Get the VFS object on which this file is stored (if any).  Returns 0
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
  virtual bool Save (const char *iFileName, iVFS* = 0);

  /// Delete all options and rewind all iterators.
  virtual void Clear();

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
  virtual int GetInt(const char *Key, int Def) const;
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
  /// Delete a key and its comment.
  virtual void DeleteKey(const char *Key);
  /// set the final comment at the end of the configuration file
  virtual void SetEOFComment(const char *Text);
  /// return the final comment at the end of the configuration file
  virtual const char *GetEOFComment() const;

private:
  friend class csConfigIterator;

  /*
   * pointer to the root node (there are always two unnamed nodes at the
   * beginning and end of the list to make inserting and deleting nodes
   * easier).
   */
  csConfigNode *FirstNode, *LastNode;
  /*
   * list of all iterators for this config object. This is required because
   * changes to the configuration may affect the iterators (e.g. when
   * you delete a key). Sorry, but this can't be a typed vector!
   */
  csArray<csConfigIterator*> *Iterators;
  // current file name and file system
  char *Filename;
  // the VFS filesystem used for this file (or 0 if not used)
  csRef<iVFS> VFS;
  /*
   * are the current contents of this object different from the contents
   * stored in the config file?
   */
  bool Dirty;
  // final comment at the end of the configuration file
  char *EOFComment;

  // private initialization function
  void InitializeObject ();
  // load the configuration from a file, ignoring the dirty flag
  virtual bool LoadNow(const char *Filename, iVFS *vfs, bool overwrite);
  /*
   * load the configuration from a data buffer and add it to the current
   * configuration. This may modify the contents of the file buffer but
   * will not delete it. This function will set the dirty flag if any
   * options have been added or modified.
   */
  virtual void LoadFromBuffer(char *Filedata, bool overwrite);
  // return a pointer to the named node or the first node of a subsection.
  csConfigNode *FindNode(const char *Name, bool isSubsection = false) const;
  // create a new node in the list
  csConfigNode *CreateNode(const char *Name);
  // deregister an iterator
  void RemoveIterator(csConfigIterator *it) const;
  // save file without looking for dirty flag
  virtual bool SaveNow(const char *Filename, iVFS *vfs) const;
};

#endif // __CS_UTIL_CFGFILE_H__
