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

#ifndef __CS_INIFILE_H__
#define __CS_INIFILE_H__

#include "icfgfile.h"
#include "csutil/csstring.h"
#include "csutil/csvector.h"
#include "ivfs.h"

class csStrVector;
struct iStrVector;

class csIniFile : public iConfigFile
{
private:
  // Branch types
  enum BranchType { TYPE_SECTION, TYPE_DATA, TYPE_COMMENT };

  // Private class
  class PrvINIbranch : public csVector
  {
  public:
    virtual ~PrvINIbranch ();
    virtual bool FreeItem (csSome Item);
  };
  friend class csIniFile::PrvINIbranch;

  // Private structure
  struct PrvINInode
  {
    BranchType Type;                      // Section type
    PrvINIbranch *Comments;    		  // Comments to this section
    union
    {
      struct
      {
        char *Name;                       // Section name
        PrvINIbranch *Vector;  		  // A vector of values
      } Section;
      struct
      {
        char *Name;                       // Key name
        size_t Size;                      // Guess what?
        csSome Pointer;                   // ...likewise
      } Data;
      struct
      {
        char *Text;                       // Comment text
      } Comment;
    };
  };

  /// Abstract iterator
  class Iterator
  {
  protected:
    const csIniFile *Source;
    const PrvINIbranch* Branch;
    BranchType Type;
    const PrvINInode* Node;
    int Current;
    int Limit;

    // Constructor
    Iterator (const csIniFile *s, const PrvINIbranch* b, BranchType t) :
      Source (s), Branch (b), Type (t), Node (NULL), Current (-1),
      Limit (b ? b->Length () : 0) {}
    virtual ~Iterator() {}

    // Remove the item we are currently pointing to
    void RemoveItem ();
    // Get the comments assocuated with current item
    const PrvINIbranch* GetComments () const { return Node->Comments; }

  public:
    /// Rewind the iterator (points to nowhere after this)
    virtual void Rewind ()
    { Current = -1; Node = NULL; }
    /// Move to next item and return true if the position is valid
    virtual bool Next ();
    /// Move to previous item and return true if the position is valid
    virtual bool Prev ();
  };

  /// Section iterator
  class SectionIterator : public iConfigSectionIterator, public Iterator
  {
    typedef Iterator superclass;
  protected:
    friend class csIniFile;

    // Constructor
    SectionIterator (const csIniFile *s, const PrvINIbranch* b) :
      Iterator (s, b, TYPE_SECTION) { CONSTRUCT_IBASE (NULL); }
    // Destructor
    virtual ~SectionIterator();

  public:
    DECLARE_IBASE;

    /// Query a pointer to the config file we're navigating: it is NOT IncRef'd.
    virtual iConfigFile *GetSource () const
    { return (iConfigFile *)Source; }
    /// Rewind the iterator (points to nowhere after this)
    virtual void Rewind ()
    { superclass::Rewind (); }
    /// Move to next item and return true if the position is valid
    virtual bool Next ()
    { return superclass::Next (); }
    /// Move to previous item and return true if the position is valid
    virtual bool Prev ()
    { return superclass::Prev (); }
    /// Return the name of the current section
    virtual const char *GetSection () const
    { return Node->Section.Name; }
  };
  friend class csIniFile::SectionIterator;

  /// A data iterator
  class DataIterator : public iConfigDataIterator, public Iterator
  {
    typedef Iterator superclass;
  protected:
    friend class csIniFile;
    const char *Section;

    DataIterator(const csIniFile *s, const PrvINInode *n) :
      Iterator (s, n->Section.Vector, TYPE_DATA)
    { CONSTRUCT_IBASE (NULL); Section = n->Section.Name; }
    // Destructor
    virtual ~DataIterator();

  public:
    DECLARE_IBASE;

    /// Query a pointer to the config file we're navigating: it is NOT IncRef'd.
    virtual iConfigFile *GetSource () const
    { return (iConfigFile *)Source; }
    /// Rewind the iterator (points to nowhere after this)
    virtual void Rewind ()
    { superclass::Rewind (); }
    /// Move to next item and return true if the position is valid
    virtual bool Next ()
    { return superclass::Next (); }
    /// Move to previous item and return true if the position is valid
    virtual bool Prev ()
    { return superclass::Prev (); }
    /// Return the name of the current section
    virtual const char *GetSection () const
    { return Section; }
    /// Return the current key name
    virtual const char *GetKey () const
    { return Node->Data.Name; }
    /// Return the current key data
    virtual csSome GetData () const
    { return Node->Data.Pointer; }
    /// Return the current data size
    virtual size_t GetDataSize () const
    { return Node->Data.Size; }
  };
  friend class csIniFile::DataIterator;

  /// A comment iterator
  class CommentIterator : public iConfigCommentIterator, public Iterator
  {
    typedef Iterator superclass;
  protected:
    friend class csIniFile;
    const char *Section;
    const char *Key;

    CommentIterator (const csIniFile *s, const PrvINIbranch *b,
      const char *iSection, const char *iKey) : Iterator (s, b, TYPE_COMMENT)
    { CONSTRUCT_IBASE (NULL); Section = iSection; Key = iKey; }
    virtual ~CommentIterator ();

  public:
    DECLARE_IBASE;

    /// Query a pointer to the config file we're navigating: it is NOT IncRef'd.
    virtual iConfigFile *GetSource () const
    { return (iConfigFile *)Source; }
    /// Rewind the iterator (points to nowhere after this)
    virtual void Rewind ()
    { superclass::Rewind (); }
    /// Move to next item and return true if the position is valid
    virtual bool Next ()
    { return superclass::Next (); }
    /// Move to previous item and return true if the position is valid
    virtual bool Prev ()
    { return superclass::Prev (); }
    /// Return the section this data iterator acts on
    virtual const char *GetSection () const
    { return Section; }
    /// Return the current key name
    virtual const char *GetKey () const
    { return Key; }
    /// Return the current key data
    virtual const char *GetComment () const
    { return Node->Comment.Text; }
  };
  friend class csIniFile::CommentIterator;

  /// Returns a section iterator; sets Found to true if any sections exist
  SectionIterator *_EnumSections () const;
  /// Returns a data iterator or NULL if no data.
  DataIterator *_EnumData (const char* iSection) const;
  /// Returns a comment iterator or NULL if no comments; iKey can be NULL
  CommentIterator *_EnumComments (const char* iSection,
    const char* iKey) const;

  /// The root of INI file
  PrvINIbranch Root;
  /// One of '#' or ';' (used in Save ())
  char CommentChar;
  /// Is object content syncronised to disk?
  bool Dirty;
  /// File name it was loaded from. If NULL, it was loaded from memory.
  char *FileName;
  /// A pointer to VFS volume, if config file was read from VFS or NULL
  iVFS *VFS;

public:
  DECLARE_IBASE;

  /// Initialize an empty INI file object
  csIniFile (char iCommentChar = ';') : CommentChar (iCommentChar),
    Dirty (false), FileName (NULL), VFS (NULL)
  { CONSTRUCT_IBASE (NULL); }
  /// Initialize INI file object and load it from a physical file
  csIniFile (const char* fName, char iCommentChar = ';');
  /// Initialize INI file object and load it from a file on VFS volume
  csIniFile (const char* fName, iVFS *vfs, char iCommentChar = ';');
  /// Destroy the object
  virtual ~csIniFile ();

  /// Load INI from a file on a VFS volume or from a physical file
  virtual bool Load (const char* fName, iVFS *vfs = NULL);

  /// Save INI to the same place it was loaded from.
  virtual bool Save () const;
  /// Save INI into the given file (on VFS or on physical filesystem)
  virtual bool Save (const char *iFileName, iVFS *vfs = NULL) const;

  /// Check if the config file is dirty
  virtual bool IsDirty () const { return Dirty; }
  /// Clear dirty flag (is this safe to expose to user?)
  void ClearDirty() { Dirty = false; }

  /// Override to type your own parsing error messages
  virtual bool Error (int LineNo, const char *Line, int Pos);

  /**
   * Get config file name. It is recommended that all files
   * be loaded from VFS, thus no special means to determine
   * whenever the path is on VFS or not is provided.
   */
  virtual const char *GetFileName ()
  { return FileName; }

  /// Returns a section iterator or NULL if no sections
  virtual iConfigSectionIterator *EnumSections () const
  { return _EnumSections (); }
  /// Returns a data iterator or NULL if no data.
  virtual iConfigDataIterator *EnumData (const char* iSection) const
  { return _EnumData (iSection); }
  /// Returns a comment iterator or NULL if no comments; iKey can be NULL
  virtual iConfigCommentIterator *EnumComments (const char* iSection,
    const char* iKey) const
  { return _EnumComments (iSection, iKey); }

  /// Query if a specific section exists
  virtual bool SectionExists (const char *iSection) const;
  /// Query if a specific key exists
  virtual bool KeyExists (const char *iSection, const char *iKey) const;

  /// Get data indexed by section name and key name
  virtual bool GetData (const char *iSection, const char *iKey, csSome &Data,
    size_t &DataSize) const;
  /// Get an integer value from the configuration instance.
  virtual int GetInt (const char *iSection, const char *iKey, int def = 0) const;
  /// Get a real value from the configuration instance.
  virtual float GetFloat (const char *iSection, const char *iKey,
    float def = 0.0) const;
  /// Get a string value from the configuration instance.
  virtual const char *GetStr (const char *iSection, const char *iKey,
    const char *def = "") const;
  /// Get a boolean value from the configuration instance.
  virtual bool GetYesNo (const char *iSection, const char *iKey,
    bool def = false) const;

  /// Set data: if Data is NULL, delete entry
  virtual bool SetData (const char *iSection, const char *iKey,
    csConstSome Data, size_t DataSize);
  /// Set an asciiz value (shortcut for SetData)
  virtual bool SetStr (const char *iSection, const char *iKey, const char *Val);
  /// Set an integer value (shortcut for SetData)
  virtual bool SetInt (const char *iSection, const char *iKey, int Value);
  /// Set an floating-point value (shortcut for SetData)
  virtual bool SetFloat (const char *iSection, const char *iKey, float Value);
  /// Set comment for given section/entry
  virtual bool SetComment (const char *iSection, const char *iKey,
    const char *Text);

  /// Delete an entry
  bool DeleteKey (const char *iSection, const char *iKey)
  { return SetData (iSection, iKey, NULL, 0); }
  /// Delete all comments for a key or section (if iKey == NULL)
  bool DeleteComment (const char *iSection, const char *iKey);

private:
  // Load INI from memory buffer
  bool _Load (const char *Data, size_t DataSize);
  /// Save INI to a string (common wrapper for all Save()'s)
  csString _Save () const;
  // Find node by given path
  PrvINInode* FindNode (const char *iSection, const char* iKey) const;
  // Set the data for a given node
  void SetData (PrvINInode*, csConstSome Data, size_t DataSize);
  // Save a comment
  void SaveComment (const char* Text, csString&) const;
  // Save comments for a section or key
  void SaveComments (const PrvINIbranch*, csString&) const;
  // Save all the keys within a section
  void SaveData (const char* Name, csSome Data, size_t DataSize,
    const PrvINIbranch* comments, csString&) const;
  // Save all the data in a section
  void SaveSection (const PrvINInode*, csString&) const;
};

#endif // __CS_INIFILE_H__
