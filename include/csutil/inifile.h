/*
    Crystal Space .INI file management
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __INIFILE_H__
#define __INIFILE_H__

#include "csutil/csvector.h"
#include "csutil/strlist.h"

class csVFS;

class csIniFile : public csBase
{
private:
  /// Private class
  class PrvINIbranch : public csVector
  {
  public:
    virtual ~PrvINIbranch ();
    virtual bool FreeItem (csSome Item);
  };

  // Private structure
  struct PrvINInode
  {
    char Type;                            // Section type
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

  /// The root of INI file
  PrvINIbranch Root;
  /// One of '#' or ';' (used in Save ())
  char CommentChar;
  /// Is object content syncronised to disk?
  bool Dirty;

public:
  /// Initialize INI file object
  csIniFile (char iCommentChar = ';');
  /// Initialize INI file object and load it from a file
  csIniFile (const char *fName, char iCommentChar = ';');
  /// Initialize INI file object and load it from a file on VFS volume
  csIniFile (csVFS *vfs, const char *fName, char iCommentChar = ';');
  /// Destroy the object
  virtual ~csIniFile ();

  /// Load INI from file
  bool Load (const char *fName);
  /// Load INI from memory buffer
  bool Load (const char *Data, size_t DataSize);
  /// Override to type your own error messages
  virtual bool Error (int LineNo, const char *Line, int Pos);

  /// Save INI file
  bool Save (const char *fName);

  /// Enumerate sections in INI file: call iterator for each section
  bool EnumSections (const char *SectionPath, bool (*iterator)
    (csSome Parm, char *Name), csSome Parm) const;

  /// Enumerate sections, but doesn't use crappy iterator
  bool EnumSections(csSTRList*); 

  /// Enumerate data entries: call iterator for each data entry
  bool EnumData (const char *SectionPath, bool (*iterator)
    (csSome Parm, char *Name, size_t DataSize, csSome Data), csSome Parm) const;
  /// Enumerate comments bound to a entry; KeyName can be NULL
  bool EnumComments (const char *SectionPath, const char *KeyName,
    bool (*iterator) (csSome Parm, char *Text), csSome Parm) const;

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
  float GetFloat (const char *SectionPath, const char *KeyName, float def = 0.0) const;
  /// Get a string value from the configuration instance.
  char *GetStr (const char *SectionPath, const char *KeyName, char *def = "") const;
  /// Get a boolean value from the configuration instance.
  bool GetYesNo (const char *SectionPath, const char *KeyName, bool def = false) const;

  /// Set data: if Data is NULL, delete entry
  bool SetData (const char *SectionPath, const char *KeyName, csConstSome Data,
    size_t DataSize);
  /// Set an asciiz value (shortcut for SetData)
  bool SetStr (const char *SectionPath, const char *KeyName, const char *Value);
  /// Set an integer value (shortcut for SetData)
  bool SetInt (const char *SectionPath, const char *KeyName, int Value);
  /// Set an floating-point value (shortcut for SetData)
  bool SetFloat (const char *SectionPath, const char *KeyName, float Value);
  /// Set comment for given section/entry
  bool SetComment (const char *SectionPath, const char *KeyName, const char *Text);

  /// Delete an entry
  bool Delete (const char *SectionPath, const char *KeyName);

private:
  /// Load a file given a "read from stream" routine
  bool Load (bool (*ReadLine) (csSome Stream, void *data, size_t size), csSome Stream);
  /// Find node by given path
  PrvINIbranch *FindNode (const char *SectionPath) const;
  /// Iterator used in Save ()
  static bool SaveEnumComments (csSome struc, char *Text);
  /// Iterator used in Save ()
  static bool SaveEnumData (csSome struc, char *Name, size_t DataSize, csSome Data);
  /// Iterator used in Save ()
  static bool SaveEnumSec (csSome struc, char *Name);
};

#endif // __INIFILE_H__
