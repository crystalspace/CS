#include "cssysdef.h"

#include <sys/stat.h>

#include "csutil/snprintf.h"
#include "csutil/cfgfile.h"
#include "cssys/sysfunc.h"

csPtr<iConfigFile> csGetPlatformConfig(const char* key)
{
  // is $HOME set? otherwise fallback to standard mode
  const char* home = getenv("HOME");
  if (!home)
    return 0;
  
  // construct directory and filename of the config file
  char fname[1000];
  char dir[1000];
  cs_snprintf(dir, 1000, "%s/.crystal", getenv("HOME"));
  cs_snprintf(fname, 1000, "%s/%s.cfg", dir, key);

  // try to create the directory (we assume that $HOME is already created)
  struct stat stats;
  if (stat(dir, &stats) != 0)
  {
    if (mkdir(dir, S_IWUSR | S_IXUSR | S_IRUSR) != 0)
    {
      fprintf(stderr,
  	  "couldn't create directory '%s' for configuration files.\n", dir);
      return 0;
    }
  }

  // create/read a config file
  csRef<csConfigFile> configfile = csPtr<csConfigFile> (new csConfigFile);
  if (!configfile->Load(fname)) {
    fprintf(stderr, "couldn't create configfile '%s'.\n", fname);
    return 0;
  }
    
  return csPtr<iConfigFile> (configfile);
}

