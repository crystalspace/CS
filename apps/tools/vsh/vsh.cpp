/*
    Virtual Shell: A test program for Virtual File System library
    Copyright (C) 1999 Andrew Zabolotny <bit@eltech.ru>

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
#include <ctype.h>
#include <errno.h>

#include "cstool/initapp.h"
#include "csutil/ansicommand.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/ref.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include "iutil/cfgmgr.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#define VFS_CONFIG_FILE	"vfs.cfg"

CS_IMPLEMENT_APPLICATION

static csRef<iVFS> VFS;
static csRef<iConfigManager> Cfg;
static bool ShutDown = false;

// forward declaration for command handlers
static void cmd_cat (wchar_t *args);
static void cmd_chdir (wchar_t *args);
static void cmd_config (wchar_t *args);
static void cmd_cp (wchar_t *args);
static void cmd_create (wchar_t *args);
static void cmd_exists (wchar_t *args);
static void cmd_help (wchar_t *args);
static void cmd_ls (wchar_t *args);
static void cmd_mount (wchar_t *args);
static void cmd_pwd (wchar_t *args);
static void cmd_quit (wchar_t *args);
static void cmd_rm (wchar_t *args);
static void cmd_save (wchar_t *args);
static void cmd_sync (wchar_t *args);
static void cmd_time (wchar_t *args);
static void cmd_unmount (wchar_t *args);
static void cmd_rpath (wchar_t *args);
static void cmd_mounts (wchar_t *args);
static void cmd_rmounts (wchar_t *args);

struct VshCmdList
{
  const wchar_t *command;
  void (*handler) (wchar_t *args);
};

VshCmdList const cmdlist [] =
{
  { L"?",       cmd_help    },
  { L"cat",     cmd_cat     },
  { L"cd",      cmd_chdir   },
  { L"chdir",   cmd_chdir   },
  { L"config",  cmd_config  },
  { L"copy",    cmd_cp      },
  { L"cp",      cmd_cp      },
  { L"create",  cmd_create  },
  { L"del",     cmd_rm      },
  { L"dir",     cmd_ls      },
  { L"exists",  cmd_exists  },
  { L"exit",    cmd_quit    },
  { L"help",    cmd_help    },
  { L"ls",      cmd_ls      },
  { L"mount",   cmd_mount   },
  { L"mounts",  cmd_mounts  },
  { L"pwd",     cmd_pwd     },
  { L"quit",    cmd_quit    },
  { L"rm",      cmd_rm      },
  { L"rmounts", cmd_rmounts },
  { L"rpath",   cmd_rpath   },
  { L"save",    cmd_save    },
  { L"sync",    cmd_sync    },
  { L"time",    cmd_time    },
  { L"type",    cmd_cat     },
  { L"unmount", cmd_unmount },
  { 0, 0 }
};

static void skipspc (wchar_t *&s)
{
  while (*s && iswspace(*s))
    s++;
  if (!*s)
    s = 0;
}

static void trimwhite (wchar_t*& s)
{
  skipspc(s);
  if (s != 0)
  {
    size_t n = wcslen(s);
    while (n-- > 0)
      if (iswspace (s[n]))
        s[n] = '\0';
      else
        break;
  }
}

static bool get2args (const wchar_t *command, wchar_t *args,
                      wchar_t *&arg1, wchar_t *&arg2, bool req2nd = true)
{
  if (!args)
  {
    csPrintfErr ("%ls: arguments required\n", command);
    return false;
  }

  arg1 = args;
  while (*args && !iswspace(*args))
    args++;
  if (!*args && req2nd)
  {
nodest:
    csPrintfErr ("%ls: no second argument\n", command);
    return false;
  }
  arg2 = args;
  if (*args)
    arg2++;
  *args = 0;
  while (iswspace(*arg2))
    arg2++;
  if (!*arg2 && req2nd)
    goto nodest;

  return true;
}

static void get_option (wchar_t *&args, bool &opt)
{
  opt = false;
  if (args && *args == '-')
  {
    opt = true;
    args++;
    skipspc (args);
  }
}

static void cmd_help (wchar_t *)
{
  csPrintf (
"----========************* Virtual Shell commands: *************========----\n"
"cat {-} file           Display file contents to console; with %s in one pass\n"
"cd {path}              Change directory to path; or to root if path not given\n"
"config {-} file        Parse a VFS config file; with %s file is on real FS\n"
"cp {-} src dst         Copy file src to file dst; with %s in one pass\n"
"create file            Create a file and copy from stdin to file until EOF\n"
"exists file            Test if file exists on VFS\n"
"exit                   Exit Virtual Shell\n"
"ls {-} {path}          List files; with %s shows full pathname\n"
"mount vpath rpath      Add a virtual path mapped to given real path\n"
"mounts                 Display all virtual mounts\n"
"pwd                    Print working directory\n"
"rm file                Delete file on VFS\n"
"rmounts vpath          Display real paths mounted at virtual path\n"
"rpath file             Convert the virtual path into real path\n"
"save                   Save current virtual file system state to " VFS_CONFIG_FILE "\n"
"sync                   Synchronize virtual file system (flush pending writes)\n"
"time file              Display file's modification time\n"
"unmount vpath {rpath}  Remove a virtual path; completely if no rpath is given\n"
"------------------------\n"
"The following aliases are also recognized:\n"
"chdir --> cd        copy --> cp         del  --> rm\n"
"dir   --> ls        quit --> exit       type --> cat\n"
"------------------------\n"
"Wildcards are okay in these commands: ls, cp, rm\n",
CS::Quote::Single ("-"),
CS::Quote::Single ("-"),
CS::Quote::Single ("-"),
CS::Quote::Single ("-")
  );
}

static void cmd_pwd (wchar_t *)
{
  csPrintf ("%s\n", VFS->GetCwd ());
}

static void cmd_chdir (wchar_t *args)
{
  VFS->ChDir (args ? csString (args) : "/");
}

static void cmd_cat (wchar_t *args_w)
{
  bool onepass;
  get_option (args_w, onepass);

    csString args (args_w);
  if (onepass)
  {
    csRef<iDataBuffer> data (VFS->ReadFile (args));
    if (!data)
    {
      csPrintfErr ("cat: cannot read file %s\n", CS::Quote::Double (args));
      return;
    }

    const size_t size = data->GetSize ();
    const size_t res = fwrite (**data, size, 1, stdout);
    if (res != size)
      csPrintfErr ("cat: could only write %zu of %zu bytes (errno = %d)!\n",
        res, size, errno);
  }
  else
  {
    csRef<iFile> F (VFS->Open (args, VFS_FILE_READ));
    if (!F)
    {
      csPrintfErr ("cat: cannot open file %s for reading\n", CS::Quote::Double (args));
      return;
    }

    while (!F->AtEOF ())
    {
      char buff [16];
      size_t len = F->Read (buff, sizeof (buff) - 1);
      buff [len] = 0;
      csPrintf ("%s", buff);
    }
  }
}

static void cmd_create (wchar_t *args_w)
{
  csString args (args_w);
  csRef<iFile> F (VFS->Open (args, VFS_FILE_WRITE));
  if (!F)
  {
    csPrintfErr ("create: cannot create or open for writing file %s\n",
      CS::Quote::Double (args));
    return;
  }

  csPrintf ("Copying from stdin to file %s, enter EOF to finish\n", CS::Quote::Double (args));
  for (;;)
  {
    wchar_t buff [160];
    if (!fgetws (buff, sizeof (buff)/sizeof(buff[0]), stdin))
      break;
	csString str_utf8 (buff);
	size_t len = F->Write (str_utf8.GetData(), str_utf8.Length());
	if (len < str_utf8.Length())
    {
      csPrintfErr ("create: error writing to file %s\n", CS::Quote::Double (args));
      break;
    }
  }
  csPrintf ("done, closing file\n");
}

static void cmd_ls (wchar_t *args)
{
  bool fullpath;
  get_option (args, fullpath);

  const char *dir;
  csRef<iDataBuffer> xpath;
  if (args)
  {
    xpath = VFS->ExpandPath (csString (args));
    dir = **xpath;
  }
  else
    dir = VFS->GetCwd ();

  csRef<iStringArray> fl (VFS->FindFiles (dir));
  if (fl->GetSize () > 0)
  {
    bool nl = false;
	
    size_t i;
    for (i = 0; i < fl->GetSize () ; i++)
    {
      const char *fname = fl->Get (i);
      if (fullpath)
      {
        csFileTime ft;
        if (!VFS->GetFileTime (fname, ft))
          memset (&ft, 0, sizeof (ft));
        size_t fs;
        if (!VFS->GetFileSize (fname, fs))
          fs = 0;
        csPrintf ("[%02d:%02d:%02d %02d-%02d-%04d]%9zu %s\n",
	  ft.hour, ft.min, ft.sec,
          ft.day, ft.mon + 1, ft.year, fs, fname);
      }
      else
      {
        size_t dirlen = strlen (fname);
        if (dirlen)
          dirlen--;
        while (dirlen && fname [dirlen - 1] != VFS_PATH_SEPARATOR)
          dirlen--;
        csPrintf ("%-19s", fname + dirlen);
        nl = true;
        if ((i & 3) == 3)
        {
          csPrintf ("\n");
          nl = false;
        }
      }
    }
    if (nl)
      csPrintf ("\n");
  }
  else
    csPrintf ("ls: no files to display\n");
}

static void cmd_cp (wchar_t *args)
{
  bool onepass;
  get_option (args, onepass);

  wchar_t *src_w, *dst_w;
  if (!get2args (L"cp", args, src_w, dst_w))
    return;

  csRef<iStringArray> fl (VFS->FindFiles (csString (src_w)));
  size_t i;
  csString dst (dst_w);
  for (i = 0; i < fl->GetSize () ; i++)
  {
    csString destname;
    const char* src = fl->Get (i);

    if (fl->GetSize () > 1)
    {
      size_t dirlen = strlen (src);
      if (dirlen)
        dirlen--;
      while (dirlen && src [dirlen - 1] != VFS_PATH_SEPARATOR)
        dirlen--;
      destname = dst;
      if (destname.IsEmpty())
        if (destname [destname.Length() - 1] != VFS_PATH_SEPARATOR)
          destname.Append ("/");
      destname.Append (src + dirlen);
      csPrintf ("%s -> %s\n", src, destname.GetData());
      dst = destname;
    }

    if (onepass)
    {
      csRef<iDataBuffer> data (VFS->ReadFile (src));
      if (!data)
      {
        csPrintfErr ("cp: cannot read file %s\n", CS::Quote::Double (src));
        return;
      }

      if (!VFS->WriteFile (dst, **data, data->GetSize ()))
        csPrintfErr ("cp: error writing to file %s\n", CS::Quote::Double (dst));
    }
    else
    {
      csRef<iFile> dF (VFS->Open (dst, VFS_FILE_WRITE));
      if (!dF)
      {
        csPrintfErr ("cp: cannot open destination file %s\n", CS::Quote::Double (dst));
        return;
      }
      csRef<iFile> sF (VFS->Open (src, VFS_FILE_READ));
      if (!sF)
      {
        csPrintfErr ("cp: cannot open source file %s\n", CS::Quote::Double (src));
        return;
      }
      while (!sF->AtEOF ())
      {
        char buff [123];
        size_t len = sF->Read (buff, sizeof (buff));
        if (dF->Write (buff, len) != len)
        {
          csPrintfErr ("cp: error writing to file %s\n", CS::Quote::Double (dst));
          break;
        }
      }
    }
  }
}

static void cmd_rm (wchar_t *args_w)
{
  csString args (args_w);
  if (!args.IsEmpty())
    csPrintfErr ("rm: empty argument\n");
  else if (!VFS->DeleteFile (args))
    csPrintfErr ("rm: cannot remove file %s\n", CS::Quote::Double (args));
}

static void cmd_save (wchar_t *)
{
  if (!VFS->SaveMounts (VFS_CONFIG_FILE))
    csPrintfErr ("save: cannot save VFS configuration file\n");
}

static void cmd_mount (wchar_t *args)
{
  wchar_t *vpath_w, *rpath_w;
  if (!get2args (L"mount", args, vpath_w, rpath_w))
    return;

  csString vpath (vpath_w);
  csString rpath (rpath_w);
  if (!VFS->Mount (vpath, rpath))
    csPrintfErr ("mount: cannot mount %s to %s\n", CS::Quote::Double (rpath), CS::Quote::Double (vpath));
}

static void cmd_unmount (wchar_t *args)
{
  wchar_t *vpath_w, *rpath_w;
  if (!get2args (L"unmount", args, vpath_w, rpath_w, false))
    return;

  if (!*rpath_w)
    rpath_w = 0;

  csString vpath (vpath_w);
  csString rpath (rpath_w);
  if (!VFS->Unmount (vpath, rpath))
    csPrintfErr ("unmount: cannot unmount %s from %s\n",
      CS::Quote::Double (rpath), CS::Quote::Double (vpath));
}

static void cmd_config (wchar_t *args_w)
{
  bool real_fs;
  get_option (args_w, real_fs);
  iVFS *CfgVFS = real_fs ? (iVFS*)0 : (iVFS*)VFS;

  csString args (args_w);
  iConfigFile *config =
    Cfg->AddDomain (args, CfgVFS, iConfigManager::ConfigPriorityCmdLine);

  if (!config)
  {
    csPrintfErr ("config: cannot load config file %s in %s\n",
      CS::Quote::Double (args), real_fs ? "real filesystem" : "VFS");
    return;
  }

  if (!VFS->LoadMountsFromFile (config))
    csPrintfErr (
      "config: mount: cannot mount all directories found in config file.\n");
}

static void cmd_sync (wchar_t *)
{
  VFS->Sync ();
}

static void cmd_quit (wchar_t *)
{
  ShutDown = true;
}

static void cmd_exists (wchar_t *args_w)
{
  if (!args_w)
  {
    csPrintfErr ("exists: empty argument\n");
    return;
  }

  csString args (args_w);
  bool IsDir = args [args.Length() - 1] == '/';
  csPrintf ("%s %s %s\n", IsDir ? "Directory" : "File", CS::Quote::Double (args),
    VFS->Exists (args) ? "exists" : "does not exist");
}

static void cmd_time (wchar_t *args_w)
{
  if (!args_w)
  {
    csPrintfErr ("time: expected filename\n");
    return;
  }

  csString args (args_w);
  csFileTime flmt;
  if (!VFS->GetFileTime (args, flmt))
  {
    csPrintfErr ("time: can not query file time (no such file maybe)\n");
    return;
  }

  struct tm time;
  memset (&time, 0, sizeof (time));
  time.tm_sec = flmt.sec;
  time.tm_min = flmt.min;
  time.tm_hour = flmt.hour;
  time.tm_mday = flmt.day;
  time.tm_mon = flmt.mon;
  time.tm_year = flmt.year - 1900;

  // No newline needed; asctime() adds it for us.
  csPrintf ("Last file modification time: %s", asctime (&time));
}

static void cmd_rpath (wchar_t *args_w)
{
  if (!args_w)
  {
    csPrintfErr ("rpath: expected filename\n");
    return;
  }

  csString args (args_w);
  csRef<iDataBuffer> db (VFS->GetRealPath (args));
  if (!db)
  {
    csPrintfErr ("rpath: no real-world path corresponding to %s\n", CS::Quote::Single (args));
    return;
  }

  csPrintf ("%s\n", (char *)db->GetData ());
}

static void cmd_mounts (wchar_t* /*args*/)
{
  csRef<iStringArray> mounts = VFS->GetMounts ();
  if (mounts->GetSize ())
  {
    bool nl = false;
    for (size_t i=0; i<mounts->GetSize () ; i++)
    {
      csPrintf ("%-19s", mounts->Get (i));
      nl = true;
      if ((i & 3) == 3)
      {
        csPrintf ("\n");
        nl = false;
      }
    }
    if (nl)
      csPrintf ("\n");
  }
  else
    csPrintf ("mounts: no current mounts to display!\n");
}

static void cmd_rmounts (wchar_t *args_w)
{
  if (!args_w)
  {
    csPrintfErr ("rmounts: expected virtual mount path\n");
    return;
  }

  csString args (args_w);
  csRef<iStringArray> rpaths = VFS->GetRealMountPaths (args);
  if (rpaths->GetSize ())
  {
    for (size_t i=0; i<rpaths->GetSize () ; i++)
    {
      csPrintf ("%s\n", rpaths->Get (i));
    }
  }
  else
    csPrintf ("rmounts: no virtual mount at path %s\n", CS::Quote::Single (args));
}

static bool execute (wchar_t *command)
{
  size_t cp = 0;
  wchar_t *args;

  while (command [cp] && !iswspace (command [cp]))
    cp++;
  args = command + cp;
  skipspc (args);
  command [cp] = 0;

  int i;
  for (i = 0; cmdlist [i].command; i++)
    if (wcscmp (cmdlist [i].command, command) == 0)
    {
      cmdlist [i].handler (args);
      return true;
    }
  return false;
}

int main (int argc, char *argv [])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  if (!csInitializer::SetupConfigManager (object_reg, 0))
  {
     csPrintfErr ("couldn't setup config!\n");
     return 1;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_END))
    return -1;

  VFS = csQueryRegistry<iVFS> (object_reg);
  if (!VFS)
  {
    csPrintfErr ("Cannot load iVFS plugin\n");
    return -1;
  }

  Cfg = csQueryRegistry<iConfigManager> (object_reg);
  if (!Cfg)
  {
    csPrintfErr ("Cannot load iConfigManager plugin\n");
    return -1;
  }

  VFS->MountRoot ("native");

  csPrintf ("Welcome to Virtual Shell\n"
          "Type %s to get a short description of commands\n"
          "\n",
	  CS::Quote::Double ("help"));

  while (!ShutDown)
  {
    wchar_t command [999];
    csPrintf (CS_ANSI_TEXT_BOLD_ON CS_ANSI_FM "%s " CS_ANSI_TEXT_BOLD_OFF 
      CS_ANSI_FG "#" CS_ANSI_RST " ", VFS->GetCwd ());
    fflush (stdout);
    if (!fgetws (command, sizeof(command)/sizeof(command[0]), stdin))
    {
      csPrintf ("\r\n");
      ShutDown = true;
    }
    else
    {
      wchar_t* s = command;
      trimwhite(s);
      if (s != 0 && !execute (s))
        csPrintfErr ("vsh: unknown command: [%ls]\n", s);
    }
  }

  Cfg = 0;
  VFS = 0;
  csInitializer::DestroyApplication (object_reg);
  return 0;
}
