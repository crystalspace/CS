/*
    Crystal Space .INI file management
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>
    Extensive functional overhaul by Eric Sunshine <sunshine@sunshineco.com>
    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __ICFGFILE_H__
#define __ICFGFILE_H__

#include "csutil/scf.h"

struct iVFS;
struct iConfigFile;

SCF_VERSION (iConfigSectionIterator, 0, 0, 1);

/**
 * This interface is the common parent for iConfigXXXXIterator classes.
 * The iterator is used to navigate amongst a specific subclass of data
 * items from a configuration file. Such objects are created by the
 * respective method from the iConfigFile interface, and then you query
 * from it all the information you are interested. When you are finished
 * with it, you DecRef the object as usual to destroy it.
 *<p>
 * Initially the iterator does not point to anything; you should call
 * some operator method before you access the data fields. Usage example:
 *<pre>
 * iConfigSectionIterator *s = inifile->EnumSections ();
 * while (s && s->Next ())
 *   printf ("Section: [%s]\n", s->GetSection ());
 * s->DecRef ();
 *</pre>
 */
struct iConfigSectionIterator : public iBase
{
  /// Query a pointer to the config file we're navigating: it is NOT IncRef'd.
  virtual iConfigFile *GetSource () const = 0;
  /// Rewind the iterator (points to nowhere after this)
  virtual void Rewind () = 0;
  /// Move to next item and return true if the position is valid
  virtual bool Next () = 0;
  /// Move to previous item and return true if the position is valid
  virtual bool Prev () = 0;
  /// Return the name of the current section
  virtual const char *GetSection () const = 0;
};

SCF_VERSION (iConfigDataIterator, 0, 0, 1);

/**
 * This is a iterator that is used to navigate through all data items
 * contained within a single section. During navigation GetSection()
 * will always return same value.
 */
struct iConfigDataIterator : public iConfigSectionIterator
{
  /// Return the current key name
  virtual const char *GetKey () const = 0;
  /// Return the current key data
  virtual csSome GetData () const = 0;
  /// Return the current data size
  virtual size_t GetDataSize () const = 0;
};

SCF_VERSION (iConfigCommentIterator, 0, 0, 1);

/**
 * This is a iterator that is used to navigate through all comments
 * tied to all keys within a specific section, or comments tied to
 * all section names.
 */
struct iConfigCommentIterator : public iConfigSectionIterator
{
  /// Return the current key name
  virtual const char *GetKey () const = 0;
  /// Return the current key data
  virtual const char *GetComment () const = 0;
};

SCF_VERSION (iConfigFile, 0, 0, 1);

/**
 * This is the interface to a object that provides access to a configuration
 * file. The configuration file can contain multiple sections, and every
 * section can contain multiple configuration values, each of which is
 * identified by a key which is an arbitrary string. Data items are
 * usually ASCII strings, although binary data is supported as well.
 */
struct iConfigFile : public iBase
{
  /**
   * Get config file name. It is recommended that all files
   * be loaded from VFS, thus no special means to determine
   * whenever the path is on VFS or not is provided.
   */
  virtual const char *GetFileName () = 0;

  /// Load INI from a file on a VFS volume (vfs != NULL) or from a physical file
  virtual bool Load (const char* fName, iVFS *vfs = NULL) = 0;

  /// Save INI to the same place it was loaded from.
  virtual bool Save () const = 0;
  /**
   * Save INI into the given file (on VFS or on the physical filesystem).
   * If the second parameter is skipped, the file will be stored to
   * the physical filesystem, otherwise it is stored on given VFS object.
   */
  virtual bool Save (const char *iFileName, iVFS *vfs = NULL) const = 0;

  /// Check if the config file is dirty
  virtual bool IsDirty () const = 0;

  /**
   * Returns a section iterator or NULL if no sections.
   * This allocates a new object; you should DecRef it when done.
   */
  virtual iConfigSectionIterator *EnumSections () const = 0;
  /**
   * Returns a data iterator or NULL if no data.
   * This allocates a new object; you should DecRef it when done.
   */
  virtual iConfigDataIterator *EnumData (const char* iSection) const = 0;
  /**
   * Returns a comment iterator or NULL if no comments; iKey can be NULL
   * in which case the comment iterator will be attached to all sections.
   * A single section name or key can have multiple comment lines attached.
   */
  virtual iConfigCommentIterator *EnumComments (const char* iSection,
    const char* iKey) const = 0;

  /// Query if a specific section exists
  virtual bool SectionExists (const char *iSection) const = 0;
  /// Query if a specific key exists
  virtual bool KeyExists (const char *iSection, const char *iKey) const = 0;

  /// Get data indexed by section name and key name
  virtual bool GetData (const char *iSection, const char *iKey,
    csSome &Data, size_t &DataSize) const = 0;
  /// Get an integer value from the configuration instance.
  virtual int GetInt (const char *iSection, const char *iKey,
    int def = 0) const = 0;
  /// Get a floating-point value from the configuration instance.
  virtual float GetFloat (const char *iSection, const char *iKey,
    float def = 0.0) const = 0;
  /// Get a string value from the configuration instance.
  virtual const char *GetStr (const char *iSection, const char *iKey,
    const char *def = "") const = 0;
  /// Get a boolean value from the configuration instance.
  virtual bool GetYesNo (const char *iSection, const char *iKey,
    bool def = false) const = 0;

  /// Set data: if Data is NULL, delete entry
  virtual bool SetData (const char *iSection, const char *iKey,
    csConstSome Data, size_t DataSize) = 0;
  /// Set an asciiz value (shortcut for SetData)
  virtual bool SetStr (const char *iSection, const char *iKey,
    const char *Val) = 0;
  /// Set an integer value (shortcut for SetData)
  virtual bool SetInt (const char *iSection, const char *iKey,
    int Value) = 0;
  /// Set an floating-point value (shortcut for SetData)
  virtual bool SetFloat (const char *iSection, const char *iKey,
    float Value) = 0;
  /// Set comment for given section/key; if Text = NULL adds a empty line
  virtual bool SetComment (const char *iSection, const char *iKey,
    const char *Text) = 0;

  /// Delete an entry
  inline bool DeleteKey (const char *iSection, const char *iKey)
  { return SetData (iSection, iKey, NULL, 0); }
  /// Delete all comments for a key or section (if iKey == NULL)
  bool DeleteComment (const char *iSection, const char *iKey);
};

#endif // __ICFGFILE_H__
