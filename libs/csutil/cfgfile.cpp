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

#include "cssysdef.h"
#include "csutil/cfgfile.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/platformfile.h"
#include "csutil/physfile.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfstringarray.h"
#include "csutil/snprintf.h"
#include "csutil/stringconv.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include "iutil/cmdline.h"
#include "iutil/vfs.h"
#include <ctype.h>
#include <errno.h>

/* config node */

class csConfigNode : public CS::Memory::CustomAllocated
{
public:
  // create a new config node. Set name to 0 to create the initial node.
  csConfigNode(const char *Name);
  // delete this node
  ~csConfigNode();
  // delete all nodes will non-0 name
  void DeleteDataNodes();
  // insert this node after the given node
  void InsertAfter(csConfigNode *Where);
  // remove this node from its list
  void Remove();
  // get the name of this node
  const char *GetName() const;
  // set the data for this key
  void SetStr(const char*);
  void SetInt(int);
  void SetFloat(float);
  void SetBool(bool);
  void SetTuple (iStringArray* Value);
  void SetComment(const char*);
  // get data
  const char *GetStr() const;
  int GetInt() const;
  float GetFloat() const;
  bool GetBool() const;
  csPtr<iStringArray> GetTuple() const;
  const char *GetComment() const;
  // return prev and next node
  csConfigNode *GetPrev() const;
  csConfigNode *GetNext() const;

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

csConfigNode::csConfigNode(const char *Keyname)
{
  Prev = Next = 0;
  Name = CS::StrDup (Keyname);
  Data = Comment = 0;
}

csConfigNode::~csConfigNode()
{
  Remove();
  cs_free (Name);
  cs_free (Data);
  cs_free (Comment);
}

const char *csConfigNode::GetName() const
{
  return Name;
}

csConfigNode *csConfigNode::GetPrev() const
{
  return Prev;
}

csConfigNode *csConfigNode::GetNext() const
{
  return Next;
}

void csConfigNode::DeleteDataNodes()
{
  if (Next) Next->DeleteDataNodes();
  if (Name) delete this;
}

void csConfigNode::InsertAfter(csConfigNode *Where)
{
  if (!Where) return;
  Next = Where->Next;
  Prev = Where;
  Where->Next = this;
  if (Next) Next->Prev = this;
}

void csConfigNode::Remove()
{
  if (Next) Next->Prev = Prev;
  if (Prev) Prev->Next = Next;
  Prev = Next = 0;
}

void csConfigNode::SetStr(const char *s)
{
  cs_free (Data);
  Data = CS::StrDup (s);
}

void csConfigNode::SetInt(int n)
{
  csString output;
  output.Format ("%d", n);
  SetStr (output);
}

void csConfigNode::SetFloat(float f)
{
  char output [64];
  cs_snprintf (output, sizeof(output), "%g", f);
  SetStr (output);
}

void csConfigNode::SetBool(bool b)
{
  SetStr(b ? "true" : "false");
}

void csConfigNode::SetTuple (iStringArray* Value)
{
  // this should output a string like
  // abc, def, ghi
  csString s;
  while (!Value->IsEmpty ())
  {
    csString i = Value->Pop ();
    if (!Value->IsEmpty ())
      i.Append (", ");
    s.Append(i);
  }
  SetStr (s);
}

void csConfigNode::SetComment(const char *s)
{
  cs_free (Comment);
  Comment = CS::StrDup (s);
}

const char *csConfigNode::GetStr() const
{
  return (Data ? Data : "");
}

int csConfigNode::GetInt() const
{
  return (Data ? atoi(Data) : 0);
}

float csConfigNode::GetFloat() const
{
  return (Data ? (float) CS::Utility::strtof(Data) : 0.0f);
}

bool csConfigNode::GetBool() const
{
  return (Data &&
    (strcasecmp(Data, "true") == 0 ||
     strcasecmp(Data, "yes" ) == 0 ||
     strcasecmp(Data, "on"  ) == 0 ||
     strcasecmp(Data, "1"   ) == 0));
}

csPtr<iStringArray> csConfigNode::GetTuple() const
{
  if (!Data)
    return 0;

  scfStringArray *items = new scfStringArray;		// the output list
  csString item;

  char *sinp = Data;
  char *comp;
  size_t len;
  bool finished = false;

  while (!finished)
  {
    comp = strchr (sinp, ',');
    if (!comp)
    {
      finished = true;
      comp = &sinp [strlen (sinp)];
    }
    len = strlen (sinp) - strlen (comp);
    item = csString (sinp, len);
    item.Trim ();
    sinp = comp + 1;
    items->Push (item);
  }

  csPtr<iStringArray> v(items);
  return v;
}

const char *csConfigNode::GetComment() const
{
  return Comment;
}

/* config iterator */

class csConfigIterator : public scfImplementation1<csConfigIterator,
                                                   iConfigIterator>
{
public:

  /// Returns the configuration object for this iterator.
  virtual iConfigFile *GetConfigFile() const;
  /// Returns the subsection in the configuruation.
  virtual const char *GetSubsection() const;

  /// Rewind the iterator (points to nowhere after this)
  virtual void Rewind ();
  /// Move to the next valid key. Returns false if no more keys exist.
  virtual bool Next();

  /**
   * Get the current key name.  Set Local to true to return only the local name
   * inside the iterated subsection.  This is the portion of the key string
   * which follows the subsection prefix which was used to create this
   * iterator.
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
  /// Get a tuple set from the configuration.
  virtual csPtr<iStringArray> GetTuple() const;
  /// Get the comment of the given key, or 0 if no comment exists.
  virtual const char *GetComment() const;

  // Create a new iterator.
  csConfigIterator(csConfigFile *Config, const char *Subsection);
  // Delete this iterator
  virtual ~csConfigIterator();
private:
  friend class csConfigFile;
  csRef<csConfigFile> Config;
  csConfigNode* Node;
  csConfigNode* nextNode;
  csString Subsection;

  // Utility function to check if a key meets subsection requirement
  bool CheckSubsection(const char *Key) const;
  // Move to the previous node, ignoring subsection
  bool DoPrev();
  // Move to previous item and return true if the position is valid
  bool Prev ();
  // Is there another valid key?
  bool HasNext();
};

csConfigIterator::csConfigIterator(csConfigFile *c, const char *sub) :
  scfImplementationType (this), Config (c), nextNode (Config->FirstNode), 
  Subsection (sub)
{
  Next();
}

csConfigIterator::~csConfigIterator()
{
  Config->RemoveIterator(this);
}

iConfigFile *csConfigIterator::GetConfigFile() const
{
  return (iConfigFile*)Config;
}

const char *csConfigIterator::GetSubsection() const
{
  return Subsection;
}

void csConfigIterator::Rewind ()
{
  nextNode = Config->FirstNode;
  Next();
}

bool csConfigIterator::DoPrev()
{
  if (!Node->GetPrev()) return false;
  Node = Node->GetPrev();
  return (Node->GetName() != 0);
}

bool csConfigIterator::CheckSubsection(const char *Key) const
{
  return (Subsection.IsEmpty() ||
    strncasecmp(Key, Subsection, Subsection.Length()) == 0);
}

bool csConfigIterator::Prev ()
{
  if (!Subsection) return DoPrev();

  while (1)
  {
    if (!DoPrev()) return false;
	if (CheckSubsection(Node->GetName())) return true;
  }
}

bool csConfigIterator::Next()
{
  Node = nextNode;

  if (Subsection.IsEmpty()) 
  {
    nextNode = Node->GetNext();
    return Node != 0;
  }

  while (1)
  {
    nextNode = nextNode->GetNext();
    if (!nextNode || !nextNode->GetName()) break;
    if (CheckSubsection(nextNode->GetName())) break;
  }
  return (Node != 0) && (Node->GetName() != 0);
}

bool csConfigIterator::HasNext()
{
  return (nextNode != 0) && (nextNode->GetName() != 0);
}

const char *csConfigIterator::GetKey(bool Local) const
{
  return Node->GetName() + (Local ? Subsection.Length() : 0);
}

int csConfigIterator::GetInt() const
{
  return Node->GetInt();
}

float csConfigIterator::GetFloat() const
{
  return Node->GetFloat();
}

const char *csConfigIterator::GetStr() const
{
  return Node->GetStr();
}

bool csConfigIterator::GetBool() const
{
  return Node->GetBool();
}

csPtr<iStringArray> csConfigIterator::GetTuple() const
{
  return Node->GetTuple();
}

const char *csConfigIterator::GetComment() const
{
  return Node->GetComment();
}

/* configuation object */

void csConfigFile::InitializeObject ()
{
  FirstNode = new csConfigNode(0);
  LastNode = new csConfigNode(0);
  LastNode->InsertAfter(FirstNode);

  Iterators = new csArray<csConfigIterator*>();
  Filename = 0;
  Dirty = false;
  EOFComment = 0;
}

csConfigFile::csConfigFile(iBase *pBase)
  : scfImplementationType (this, pBase)
{
  InitializeObject ();
}

csConfigFile::csConfigFile(const char *file, iVFS *vfs)
  : scfImplementationType (this)
{
  InitializeObject ();
  if (file)
    Load(file, vfs);
}

csConfigFile::~csConfigFile()
{
  Clear();
  delete FirstNode;
  delete LastNode;
  // every iterator holds a reference to this object, so when this is
  // deleted there shouldn't be any iterators left.
  CS_ASSERT(Iterators->GetSize () == 0);
  delete Iterators;
  cs_free (Filename);
}

bool csConfigFile::IsEmpty() const
{
  csConfigNode* const n = FirstNode->GetNext();
  return n == 0 || n->GetName() == 0;
}

const char* csConfigFile::GetFileName() const
{
  return Filename;
}

iVFS* csConfigFile::GetVFS() const
{
  return VFS;
}

void csConfigFile::SetFileName(const char *fName, iVFS *vfs)
{
  cs_free (Filename);

  Filename = CS::StrDup (fName);
  VFS = vfs;
  Dirty = true;
}

bool csConfigFile::Load(const char* fName, iVFS *vfs, bool Merge, bool NewWins)
{
  // We want these changes even if the new config file does not exist:
  if (!Merge)
  {
    // Replace current configuration.  Clear all options, change file name and
    // set the dirty flag.  In case the file cannot be opened our configuration
    // IS dirty.  If we can open it successfully, the flag will be cleared
    // later.
    Clear();
    SetFileName(fName, vfs);
    Dirty = true;
  } // else: Only set dirty flag if we really insert something.

  if (!LoadNow(fName, vfs, NewWins))
    return false;

  if (!Merge)
  {
    // the file has successfully replaced the old configuration. Now the
    // configuration is in sync with the file, so clear the dirty flag.
    Dirty = false;
  }
  // else: LoadNow has already set the dirty flag if any options
  // have been inserted.
  return true;
}

bool csConfigFile::Save()
{
  if (!Dirty)
    return true;

  if (!SaveNow(Filename, VFS))
    return false;

  Dirty = false;
  return true;
}

bool csConfigFile::Save(const char *file, iVFS *vfs)
{
  // detect bad parameters
  if (!file) return false;

  // look if we are trying to save to 'our' file. This is only a rough
  // check and will not detect if these are the same files in all cases.
  if (Filename && strcmp(Filename, file)==0 && VFS==vfs)
  {
    if (!Dirty) return true;
  }

  if (!SaveNow(file, vfs))
    return false;

  Dirty = false;
  return true;
}

void WriteComment(csString &Filedata, const char *s)
{
  if (s != 0)
  {
    for (const char* b = strchr(s,'\n'); b != 0; b = strchr(s,'\n'))
    {
      if (*s != '\n' && *s != ';') // Prepend comment character if absent.
        Filedata << "; ";
      Filedata.Append(s, b - s + 1);
      s = b + 1;
    }
    if (*s)
    {
      if (*s != ';')
        Filedata << "; ";
      Filedata << s;
    }
    if (!Filedata.IsEmpty() && Filedata.GetAt(Filedata.Length()-1) != '\n')
      Filedata << '\n';
  }
}

bool csConfigFile::SaveNow(const char *file, iVFS *vfs) const
{
  // @@@ handle different newline codes on different systems
  // @@@ handle base64-encoded data (?)
  csString Filedata;
  csConfigNode *n;

  for (n = FirstNode; n != 0; n = n->GetNext())
  {
    // don't write first and last nodes
    if (n->GetName() == 0) continue;

    // write comment
    WriteComment(Filedata, n->GetComment());

    // write key line
    Filedata << n->GetName() << " = " << n->GetStr() << '\n';
  }
  // write end-of-file comment
  WriteComment(Filedata, EOFComment);

  const size_t length = Filedata.Length ();

  if (vfs)
  {
    return vfs->WriteFile(file, Filedata.GetData(), length);
  }
  else
  {
    FILE *fp = CS::Platform::File::Open (file, "wb");
    if (!fp) return false;
    const size_t res = fwrite(Filedata.GetData(), sizeof(char), length, fp);
    const int errcode = errno;
    fclose(fp);
    if (res != length)
    {
      csPrintfErr (
        "csConfigFile::SaveNow(): fwrite() error for %s (errno = %d)!\n",
          file, errcode);
      return false;
    }
    return true;
  }
}

void csConfigFile::Clear()
{
  // delete all nodes but the first and last one
  FirstNode->DeleteDataNodes();
  // rewind all iterators
  for (size_t i = 0; i < Iterators->GetSize (); i++)
  {
    csConfigIterator *it = Iterators->Get(i);
    it->Rewind();
  }
  if (EOFComment)
  {
    cs_free (EOFComment);
    EOFComment = 0;
  }
  Dirty = true;
}

csPtr<iConfigIterator> csConfigFile::Enumerate(const char *Subsection)
{
  csConfigIterator *it = new csConfigIterator(this, Subsection);
  Iterators->Push(it);
  return csPtr<iConfigIterator> (it);
}

bool csConfigFile::KeyExists(const char *Key) const
{
  return (FindNode(Key) != 0);
}

bool csConfigFile::SubsectionExists(const char *Subsection) const
{
  return (FindNode(Subsection, true) != 0);
}

int csConfigFile::GetInt(const char *Key, int Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetInt() : Def;
}

float csConfigFile::GetFloat(const char *Key, float Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetFloat() : Def;
}

const char *csConfigFile::GetStr(const char *Key, const char *Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetStr() : Def;
}

bool csConfigFile::GetBool(const char *Key, bool Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetBool() : Def;
}

csPtr<iStringArray> csConfigFile::GetTuple(const char *Key) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetTuple() : 0;
}

const char *csConfigFile::GetComment(const char *Key) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetComment() : 0;
}

void csConfigFile::SetStr (const char *Key, const char *Val)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) Node = CreateNode(Key);
  if (Node)
  {
    const char* s = Node->GetStr();
    if ((s && !Val) || (!s && Val) || (Val && strcmp(s,Val) != 0))
    { // The new value differs from the old.
      Node->SetStr(Val);
      Dirty = true;
    }
  }
}

void csConfigFile::SetInt (const char *Key, int Value)
{
  csConfigNode *Node = FindNode(Key);
  bool const Create = !Node;
  if (Create) Node = CreateNode(Key);
  if (Node && (Create || Value != Node->GetInt()))
  {
    Node->SetInt(Value);
    Dirty = true;
  }
}

void csConfigFile::SetFloat (const char *Key, float Value)
{
  csConfigNode *Node = FindNode(Key);
  bool const Create = !Node;
  if (Create) Node = CreateNode(Key);
  if (Node && (Create || Value != Node->GetFloat()))
  {
    Node->SetFloat(Value);
    Dirty = true;
  }
}

void csConfigFile::SetBool (const char *Key, bool Value)
{
  csConfigNode *Node = FindNode(Key);
  bool const Create = !Node;
  if (Create) Node = CreateNode(Key);
  if (Node && (Create || Value != Node->GetBool()))
  {
    Node->SetBool(Value);
    Dirty = true;
  }
}

void csConfigFile::SetTuple (const char *Key, iStringArray* Value)
{
  csConfigNode *Node = FindNode(Key);
  bool const Create = !Node;
  if (Create) Node = CreateNode(Key);

  bool changed = false;
  // This checks to see whether the thing
  // we're saving is the same as the saved
  if (Node)
  {
    csRef<iStringArray> sa = Node->GetTuple ();
    // was the tuple valid ?
    if (sa)
    {
      // its different if lengths differ
      if (sa->GetSize () != Value->GetSize ())
      {
        changed = true;
      }
      else
      {
        for (uint i = 0 ; i < sa->GetSize (); i++)
        {
          // found 2 different strings in tuple
          if (sa->Get (i) != Value->Get (i))
          {
            changed = true;
            break;
          }
        }
      }
    }
    else
      changed = true;
  }

  if (Node && (Create || changed))
  {
    Node->SetTuple(Value);
    Dirty = true;
  }
}

bool csConfigFile::SetComment (const char *Key, const char *Text)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) return false;
  const char* s = Node->GetComment();
  if ((s && !Text) || (!s && Text) || (Text && strcmp(s,Text) != 0))
  { // The new comment differs from the old.
    Node->SetComment(Text);
    Dirty = true;
  }
  return true;
}

void csConfigFile::DeleteKey(const char *Name)
{
  csConfigNode *Node = FindNode(Name);
  if (!Node) return;

  // look for iterators on that node
  for (size_t i = 0; i < Iterators->GetSize (); i++)
  {
    csConfigIterator *it = (csConfigIterator*)Iterators->Get(i);
    if (it->Node == Node) it->Prev();
  }

  // remove and delete the node
  Node->Remove();
  delete Node;
  Dirty = true;
}

bool csConfigFile::LoadNow(const char *fName, iVFS *vfs, bool overwrite)
{
  // load the file buffer
  csRef<iDataBuffer> Filedata;
  if (vfs)
  {
    Filedata = vfs->ReadFile(fName);
  }
  else
  {
    csRef<iFile> file;
    file.AttachNew (new csPhysicalFile (fName, "rb"));
    Filedata = file->GetAllData (true);
  }
  if (!Filedata) return false;

  // parse the data
  LoadFromBuffer ((char*)Filedata->GetInt8(), overwrite);

  return true;
}

void csConfigFile::LoadFromBuffer(const char *Filedata, bool overwrite)
{
  csString CurrentComment, currentLineBuf, key, value;
  const char* s = 0;
  int SkipCount = 0;
  bool LastLine = false;
  int  Line;
  for (Line = 1; !LastLine; Line++, Filedata = s + SkipCount)
  {
    s = Filedata + strcspn (Filedata, "\n\r");
    LastLine = (*s == 0);
    if (!LastLine)
    {
      // Advance past LF, CR, or CRLF.
      SkipCount = (*s == '\r' && *(s+1) == '\n' ? 2 : 1);
      // If the next advance will get to the end of file this is the 
      // last line we are parsing
      if(*(s+SkipCount) == 0) LastLine = true;
    }

    currentLineBuf.Replace (Filedata, s - Filedata);
    currentLineBuf.Trim ();

    // check if this is a comment or a blank line
    if (currentLineBuf.IsEmpty() || currentLineBuf.GetAt (0) == ';')
      CurrentComment << currentLineBuf << '\n';
    else
    {
      // this is a key. Find equal sign
      size_t eqPos = currentLineBuf.FindFirst ('=');
      // if no equal sign, this is an invalid line
      if (eqPos == (size_t)-1)
      {
        csFPrintf(stderr, "Missing `=' on line %d of %s\n", Line,
	  (Filename ? Filename : "configuration data"));
        CurrentComment.Clear();
        continue;
      }
      // check for missing key name
      if (eqPos == 0)
      {
        csFPrintf(stderr, "Missing key name (before `=') on line %d of %s\n",
	  Line, (Filename ? Filename : "configuration data"));
        CurrentComment.Clear();
        continue;
      }
      // delete whitespace before equal sign
      key.Replace (currentLineBuf, eqPos);
      key.RTrim();
      // check if node already exists and create node
      csConfigNode *Node = FindNode (key);
      if (Node && !overwrite)
      {
        CurrentComment.Clear();
        continue;
      }
      if (!Node) Node = CreateNode (key);
      // skip whitespace after equal sign
      value.Replace (currentLineBuf.GetData() + eqPos + 1);
      value.LTrim();
      // extract key value
      Node->SetStr (value);
      // apply comment
      if (!CurrentComment.IsEmpty())
      {
        Node->SetComment(CurrentComment);
        CurrentComment.Clear();
      }
      // set dirty flag
      Dirty = true;
    }
  }
  if (!CurrentComment.IsEmpty())
    SetEOFComment(CurrentComment);
}

csConfigNode *csConfigFile::FindNode(const char *Name, bool isSubsection) const
{
  if (!Name) return 0;

  csConfigNode *n = FirstNode;
  const size_t sz = (isSubsection ? strlen(Name) : 0);
  while (n)
  {
    const char* s = n->GetName();
    if (s && ((isSubsection && strncasecmp(s, Name, sz) == 0) ||
       (strcasecmp(s, Name) == 0)))
      return n;
    n = n->GetNext();
  }
  return 0;
}

csConfigNode *csConfigFile::CreateNode(const char *Name)
{
  if (!Name) return 0;

  csConfigNode *n = new csConfigNode(Name);
  n->InsertAfter(LastNode->GetPrev());
  Dirty = true;
  return n;
}

void csConfigFile::RemoveIterator(csConfigIterator *it) const
{
  size_t n = Iterators->Find(it);
  CS_ASSERT(n != csArrayItemNotFound);
  Iterators->DeleteIndex(n);
}

void csConfigFile::SetEOFComment(const char *text)
{
  cs_free (EOFComment);
  EOFComment = (text ? CS::StrDup(text) : 0);
  Dirty = true;
}

const char *csConfigFile::GetEOFComment() const
{
  return EOFComment;
}

void csConfigFile::ParseCommandLine (iCommandLineParser* cmdline, iVFS* vfs,
                                     bool Merge, bool NewWins)
{
  // We want these changes even if the new config file does not exist:
  if (!Merge)
  {
    Clear();
    SetFileName ("<command line>", 0);
    Dirty = true;
  }

  csString buffer;
  size_t index = 0, cfgsetNum = 0, cfgfileNum = 0;
  const char* option;
  while ((option = cmdline->GetOptionName (index)) != 0)
  {
    index++;
    if (strcmp (option, "cfgset") == 0)
    {
      const char* setting = cmdline->GetOption ("cfgset", cfgsetNum++);
      buffer << setting << '\n';
    }
    else if (strcmp (option, "cfgfile") == 0)
    {
      const char* fName = cmdline->GetOption ("cfgfile", cfgfileNum++);
      csRef<iDataBuffer> Filedata;
      if (vfs)
      {
        Filedata = vfs->ReadFile(fName);
      }
      else
      {
        csRef<iFile> file;
        file.AttachNew (new csPhysicalFile (fName, "rb"));
        Filedata = file->GetAllData (true);
      }
      if (Filedata.IsValid())
      {
        buffer.Append (Filedata->GetData(), Filedata->GetSize());
        buffer << '\n';
      }
    }
  }

  if (!buffer.IsEmpty ())
    LoadFromBuffer (buffer, NewWins);

  if (!Merge)
  {
    // the file has successfully replaced the old configuration. Now the
    // configuration is in sync with the file, so clear the dirty flag.
    Dirty = false;
  }
}
