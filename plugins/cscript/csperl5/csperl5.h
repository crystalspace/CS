
#include "ivaria/script.h"
#include "iutil/comp.h"

#undef MIN
#undef MAX
#include <EXTERN.h>
#include <perl.h>

struct iObjectRegistry;

class csPerl5 : public iScript
{
  private:

  PerlInterpreter *perl;

  public:

  SCF_DECLARE_IBASE;

  csPerl5 (iBase *);
  virtual ~csPerl5 ();

  virtual bool Initialize (iObjectRegistry *);
  virtual bool RunText (const char *);
  virtual bool LoadModule (const char *);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPerl5);
    virtual bool Initialize (iObjectRegistry *);
  } scfiComponent;
  friend struct eiComponent;

  /**
   * Store() is implementation-dependent and meant for internal use
   * tag is casted to (const char *)
   * its value determines what type data is casted to:
   * "i" : (int *)
   * "f" : (float *)
   * "s" : (const char *)
   */
  virtual bool Store (const char *name, void *data, void *tag);
};

