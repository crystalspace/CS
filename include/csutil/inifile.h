/*
    Crystal Space .INI file management
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include "csutil/csvector.h"
#include "ivfs.h"

class csVFS;
class csStrVector;

typedef bool (csIniWriteFunc) (csSome Stream, const void *data,
  unsigned int iniWriteType, size_t len);

class csIniFile : public csBase
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
    const csIniFile& Source;
    const PrvINIbranch* Branch;
    BranchType Type;
    const PrvINInode* Node;
    int Current;
    int Limit;
    Iterator(const csIniFile& s, const PrvINIbranch* b, BranchType t) :
      Source(s), Branch(b), Type(t), Node(NULL), Current(-1),
      Limit(b ? b->Length():0) {}
    void Clone(const Iterator&);
    void RemoveItem();
    const PrvINIbranch* GetComments() const { return Node->Comments; }
  public:
    /// Copy constructor
    Iterator(const Iterator& i) : Source(i.Source) { Clone(i); }
    /// Assignment operator
    Iterator& operator=(const Iterator& i) { Clone(i); return *this; }
    /// Returns true if another item exists; must call at least once
    bool NextItem();
    /// Returns the INI file over which this object iterates
    const csIniFile& GetSource() const { return Source; }
  };
  friend class csIniFile::Iterator;

  /// The root of INI file
  PrvINIbranch Root;
  /// One of '#' or ';' (used in Save ())
  char CommentChar;
  /// Is object content syncronised to disk?
  bool Dirty;

public:
  /// Initialize INI file object
  csIniFile (char Comment = ';') : CommentChar(Comment), Dirty(false) {}
  /// Initialize INI file object and load it from a file
  csIniFile (const char* path, char Comment = ';');
  /// Initialize INI file object and load it from a file on VFS volume
  csIniFile (csVFS* vfs, const char* path, char iCommentChar = ';');
  /// Initialize INI file object and load it from an iFile
  csIniFile (iFile* f, char iCommentChar = ';');
  /// Destroy the object
  virtual ~csIniFile ();

  /// Load INI from file
  bool Load (const char *fName);
  /// Load INI from iFile
  bool Load (iFile *f);
  /// Load INI from memory buffer
  bool Load (const char *Data, size_t DataSize);
  /// Override to type your own error messages
  virtual bool Error (int LineNo, const char *Line, int Pos);

  /// Save INI file
  bool Save (const char *fName);
  bool Save (iFile *f);

  /// A section iterator
  class SectionIterator : public Iterator
  {
    typedef Iterator superclass;
  protected:
    friend class csIniFile;
    SectionIterator(const csIniFile& s, const PrvINIbranch* b) :
      Iterator (s, b, TYPE_SECTION) {}
  public:
    /// Returns the section's name
    const char* GetName() const { return Node->Section.Name; }
  };
  friend class csIniFile::SectionIterator;

  /// Returns a section iterator; sets Found to true if any sections exist
  SectionIterator EnumSections(bool& Found) const;
  /// Returns a section iterator
  SectionIterator EnumSections() const { bool b; return EnumSections(b); }
  /// Enumerate sections and put section names into a string vector
  bool EnumSections (csStrVector *oList) const;

  /// A data iterator
  class DataIterator : public Iterator
  {
    typedef Iterator superclass;
  protected:
    friend class csIniFile;
    char* Section;
    DataIterator(const csIniFile&, const PrvINIbranch*, const char* Section);
    void Clone(const DataIterator&);
  public:
    /// Copy constructor
    DataIterator(const DataIterator& i) : Iterator(i), Section(NULL)
      { Clone(i); }
    /// Destructor
    ~DataIterator();
    /// Assignment operator
    DataIterator& operator=(const DataIterator& i) { Clone(i); return *this; }
    /// Returns section name over which this object iterates data
    const char* GetSection() const { return Section; }
    /// Returns name of item
    const char* GetName() const { return Node->Data.Name; }
    /// Returns value of item
    csSome GetData() const { return Node->Data.Pointer; }
    /// Returns size of value of item
    size_t GetDataSize() const { return Node->Data.Size; }
  };
  friend class csIniFile::DataIterator;

  /// Returns a data iterator; sets Found to true if SectionPath is found
  DataIterator EnumData (const char* SectionPath, bool& Found) const;
  /// Returns a data iterator
  DataIterator EnumData (const char* SectionPath) const
    { bool b; return EnumData (SectionPath, b); }
  /// Enumerate data entries and put their names into a string vector
  bool EnumData (const char *SectionPath, csStrVector *oList) const;

  /// A comment iterator
  class CommentIterator : public Iterator
  {
    typedef Iterator superclass;
  protected:
    friend class csIniFile;
    char* Section;
    char* Key;
    CommentIterator(const csIniFile&, const PrvINIbranch*,
      const char* SectionPath, const char* KeyName);
    void Clone(const CommentIterator&);
  public:
    /// Copy constructor
    CommentIterator(const CommentIterator& i) :
      Iterator(i), Section(NULL), Key(NULL) { Clone(i); }
    /// Assignment operator
    CommentIterator& operator=(const CommentIterator& i)
      { Clone(i); return *this; }
    /// Destructor
    ~CommentIterator();
    /// Returns section name over which this object iterates comments
    const char* GetSection () const { return Section; }
    /// Returns key name (if specified) over which this object iterates
    const char* GetKey () const { return Key; }
    /// Returns text of comment
    const char* GetText () const { return Node->Comment.Text; }
  };
  friend class csIniFile::CommentIterator;

  /// Returns a comment iterator; sets Found to true if comments are found
  CommentIterator EnumComments (const char* SectionPath, const char* KeyName,
    bool& Found) const;
  /// Returns a comment iterator; KeyName may be NULL
  CommentIterator EnumComments (const char* SectionPath, const char* KeyName)
    const { bool b; return EnumComments (SectionPath, KeyName, b); }
  /// Enumerate comments bound to a entry; KeyName may be NULL
  bool EnumComments (const char* SectionPath, const char* KeyName,
    csStrVector *oList) const;

  /// Query if a specific section exists
  bool SectionExists (const char *SectionPath) const;
  /// Query if a specific key exists
  bool KeyExists (const char *SectionPath, const char *KeyName) const;

  /// Get data indexed by section name and key name
  bool GetData (const char *SectionPath, const char *KeyName, csSome &Data,
    size_t &DataSize) const;
  /// Get an integer value from the configuration instance.
  int GetInt (const char *SectionPath, const char *KeyName, int def = 0) const;
  /// Get a real value from the configuration instance.
  float GetFloat (const char *SectionPath, const char *KeyName,
    float def = 0.0) const;
  /// Get a string value from the configuration instance.
  const char *GetStr (const char *SectionPath, const char *KeyName,
    const char *def = "") const;
  /// Get a boolean value from the configuration instance.
  bool GetYesNo (const char *SectionPath, const char *KeyName,
    bool def = false) const;

  /// Set data: if Data is NULL, delete entry
  bool SetData (const char *SectionPath, const char *KeyName, csConstSome Data,
    size_t DataSize);
  /// Set an asciiz value (shortcut for SetData)
  bool SetStr (const char *SectionPath, const char *KeyName, const char *Val);
  /// Set an integer value (shortcut for SetData)
  bool SetInt (const char *SectionPath, const char *KeyName, int Value);
  /// Set an floating-point value (shortcut for SetData)
  bool SetFloat (const char *SectionPath, const char *KeyName, float Value);
  /// Set comment for given section/entry
  bool SetComment (const char *SectionPath, const char *KeyName,
    const char *Text);

  /// Delete an entry
  bool Delete (const char *SectionPath, const char *KeyName);
  /// Delete all comments for a key or section (if KeyName == NULL)
  bool DeleteComment (const char *SectionPath, const char *KeyName);

private:
  /// Find node by given path
  PrvINInode* FindNode (const char *SectionPath, const char* KeyName) const;
  /// Set the data for a given node
  void SetData (PrvINInode*, csConstSome Data, size_t DataSize);
  /// Load a file given a "read from stream" routine
  bool Load (bool (*ReadLine) (csSome Stream, void *data, size_t size),
    csSome Stream);
  /// Save a comment
  void SaveComment (const char* Text, csIniWriteFunc writeFunc, csSome Stream) const;
  /// Save comments for a section or key
  void SaveComments (const PrvINIbranch*, csIniWriteFunc writeFunc, csSome Stream) const;
  /// Save all the keys within a section
  void SaveData (const char* Name, csSome Data, size_t DataSize,
    const PrvINIbranch* comments, csIniWriteFunc writeFunc, csSome Stream) const;
  /// Save all the data in a section
  void SaveSection (const PrvINInode*, csIniWriteFunc writeFunc, csSome Stream) const;
};

#endif // __CS_INIFILE_H__
