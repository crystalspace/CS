#include "csutil/scf.h"
#include "iutil/comp.h"
#include "iutil/document.h"

struct iObjectRegistry;

class csXMLDocumentSystem : public iDocumentSystem, public iComponent
{
public:
  SCF_DECLARE_IBASE;
  
  csXMLDocumentSystem (iBase* parent = NULL);
  virtual ~csXMLDocumentSystem ();

  virtual bool Initialize (iObjectRegistry* objreg);

  csRef<iDocument> CreateDocument ();
};
