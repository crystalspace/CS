
#ifndef __CFGFILE_H__
#define __CFGFILE_H__

#include "icfgnew.h"
#include "csutil/csvector.h"
#include "csutil/util.h"

class csConfigNode;
class csConfigIterator;

class csConfigFile : public iConfigFileNew
{
public:
  DECLARE_IBASE;

  csConfigFile (iBase *pBase);
  /// Create a new configuration object from the given file.
  csConfigFile(const char *Filename = NULL, iVFS *vfs = NULL);
  /// Delete this configuration.
  virtual ~csConfigFile();
  
  /**
   * Get config file name. It is recommended that all files
   * be loaded from VFS, thus no special means to determine
   * whenever the path is on VFS or not is provided.
   */
  virtual const char *GetFileName () const;

  /**
   * Set config file name. You can use this if you want Save()
   * to write to another file. This will set the dirty flag.
   */
  virtual void SetFileName (const char *fn, iVFS *vfs);

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
    bool Insert = false, bool Overwrite = true);

  /// Save INI to the same place it was loaded from.
  virtual bool Save () const;
  
  /**
   * Save INI into the given file (on VFS or on the physical filesystem).
   * If the second parameter is skipped, the file will be stored to
   * the physical filesystem, otherwise it is stored on given VFS object.
   * This will not change the internally stored file name.
   */
  virtual bool Save (const char *iFileName, iVFS *vfs = NULL) const;

  /// delete all options and rewind all iterators.
  virtual void Clear();

  /**
   * Enumerate all keys. If a subsection is given, only those keys are
   * enumerated that begin with this string. The returned iterator does
   * not yet point to a valid key. You must call Next() once to set it
   * to the first key. <p>
   */
  virtual iConfigIterator *Enumerate(const char *Subsection = NULL) const;
  
  /// Test if a key exists.
  virtual bool KeyExist(const char *Key) const;
  
  /// Get an integer value from the configuration.
  virtual int GetInt(const char *Key, int Def) const;
  /// Get a float value from the configuration.
  virtual float GetFloat(const char *Key, float Def = 0.0) const;
  /// Get a string value from the configuration.
  virtual const char *GetStr(const char *Key, const char *Def = "")
      const;
  /// Get a boolean value from the configuration.
  virtual bool GetBool(const char *Key, bool Def = false) const;
  /// Get the comment of the given key, or NULL if no comment exists.
  virtual const char *GetComment(const char *Key) const;
  
  /// Set an asciiz value.
  virtual void SetStr (const char *Key, const char *Val);
  /// Set an integer value.
  virtual void SetInt (const char *Key, int Value);
  /// Set a floating-point value.
  virtual void SetFloat (const char *Key, float Value);
  /// Set a boolean value.
  virtual void SetBool (const char *Key, bool Value);
  /**
   * Set the comment for given key. Set Text="" to put an empty comment
   * line before this key. Set Text = NULL to remove the comment. The
   * comment may contain newline characters.
   * Returns false if the key does not exist.
   */
  virtual bool SetComment (const char *Key, const char *Text);
  /// Delete a key and its comment.
  virtual void DeleteKey(const char *Name);

private:
  friend class csConfigIterator;

  // pointer to the root node (there is always two unnamed nodes at the
  // beginning and end of the list to make inserting and deleting nodes
  // easier).
  csConfigNode *FirstNode, *LastNode;
  // list of all iterators for this config object. This is required because
  // changes to the configuration may affect the iterators (e.g. when
  // you delete a key). Sorry, but this can't be a typed vector!
  csVector *Iterators;
  // current file name and file system
  char *Filename;
  iVFS *VFS;
  // are the current contents of this object different from the contents
  // stored in the config file?
  bool Dirty;
  
  // load the configuration from a data buffer and add it to the current
  // configuration. This may modify the contents of the file buffer but
  // will not delete it. This function will set the dirty flag if any
  // options have been added or modified.
  void LoadFromBuffer(char *Filedata, bool overwrite);
  // return a pointer to the node
  csConfigNode *FindNode(const char *Name) const;
  // create a new node in the list
  csConfigNode *CreateNode(const char *Name);
  // deregister an iterator
  void RemoveIterator(csConfigIterator *it) const;
  // save file without looking for dirty flag
  bool SaveNow(const char *Filename, iVFS *vfs) const;
};


/*******************/
/* private classes */
/*******************/

class csConfigNode
{
public:
  // create a new config node. Set name to NULL to create the initial node.
  csConfigNode(const char *Name);
  // delete this node
  ~csConfigNode();
  // delete all nodes will non-NULL name
  void DeleteDataNodes();
  // insert this node after the given node
  void InsertAfter(csConfigNode *Where);
  // remove this node from its list
  void Remove();
  // get the name of this node
  const char *GetName();
  // set the data for this key
  void SetStr(const char *s);
  void SetInt(int i);
  void SetFloat(float f);
  void SetBool(bool b);
  void SetComment(const char *s);
  // get data
  const char *GetStr();
  int GetInt();
  float GetFloat();
  bool GetBool();
  const char *GetComment();
  // return prev and next node
  csConfigNode *GetPrev();
  csConfigNode *GetNext();

private:
  // previous and next node
  csConfigNode *Prev, *Next;
  // key name
  char *Name;
  // key data
  char *Data;
  // comment
  char *Comment;
};

class csConfigIterator : public iConfigIterator
{
public:
  DECLARE_IBASE;

  /// Returns the configuration object for this iterator.
  virtual iConfigFileNew *GetConfigFile() const;
  /// Returns the subsection in the configuruation.
  virtual const char *GetSubsection() const;

  /// Rewind the iterator (points to nowhere after this)
  virtual void Rewind ();
  /// Move to previous item and return true if the position is valid
  virtual bool Prev ();
  /// Move to the next valid key. Returns false if no more keys exist.
  virtual bool Next();
  
  /**
   * Get the current key name. Set Local to true to return only the local
   * name inside the iterated subsection.
   */
  virtual const char *GetKey(bool Local = false) const;
  /// Get an integer value from the configuration.
  virtual int GetInt() const;
  /// Get a float value from the configuration.
  virtual float GetFloat() const;
  /// Get a string value from the configuration.
  virtual const char *GetStr() const;
  /// Get a boolean value from the configuration.
  virtual bool GetBool() const;
  /// Get the comment of the given key, or NULL if no comment exists.
  virtual const char *GetComment() const;

private:
  friend class csConfigFile;
  const csConfigFile *Config;
  csConfigNode *Node;
  char *Subsection;
  int SubsectionLength;

  // Create a new iterator. This should not be public. Always use
  // EnumerateKeys, because it does more than creating this object.
  csConfigIterator(const csConfigFile *Config, const char *Subsection);
  // delete this iterator
  virtual ~csConfigIterator();
  // move to the previous node, ignoring subsection
  bool DoPrev();
  // move to the next node, ignoring subsection
  bool DoNext();
};

#endif
