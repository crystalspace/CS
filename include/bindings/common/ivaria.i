%include "ivaria/decal.h"
/*%ignore iReporter::ReportV;
%ignore csReporterHelper::ReportV;
%include "ivaria/reporter.h"*/

%ignore iConsoleOutput::PutTextV;
%ignore iConsoleOutput::PerformExtensionV;
%include "ivaria/conout.h"
%include "ivaria/conin.h"

%include "ivaria/stdrep.h"
%include "ivaria/view.h"
%include "ivaria/bugplug.h"
%include "ivaria/collider.h"
ARRAY_CHANGE_ALL_TEMPLATE(csCollisionPair)
%include "ivaria/dynamics.h"
%include "ivaria/ode.h"
%include "ivaria/engseq.h"
%include "ivaria/movierecorder.h"
%include "ivaria/mapnode.h"
%include "ivaria/sequence.h"

%ignore *::Call (const char *name, const char *format, ...);
%ignore *::Call (const char *name, int &ret, const char *fmt, ...);
%ignore *::Call (const char *name, float &ret, const char *fmt, ...);
%ignore *::Call (const char *name, double &ret, const char *fmt, ...);
%ignore *::Call (const char *name, csRef<iString>&, const char *fmt, ...);
%ignore *::Call (const char *name, csRef<iScriptObject> &ret, const char *fmt, ...);
%ignore iScript::Store (const char *name, int data);
%ignore iScript::Store (const char *name, float data);
%ignore iScript::Store (const char *name, double data);
%ignore iScript::Store (const char *name, char const *data);
%ignore iScript::Store (const char *name, iScriptObject *data);
%ignore iScript::SetTruth (const char *name, bool isTrue);
%ignore iScript::Retrieve (const char *name, int &data);
%ignore iScript::Retrieve (const char *name, float &data);
%ignore iScript::Retrieve (const char *name, double &data);
%ignore iScript::Retrieve (const char *name, csRef<iString>&);
%ignore iScript::Retrieve (const char *name, csRef<iScriptObject>&);
%ignore iScript::GetTruth (const char *name, bool &isTrue);
%include "ivaria/script.h"
%include "ivaria/simpleformer.h"
%include "ivaria/terraform.h"
%include "ivaria/translator.h"


// ivaria/collider.h
%extend iCollideSystem
{
  csCollisionPair * GetCollisionPairByIndex (int index)
  { return self->GetCollisionPairs() + index; }
}

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST IVARIA_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(ivariapost.i)

