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
#include "csutil/util.h"
#include "csutil/ref.h"
#include "csutil/databuf.h"
#include "cstool/initapp.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"
#include "iutil/cfgmgr.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include <ctype.h>

#define VFS_CONFIG_FILE	"vfs.cfg"

CS_IMPLEMENT_APPLICATION

static csRef<iVFS> VFS;
static csRef<iConfigManager> Cfg;
static bool ShutDown = false;

// forward declaration for command handlers
static void cmd_cat (char *args);
static void cmd_chdir (char *args);
static void cmd_conf (char *args);
static void cmd_cp (char *args);
static void cmd_creat (char *args);
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
  { "cat", cmd_cat },
  { "cd", cmd_chdir },
  { "chdir", cmd_chdir },
  { "conf", cmd_conf },
  { "cp", cmd_cp },
  { "creat", cmd_creat },
  { "dir", cmd_ls },
  { "exists", cmd_exists },
  { "exit", cmd_quit },
  { "help", cmd_help },
  { "ls", cmd_ls },
  { "mount", cmd_mount },
  { "pwd", cmd_pwd },
  { "quit", cmd_quit },
  { "rm", cmd_rm },
  { "save", cmd_save },
  { "sync", cmd_sync },
  { "time", cmd_time },
  { "unmount", cmd_unmount },
  { "rpath", cmd_rpath },
  { "mounts", cmd_mounts },
  { "rmounts", cmd_rmounts },
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
    int n = strlen(s);
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
    fprintf (stderr, "%s: arguments required\n", command);
    return false;
  }

  arg1 = args;
  while (*args && !isspace(*args))
    args++;
  if (!*args && req2nd)
  {
nodest:
    fprintf (stderr, "%s: no second argument\n", command);
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
  printf
  (
       "----========************* Virtual Shell commands: *************========----\n"
       "pwd			Print work directory\n"
       "[cd|chdir] {path}	Change directory\n"
       "[ls|dir] {-} {path}	List files; with '-' shows full pathname\n"
       "cat {-} [file]		Display file contents to console; with '-' in one pass\n"
       "cp {-} [src] [dst]	Copy file src to file dst; with '-' in one pass\n"
       "creat [file]		Create a file and copy from stdin to file until EOF\n"
       "rm [file]		Delete a file on VFS\n"
       "exists [file]		Test whenever a file exists on VFS\n"
       "sync			Synchronize virtual file system\n"
       "mount [vpath] [rpath]	Add a virtual path mapped to given real path\n"
       "unmount [vpath] {rpath}	Remove a virtual path; if no rpath is given, completely\n"
       "conf {-} [file]         Parse a VFS config file; with '-' file is on real FS\n"
       "save			Save current virtual file system state to " VFS_CONFIG_FILE "\n"
       "time [file]             Display the file's modification time\n"
       "rpath [file]            Convert the virtual path into `real-world' path\n"
       "mounts                  Display current virtual mount paths\n"
       "rmounts [vpath]         Display real-world paths mounted at virtual mount\n"
       "exit			Exit Virtual Shell\n"
       "------------------------\n"
       "Wildcards are okay in these commands: ls, cp, rm\n"
  );
}

static void cmd_pwd (char *)
{
  printf ("%s\n", VFS->GetCwd ());
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
      fprintf (stderr, "cat: cannot read file \"%s\"\n", args);
      return;
    }

    fwrite (**data, data->GetSize (), 1, stdout);
  }
  else
  {
    csRef<iFile> F (VFS->Open (args, VFS_FILE_READ));
    if (!F)
    {
      fprintf (stderr, "cat: cannot open file \"%s\" for reading\n", args);
      return;
    }

    while (!F->AtEOF ())
    {
      char buff [16];
      size_t len = F->Read (buff, sizeof (buff) - 1);
      buff [len] = 0;
      printf ("%s", buff);
    }
  }
}

static void cmd_creat (char *args)
{
  csRef<iFile> F (VFS->Open (args, VFS_FILE_WRITE));
  if (!F)
  {
    fprintf (stderr, "creat: cannot create or open for writing file \"%s\"\n", args);
    return;
  }

  printf ("Copying from stdin to file \"%s\", enter EOF to finish\n", args);
  for (;;)
  {
    char buff [160];
    if (!fgets (buff, sizeof (buff), stdin))
      break;
    size_t len = F->Write (buff, strlen (buff));
    if (len < strlen (buff))
    {
      fprintf (stderr, "creat: error writing to file \"%s\"\n", args);
      break;
    }
  }
  printf ("done, closing file\n");
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
        printf ("[%02d:%02d:%02d %02d-%02d-%04d]%9lu %s\n",
	  ft.hour, ft.min, ft.sec,
          ft.day, ft.mon + 1, ft.year, (unsigned long)fs, fname);
      }
      else
      {
        int dirlen = strlen (fname);
        if (dirlen)
          dirlen--;
        while (dirlen && fname [dirlen - 1] != VFS_PATH_SEPARATOR)
          dirlen--;
        printf ("%-19s", fname + dirlen);
        nl = true;
        if ((i & 3) == 3)
        {
          printf ("\n");
          nl = false;
        }
      }
    }
    if (nl)
      printf ("\n");
  }
  else
    printf ("ls: no files to display\n");
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
      int dirlen = strlen (src);
      if (dirlen)
        dirlen--;
      while (dirlen && src [dirlen - 1] != VFS_PATH_SEPARATOR)
        dirlen--;
      strcpy (destname, dst);
      if (destname [0])
        if (destname [strlen (destname) - 1] != VFS_PATH_SEPARATOR)
          strcat (destname, "/");
      strcat (destname, src + dirlen);
      printf ("%s -> %s\n", src, destname);
      dst = destname;
    }

    if (onepass)
    {
      csRef<iDataBuffer> data (VFS->ReadFile (src));
      if (!data)
      {
        fprintf (stderr, "cp: cannot read file \"%s\"\n", src);
        return;
      }

      if (!VFS->WriteFile (dst, **data, data->GetSize ()))
        fprintf (stderr, "cp: error writing to file \"%s\"\n", dst);
    }
    else
    {
      csRef<iFile> dF (VFS->Open (dst, VFS_FILE_WRITE));
      if (!dF)
      {
        fprintf (stderr, "cp: cannot open destination file \"%s\"\n", dst);
        return;
      }
      csRef<iFile> sF (VFS->Open (src, VFS_FILE_READ));
      if (!sF)
      {
        fprintf (stderr, "cp: cannot open source file \"%s\"\n", src);
        return;
      }
      while (!sF->AtEOF ())
      {
        char buff [123];
        size_t len = sF->Read (buff, sizeof (buff));
        if (dF->Write (buff, len) != len)
        {
          fprintf (stderr, "cp: error writing to file \"%s\"\n", dst);
          break;
        }
      }
    }
  }
}

static void cmd_rm (char *args)
{
  if (!args)
    fprintf (stderr, "rm: empty argument\n");
  else if (!VFS->DeleteFile (args))
    fprintf (stderr, "rm: cannot remove file \"%s\"\n", args);
}

static void cmd_save (char *)
{
  if (!VFS->SaveMounts (VFS_CONFIG_FILE))
    fprintf (stderr, "save: cannot save VFS configuration file\n");
}

static void cmd_mount (char *args)
{
  char *vpath, *rpath;
  if (!get2args ("mount", args, vpath, rpath))
    return;

  if (!VFS->Mount (vpath, rpath))
    fprintf (stderr, "mount: cannot mount \"%s\" to \"%s\"\n", rpath, vpath);
}

static void cmd_unmount (char *args)
{
  char *vpath, *rpath;
  if (!get2args ("unmount", args, vpath, rpath, false))
    return;

  if (!*rpath)
    rpath = 0;

  if (!VFS->Unmount (vpath, rpath))
    fprintf (stderr, "unmount: cannot unmount \"%s\" from \"%s\"\n", rpath, vpath);
}

static void cmd_conf (char *args)
{
  bool real_fs;
  get_option (args, real_fs);
  iVFS *CfgVFS;
  if (real_fs) CfgVFS = 0; else CfgVFS = VFS;

  iConfigFile *config = Cfg->AddDomain (args, CfgVFS, iConfigManager::ConfigPriorityCmdLine);

  if (!config)
  {
    fprintf (stderr, "conf: cannot load config file \"%s\" in %s\n", args, real_fs ? "real filesystem" : "VFS");
    return;
  }

  if (!VFS->LoadMountsFromFile (config))
    fprintf (stderr, "conf: mount: cannot mount all directories found in "
	     "config file.\n");

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
    fprintf (stderr, "exists: empty argument\n");
    return;
  }

  bool IsDir = args [strlen (args) - 1] == '/';
  printf ("%s \"%s\" %s\n", IsDir ? "Directory" : "File", args,
    VFS->Exists (args) ? "exists" : "does not exist");
}

static void cmd_time (char *args)
{
  if (!args)
  {
    fprintf (stderr, "time: expected filename\n");
    return;
  }

  csFileTime flmt;
  if (!VFS->GetFileTime (args, flmt))
  {
    fprintf (stderr, "time: cannot query file time (no such file maybe)\n");
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
  printf ("Last file modification time: %s", asctime (&time));
}

static void cmd_rpath (char *args)
{
  if (!args)
  {
    fprintf (stderr, "rpath: expected filename\n");
    return;
  }

  csRef<iDataBuffer> db (VFS->GetRealPath (args));
  if (!db)
  {
    fprintf (stderr, "rpath: no real-world path corresponding to `%s'\n", args);
    return;
  }

  puts ((char *)db->GetData ());
}

static void cmd_mounts (char *args)
{
  csStringArray mounts = VFS->GetMounts ();
  if (mounts.Length ())
  {
    bool nl = false;
    for (size_t i=0; i<mounts.Length (); i++)
    {
      printf ("%-19s", mounts[i]);
      nl = true;
      if ((i & 3) == 3)
      {
        printf ("\n");
        nl = false;
      }
    }

  }
  else
    printf ("mounts: No current mounts to display!\n");
}

static void cmd_rmounts (char *args)
{
  if (!args)
  {
    fprintf (stderr, "rmounts: expected virtual mount path\n");
    return;
  }

  csStringArray rpaths = VFS->GetRealMountPaths (args);
  if (rpaths.Length ())
  {
    bool nl = false;
    for (size_t i=0; i<rpaths.Length (); i++)
    {
      printf ("%s\n", rpaths[i]);
    }
  }
  else
    printf ("rmounts: No virtual mount at path `%s'\n", args);
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
     fprintf (stderr, "couldn't setup config!\n");
     return 1;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_END))
    return -1;

  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    fprintf (stderr, "Cannot load iVFS plugin\n");
    return -1;
  }

  Cfg = CS_QUERY_REGISTRY (object_reg, iConfigManager);
  if (!Cfg)
  {
    fprintf (stderr, "Cannot load iConfigManager plugin\n");
    return -1;
  }

  VFS->MountRoot ("native");

  printf ("Welcome to Virtual Shell\n"
          "Type \"help\" to get a short description of commands\n"
          "\n");

  while (!ShutDown)
  {
    char command [999];
    printf ("%s# ", VFS->GetCwd ());
    fflush (stdout);
    if (!fgets (command, sizeof(command), stdin))
    {
      printf ("\r\n");
      ShutDown = true;
    }
    else
    {
      char* s = command;
      trimwhite(s);
      if (s != 0 && !execute (s))
        fprintf (stderr, "vsh: unknown command: [%s]\n", s);
    }
  }

  Cfg = 0;
  VFS = 0;
  csInitializer::DestroyApplication (object_reg);

  return 0;
}
