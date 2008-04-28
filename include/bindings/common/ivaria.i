%include "ivaria/decal.h"
/*%ignore iReporter::ReportV;
%ignore csReporterHelper::ReportV;
%include "ivaria/reporter.h"*/

%ignore iConsoleOutput::PutTextV;
%ignore iConsoleOutput::PerformExtensionV;
%include "ivaria/conout.h"

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

%rename(IntCall) *::Call(const char*, int&, const char*, ...);
%rename(FloatCall) *::Call(const char*, float&, const char*, ...);
%rename(DoubleCall) *::Call(const char*, double&, const char*, ...);
%rename(StringCall) *::Call(const char*, char**, const char*, ...);
%rename(ObjectCall) *::Call(const char*,csRef<iScriptObject>&,const char*,...);
%rename(StoreInt) iScript::Store(const char*, int);
%rename(StoreFloat) iScript::Store(const char*, float);
%rename(StoreDouble) iScript::Store(const char*, double);
%rename(StoreString) iScript::Store(const char*, const char*);
%rename(StoreObject) iScript::Store(const char*, iStringObject*);
%rename(RetrieveInt) iScript::Retrieve(const char*, int);
%rename(RetrieveFloat) iScript::Retrieve(const char*, float&);
%rename(RetrieveDouble) iScript::Retrieve(const char*, double&);
%rename(RetrieveString) iScript::Retrieve(const char*, char**);
%rename(RetrieveObject) iScript::Retrieve(const char*, csRef<iStringObject>&);
%rename(SetInt) iScriptObject::Set(const char*, int);
%rename(SetFloat) iScriptObject::Set(const char*, float);
%rename(SetDouble) iScriptObject::Set(const char*, double);
%rename(SetString) iScriptObject::Set(const char*, const char*);
%rename(SetObject) iScriptObject::Set(const char*, iStringObject*);
%rename(GetInt) iScriptObject::Get(const char*, int);
%rename(GetFloat) iScriptObject::Get(const char*, float&);
%rename(GetDouble) iScriptObject::Get(const char*, double&);
%rename(GetString) iScriptObject::Get(const char*, char**);
%rename(GetObject) iScriptObject::Get(const char*, csRef<iStringObject>&);
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

