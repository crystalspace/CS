#ifndef __IVARIA_AWS_H__
#define __IVARIA_AWS_H__

#include "csutil/scf.h"
#include "csgeom/csrect.h"
#include "iutil/string.h"

struct iAws;
struct iAwsPrefs;
            

SCF_VERSION (iAws, 0, 0, 1);

struct iAws : public iBase
{
public:  
  /// Get a pointer to the preference manager
  virtual iAwsPrefs *GetPrefMgr()=0;

  /// Set the preference manager used by the window system
  virtual void       SetPrefMgr(iAwsPrefs *pmgr)=0;
};


SCF_VERSION (iAwsPrefs, 0, 0, 1);

struct iAwsPrefs : public iBase
{
public:
  /// Invokes the definition parser to load definition files
  virtual void Load(const char *def_file)=0;

  /// Maps a name to an id
  virtual unsigned long NameToId(char *name)=0;
    
  /// Select which skin is the default for components, the skin must be loaded.  True on success, false otherwise.
  virtual bool SelectDefaultSkin(char *skin_name)=0;

  /// Lookup the value of an int key by name (from the skin def)
  virtual bool LookupIntKey(char *name, int &val)=0; 

  /// Lookup the value of an int key by id (from the skin def)
  virtual bool LookupIntKey(unsigned long id, int &val)=0; 

  /// Lookup the value of a string key by name (from the skin def)
  virtual bool LookupStringKey(char *name, iString *&val)=0; 

  /// Lookup the value of a string key by id (from the skin def)
  virtual bool LookupStringKey(unsigned long id, iString *&val)=0; 

  /// Lookup the value of a rect key by name (from the skin def)
  virtual bool LookupRectKey(char *name, csRect &rect)=0; 

  /// Lookup the value of a rect key by id (from the skin def)
  virtual bool LookupRectKey(unsigned long id, csRect &rect)=0; 
};

#endif // __IVARIA_AWS_H__
