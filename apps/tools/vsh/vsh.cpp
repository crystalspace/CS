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
#include "cstool/initapp.h"
#include "csutil/ansicolor.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/ref.h"
#include "csutil/util.h"
#include "csutil/sysfunc.h"
#include "iutil/cfgmgr.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"

#define VFS_CONFIG_FILE	"vfs.cfg"

CS_IMPLEMENT_APPLICATION

static csRef<iVFS> VFS;
static csRef<iConfigManager> Cfg;
static bool ShutDown = false;

// forward declaration for command handlers
static void cmd_cat (char *args);
static void cmd_chdir (char *args);
static void cmd_config (char *args);
static void cmd_cp (char *args);
static void cmd_create (char *args);
static void cmd_exists (char *args);
static void cmd_help (char *args);
static void cmd_ls (char *args);
static void cmd_mount (char *args);
static void cmd_pwd (char *args);
static void cmd_quit (char *args);
static void cmd_rm (char *args);
static void cmd_save (char *args);
static void cmd_sync (char *args);
static void cmd_time (char *args);
static void cmd_unmount (char *args);
static void cmd_rpath (char *args);
static void cmd_mounts (char *args);
static void cmd_rmounts (char *args);

struct
{
  const char *command;
  void (*handler) (char *args);
} cmdlist [] =
{
  { "?",       cmd_help    },
  { "cat",     cmd_cat     },
  { "cd",      cmd_chdir   },
  { "chdir",   cmd_chdir   },
  { "config",  cmd_config  },
  { "copy",    cmd_cp      },
  { "cp",      cmd_cp      },
  { "create",  cmd_create  },
  { "del",     cmd_rm      },
  { "dir",     cmd_ls      },
  { "exists",  cmd_exists  },
  { "exit",    cmd_quit    },
  { "help",    cmd_help    },
  { "ls",      cmd_ls      },
  { "mount",   cmd_mount   },
  { "mounts",  cmd_mounts  },
  { "pwd",     cmd_pwd     },
  { "quit",    cmd_quit    },
  { "rm",      cmd_rm      },
  { "rmounts", cmd_rmounts },
  { "rpath",   cmd_rpath   },
  { "save",    cmd_save    },
  { "sync",    cmd_sync    },
  { "time",    cmd_time    },
  { "type",    cmd_cat     },
  { "unmount", cmd_unmount },
  { 0, 0 }
};

static void skipspc (char *&s)
{
  while (*s && isspace(*s))
    s++;
  if (!*s)
    s = 0;
}

static void trimwhite (char*& s)
{
  skipspc(s);
  if (s != 0)
  {
    size_t n = strlen(s);
    while (n-- > 0)
      if (isspace (s[n]))
        s[n] = '\0';
      else
        break;
  }
}

static bool get2args (char *command, char *args, char *&arg1, char *&arg2,
  bool req2nd = true)
{
  if (!args)
  {
    csPrintfErr ("%s: arguments required\n", command);
    return false;
  }

  arg1 = args;
  while (*args && !isspace(*args))
    args++;
  if (!*args && req2nd)
  {
nodest:
    csPrintfErr ("%s: no second argument\n", command);
    return false;
  }
  arg2 = args;
  if (*args)
    arg2++;
  *args = 0;
  while (isspace(*arg2))
    arg2++;
  if (!*arg2 && req2nd)
    goto nodest;

  return true;
}

static void get_option (char *&args, bool &opt)
{
  opt = false;
  if (args && *args == '-')
  {
    opt = true;
    args++;
    skipspc (args);
  }
}

static void cmd_help (char *)
{
  csPrintf (
"----========************* Virtual Shell commands: *************========----\n"
"cat {-} file           Display file contents to console; with '-' in one pass\n"
"cd {path}              Change directory to path; or to root if path not given\n"
"config {-} file        Parse a VFS config file; with '-' file is on real FS\n"
"cp {-} src dst         Copy file src to file dst; with '-' in one pass\n"
"create file            Create a file and copy from stdin to file until EOF\n"
"exists file            Test if file exists on VFS\n"
"exit                   Exit Virtual Shell\n"
"ls {-} {path}          List files; with '-' shows full pathname\n"
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
"chdir --> cd        copy --> cp         del --> rm\n"
"dir   --> ls        quit --> exit       type --> cat\n"
"------------------------\n"
"Wildcards are okay in these commands: ls, cp, rm\n"
  );
}

static void cmd_pwd (char *)
{
  csPrintf ("%s\n", VFS->GetCwd ());
}

static void cmd_chdir (char *args)
{
  VFS->ChDir (args ? args : "/");
}

static void cmd_cat (char *args)
{
  bool onepass;
  get_option (args, onepass);

  if (onepass)
  {
    csRef<iDataBuffer> data (VFS->ReadFile (args));
    if (!data)
    {
      csPrintfErr ("cat: cannot read file \"%s\"\n", args);
      return;
    }

    fwrite (**data, data->GetSize (), 1, stdout);
  }
  else
  {
    csRef<iFile> F (VFS->Open (args, VFS_FILE_READ));
    if (!F)
    {
      csPrintfErr ("cat: cannot open file \"%s\" for reading\n", args);
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

static void cmd_create (char *args)
{
  csRef<iFile> F (VFS->Open (args, VFS_FILE_WRITE));
  if (!F)
  {
    csPrintfErr ("create: cannot create or open for writing file \"%s\"\n",
      args);
    return;
  }

  csPrintf ("Copying from stdin to file \"%s\", enter EOF to finish\n", args);
  for (;;)
  {
    char buff [160];
    if (!fgets (buff, sizeof (buff), stdin))
      break;
    size_t len = F->Write (buff, strlen (buff));
    if (len < strlen (buff))
    {
      csPrintfErr ("create: error writing to file \"%s\"\n", args);
      break;
    }
  }
  csPrintf ("done, closing file\n");
}

static void cmd_ls (char *args)
{
  bool fullpath;
  get_option (args, fullpath);

  const char *dir;
  csRef<iDataBuffer> xpath;
  if (args)
  {
    xpath = VFS->ExpandPath (args);
    dir = **xpath;
  }
  else
    dir = VFS->GetCwd ();

  csRef<iStringArray> fl (VFS->FindFiles (dir));
  if (fl->Length() > 0)
  {
    bool nl = false;
	
    size_t i;
    for (i = 0; i < fl->Length (); i++)
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

static void cmd_cp (char *args)
{
  bool onepass;
  get_option (args, onepass);

  char *src, *dst;
  if (!get2args ("cp", args, src, dst))
    return;

  csRef<iStringArray> fl (VFS->FindFiles (src));
  size_t i;
  for (i = 0; i < fl->Length (); i++)
  {
    char destname [VFS_MAX_PATH_LEN + 1];
    src = (char *)fl->Get (i);

    if (fl->Length () > 1)
    {
      size_t dirlen = strlen (src);
      if (dirlen)
        dirlen--;
      while (dirlen && src [dirlen - 1] != VFS_PATH_SEPARATOR)
        dirlen--;
      strcpy (destname, dst);
      if (destname [0])
        if (destname [strlen (destname) - 1] != VFS_PATH_SEPARATOR)
          strcat (destname, "/");
      strcat (destname, src + dirlen);
      csPrintf ("%s -> %s\n", src, destname);
      dst = destname;
    }

    if (onepass)
    {
      csRef<iDataBuffer> data (VFS->ReadFile (src));
      if (!data)
      {
        csPrintfErr ("cp: cannot read file \"%s\"\n", src);
        return;
      }

      if (!VFS->WriteFile (dst, **data, data->GetSize ()))
        csPrintfErr ("cp: error writing to file \"%s\"\n", dst);
    }
    else
    {
      csRef<iFile> dF (VFS->Open (dst, VFS_FILE_WRITE));
      if (!dF)
      {
        csPrintfErr ("cp: cannot open destination file \"%s\"\n", dst);
        return;
      }
      csRef<iFile> sF (VFS->Open (src, VFS_FILE_READ));
      if (!sF)
      {
        csPrintfErr ("cp: cannot open source file \"%s\"\n", src);
        return;
      }
      while (!sF->AtEOF ())
      {
        char buff [123];
        size_t len = sF->Read (buff, sizeof (buff));
        if (dF->Write (buff, len) != len)
        {
          csPrintfErr ("cp: error writing to file \"%s\"\n", dst);
          break;
        }
      }
    }
  }
}

static void cmd_rm (char *args)
{
  if (!args)
    csPrintfErr ("rm: empty argument\n");
  else if (!VFS->DeleteFile (args))
    csPrintfErr ("rm: cannot remove file \"%s\"\n", args);
}

static void cmd_save (char *)
{
  if (!VFS->SaveMounts (VFS_CONFIG_FILE))
    csPrintfErr ("save: cannot save VFS configuration file\n");
}

static void cmd_mount (char *args)
{
  char *vpath, *rpath;
  if (!get2args ("mount", args, vpath, rpath))
    return;

  if (!VFS->Mount (vpath, rpath))
    csPrintfErr ("mount: cannot mount \"%s\" to \"%s\"\n", rpath, vpath);
}

static void cmd_unmount (char *args)
{
  char *vpath, *rpath;
  if (!get2args ("unmount", args, vpath, rpath, false))
    return;

  if (!*rpath)
    rpath = 0;

  if (!VFS->Unmount (vpath, rpath))
    csPrintfErr ("unmount: cannot unmount \"%s\" from \"%s\"\n",
      rpath, vpath);
}

static void cmd_config (char *args)
{
  bool real_fs;
  get_option (args, real_fs);
  iVFS *CfgVFS = real_fs ? 0 : VFS;

  iConfigFile *config =
    Cfg->AddDomain (args, CfgVFS, iConfigManager::ConfigPriorityCmdLine);

  if (!config)
  {
    csPrintfErr ("config: cannot load config file \"%s\" in %s\n",
      args, real_fs ? "real filesystem" : "VFS");
    return;
  }

  if (!VFS->LoadMountsFromFile (config))
    csPrintfErr (
      "config: mount: cannot mount all directories found in config file.\n");
}

static void cmd_sync (char *)
{
  VFS->Sync ();
}

static void cmd_quit (char *)
{
  ShutDown = true;
}

static void cmd_exists (char *args)
{
  if (!args)
  {
    csPrintfErr ("exists: empty argument\n");
    return;
  }

  bool IsDir = args [strlen (args) - 1] == '/';
  csPrintf ("%s \"%s\" %s\n", IsDir ? "Directory" : "File", args,
    VFS->Exists (args) ? "exists" : "does not exist");
}

static void cmd_time (char *args)
{
  if (!args)
  {
    csPrintfErr ("time: expected filename\n");
    return;
  }

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

static void cmd_rpath (char *args)
{
  if (!args)
  {
    csPrintfErr ("rpath: expected filename\n");
    return;
  }

  csRef<iDataBuffer> db (VFS->GetRealPath (args));
  if (!db)
  {
    csPrintfErr ("rpath: no real-world path corresponding to `%s'\n", args);
    return;
  }

  puts ((char *)db->GetData ());
}

static void cmd_mounts (char *args)
{
  csRef<iStringArray> mounts = VFS->GetMounts ();
  if (mounts->Length ())
  {
    bool nl = false;
    for (size_t i=0; i<mounts->Length (); i++)
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

static void cmd_rmounts (char *args)
{
  if (!args)
  {
    csPrintfErr ("rmounts: expected virtual mount path\n");
    return;
  }

  csRef<iStringArray> rpaths = VFS->GetRealMountPaths (args);
  if (rpaths->Length ())
  {
    for (size_t i=0; i<rpaths->Length (); i++)
    {
      csPrintf ("%s\n", rpaths->Get (i));
    }
  }
  else
    csPrintf ("rmounts: no virtual mount at path `%s'\n", args);
}

static bool execute (char *command)
{
  size_t cp = 0;
  char *args;

  while (command [cp] && !isspace (command [cp]))
    cp++;
  args = command + cp;
  skipspc (args);
  command [cp] = 0;

  int i;
  for (i = 0; cmdlist [i].command; i++)
    if (strcmp (cmdlist [i].command, command) == 0)
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

  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    csPrintfErr ("Cannot load iVFS plugin\n");
    return -1;
  }

  Cfg = CS_QUERY_REGISTRY (object_reg, iConfigManager);
  if (!Cfg)
  {
    csPrintfErr ("Cannot load iConfigManager plugin\n");
    return -1;
  }

  VFS->MountRoot ("native");

  csPrintf ("Welcome to Virtual Shell\n"
          "Type \"help\" to get a short description of commands\n"
          "\n");

  while (!ShutDown)
  {
    char command [999];
    csPrintf (CS_ANSI_FI CS_ANSI_FM "%s " CS_ANSI_FI_OFF 
      CS_ANSI_FG "#" CS_ANSI_RST " ", VFS->GetCwd ());
    fflush (stdout);
    if (!fgets (command, sizeof(command), stdin))
    {
      csPrintf ("\r\n");
      ShutDown = true;
    }
    else
    {
      char* s = command;
      trimwhite(s);
      if (s != 0 && !execute (s))
        csPrintfErr ("vsh: unknown command: [%s]\n", s);
    }
  }

  Cfg = 0;
  VFS = 0;
  csInitializer::DestroyApplication (object_reg);
  return 0;
}
