
#ifndef __ICFGNEW_H__
#define __ICFGNEW_H__

#include "csutil/scf.h"

struct iConfigIterator;
struct iVFS;


SCF_VERSION(iConfigFileNew, 0, 0, 1);

/**
 * @@@ Document me !!!
 */
struct iConfigFileNew : public iBase
{
  /**
   * Get config file name. It is recommended that all files
   * be loaded from VFS, thus no special means to determine
   * whenever the path is on VFS or not is provided.
   */
  virtual const char *GetFileName () const = 0;
  
  /**
   * Set config file name. You can use this if you want Save()
   * to write to another file. This will set the dirty flag.
   */
  virtual void SetFileName (const char *fn, iVFS *vfs) = 0;

  /**
   * Load INI from a file on a VFS volume (vfs != NULL) or
   * from a physical file. This will clear all options before
   * loading the new options, even if the file cannot be opened. <p>
   *
   * You can set the Insert flag to keep the old options. If you do this,
   * nothing will happen if the file doesn't exist. The Overwrite flag
   * determines whether the old (false) or new (true, default) options
   * will be loaded in case of a name conflict. The internally stored
   * file name will not be modified when the Insert flag is set.
   */
  virtual bool Load (const char* fName, iVFS *vfs = NULL,
    bool Insert = false, bool Overwrite = true) = 0;

  /// Save INI to the same place it was loaded from.
  virtual bool Save () const = 0;
  
  /**
   * Save INI into the given file (on VFS or on the physical filesystem).
   * If the second parameter is skipped, the file will be stored to
   * the physical filesystem, otherwise it is stored on given VFS object.
   * This will not change the internally stored file name.
   */
  virtual bool Save (const char *iFileName, iVFS *vfs = NULL) const = 0;

  /// delete all options and rewind all iterators.
  virtual void Clear() = 0;

  /**
   * Enumerate all keys. If a subsection is given, only those keys are
   * enumerated that begin with this string. The returned iterator does
   * not yet point to a valid key. You must call Next() once to set it
   * to the first key. <p>
   */
  virtual iConfigIterator *Enumerate(const char *Subsection = NULL) const = 0;
  
  /// Test if a key exists.
  virtual bool KeyExist(const char *Key) const = 0;
  
  /// Get an integer value from the configuration.
  virtual int GetInt(const char *Key, int Def = 0) const = 0;
  /// Get a float value from the configuration.
  virtual float GetFloat(const char *Key, float Def = 0.0) const = 0;
  /// Get a string value from the configuration.
  virtual const char *GetStr(const char *Key, const char *Def = "")
      const = 0;
  /// Get a boolean value from the configuration.
  virtual bool GetBool(const char *Key, bool Def = false) const = 0;
  /// Get the comment of the given key, or NULL if no comment exists.
  virtual const char *GetComment(const char *Key) const = 0;
  
  /// Set an asciiz value.
  virtual void SetStr (const char *Key, const char *Val) = 0;
  /// Set an integer value.
  virtual void SetInt (const char *Key, int Value) = 0;
  /// Set a floating-point value.
  virtual void SetFloat (const char *Key, float Value) = 0;
  /// Set a boolean value.
  virtual void SetBool (const char *Key, bool Value) = 0;
  /**
   * Set the comment for given key. Set Text="" to put an empty comment
   * line before this key. Set Text = NULL to remove the comment. The
   * comment may contain newline characters.
   * Returns false if the key does not exist.
   */
  virtual bool SetComment (const char *Key, const char *Text) = 0;
  /// Delete a key and its comment.
  virtual void DeleteKey(const char *Name) = 0;
};


SCF_VERSION(iConfigIterator, 0, 0, 1);

/**
 * @@@ Document me !!!
 */
struct iConfigIterator : public iBase
{
  /// Returns the configuration object for this iterator.
  virtual iConfigFileNew *GetConfigFile() const = 0;
  /// Returns the subsection in the configuruation.
  virtual const char *GetSubsection() const = 0;

  /// Rewind the iterator (points to nowhere after this)
  virtual void Rewind () = 0;
  /// Move to previous item and return true if the position is valid
  virtual bool Prev () = 0;
  /// Move to the next valid key. Returns false if no more keys exist.
  virtual bool Next() = 0;
  
  /**
   * Get the current key name. Set Local to true to return only the local
   * name inside the iterated subsection.
   */
  virtual const char *GetKey(bool Local = false) const = 0;
  /// Get an integer value from the configuration.
  virtual int GetInt() const = 0;
  /// Get a float value from the configuration.
  virtual float GetFloat() const = 0;
  /// Get a string value from the configuration.
  virtual const char *GetStr() const = 0;
  /// Get a boolean value from the configuration.
  virtual bool GetBool() const = 0;
  /// Get the comment of the given key, or NULL if no comment exists.
  virtual const char *GetComment() const = 0;
};

#endif
