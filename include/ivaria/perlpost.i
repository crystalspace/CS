/*
    Copyright (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifdef SWIGPERL5

/****************************************************************************
 * The AUTOLOAD function is called by Perl if the user tries to call a
 * non-existant object method. We use this to add loads of custom methods
 * to every class without bloating the module. Specifically, we add object
 * property accessor methods.
 ****************************************************************************/
%native(AUTOLOAD) Autoload;
%{
  void Autoload (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    dTARG;
    SV *self = ST (0);
    char *prop = SvPV_nolen (ST (1));
    SV *val = ST (2);

    if (! (SvROK (self) && SvTYPE (self) == SVt_PVHV))
      croak ("No such class method %s in class %s", prop, SvPV_nolen (self));

    HV *obj = (HV *) SvRV (self);
    SV **valp = hv_fetch (obj, prop, 0, val ? 1 : 0);

    if (! valp)
      croak ("No such instance method %s in object", prop);

    if (val) sv_setsv (* valp, val);

    ST (0) = * valp;
    XSRETURN (1);
  }
%}

/****************************************************************************
 * Create an scfInitialize function in Perl which grabs the program path
 * automatically, then remove the C argc/argv version.
 ****************************************************************************/
%inline %{
  void scfInitialize ()
  {
    static char *argv[] = { 0, 0 };
    argv[0] = SvPV_nolen (get_sv ("0", 0));
    scfInitialize (0, argv);
  }
%}

%ignore scfInitialize(int argc, const char * const argv []);

/****************************************************************************
 * This is CS's interface to the Perl script's event handler.
 ****************************************************************************/
%{
  static const int csInitializer_SetupEventHandler_DefaultMask
    = CSMASK_Nothing
    | CSMASK_Broadcast
    | CSMASK_MouseUp
    | CSMASK_MouseDown
    | CSMASK_MouseMove
    | CSMASK_Keyboard
    | CSMASK_MouseClick
    | CSMASK_MouseDoubleClick
    | CSMASK_JoystickMove
    | CSMASK_JoystickDown
    | CSMASK_JoystickUp;

  static SV * pl_csInitializer_EventHandler = 0;

  bool csInitializer_EventHandler (iEvent &event)
  {
    if (! pl_csInitializer_EventHandler) return false;
    SV *event_obj = newSVsv (& PL_sv_undef);
    sv_setref_iv (event_obj, "cspace::iEvent", (int) & event);

    dSP;
    PUSHMARK (SP);
    XPUSHs (event_obj);
    PUTBACK;
    call_sv (pl_csInitializer_EventHandler, G_SCALAR);
    SPAGAIN;
    SV *result = POPs;
    PUTBACK;

    bool ok = SvTRUE (result);
    SvREFCNT_dec (event_obj);
    SvREFCNT_dec (result);
    return ok;
  }
%}

/****************************************************************************
 * This is the Perl script's interface to SetupEventHandler.
 ****************************************************************************/
%native(csInitializer_SetupEventHandler) pl_csInitializer_SetupEventHandler;
%{
  void pl_csInitializer_SetupEventHandler (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    dTARG;
    SV *reg_ref = ST (0);
    SV *func_rv = ST (1);
    SV *mask_sv = ST (2);
    if (! (reg_ref && func_rv))
      croak ("SetupEventHandler needs at least 2 arguments");

    unsigned int mask;
    if (mask_sv)
      mask = SvUV (mask_sv);
    else
      mask = csInitializer_SetupEventHandler_DefaultMask;

    SV *reg_obj = SvRV (reg_ref);
    if (! sv_isa (reg_obj, "cspace::iObjectRegistry"))
      croak ("SetupEventHandler argument 1 must be iObjectRegistry");
    iObjectRegistry *reg = (iObjectRegistry *) SvIV (reg_obj);

    SV *func = SvRV (func_rv);
    pl_csInitializer_EventHandler = func;

    bool ok = csInitializer::SetupEventHandler
      (reg, csInitializer_EventHandler, mask);
    XSRETURN_IV (ok ? 1 : 0);
  }
%}

/****************************************************************************
 * This is a wrapper function for RequestPlugin's variable argument list.
 ****************************************************************************/
%native(csInitializer_RequestPlugins) _csInitializer_RequestPlugins;
%{
  void _csInitializer_RequestPlugins (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    dTARG;
    SV *reg_ref = ST (0);
    SV *reg_obj = SvRV (reg_ref);
    if (! sv_isa (reg_obj, "cspace::iObjectRegistry"))
      croak ("RequestPlugins argument 1 must be iObjectRegistry");

    iObjectRegistry *reg = (iObjectRegistry *) SvIV (reg_obj);
    SvREFCNT_dec (reg_obj);

    bool ok = true;
    for (int arg = 1; ok; arg += 4)
    {
      SV *plug_sv = ST (arg);
      SV *iface_sv = ST (arg + 1);
      SV *scfid_sv = ST (arg + 2);
      SV *ver_sv = ST (arg + 3);

      const char *plug = SvPV_nolen (plug_sv);
      const char *iface = SvPV_nolen (iface_sv);
      int scfid = SvIV (scfid_sv);
      int ver = SvIV (ver_sv);

      bool ok1 = csInitializer::RequestPlugins
        (reg, plug, iface, scfid, ver, 0);
      if (! ok1) ok = false;
    }

    XSRETURN_IV (ok ? 1 : 0);
  }
%}

/****************************************************************************
 * This is a hack to get the iBlah_VERSION constant for any interface, given
 * the "iBlah" is provided as a string.
 * Used by the macro replacements below.
 ****************************************************************************/
%{
  int scfGetVersion (const char *iface)
  {
    char *var = (char *) malloc (strlen (iface) + 9);
    strcpy (var, iface);
    strcat (var, "_VERSION");

    SV *sv = get_sv (var, 0);

    free (var);

    if (sv) return SvIV (sv);
    else return -1;
  }
%}

/****************************************************************************
 * These functions are replacements for CS's macros of the same names.
 * These functions can be wrapped by Swig but the macros can't.
 * They use scfGetVersion defined above.
 ****************************************************************************/
%inline %{
  #undef CS_QUERY_REGISTRY
  #undef CS_QUERY_REGISTRY_TAG_INTERFACE
  #undef SCF_QUERY_INTERFACE
  #undef SCF_QUERY_INTERFACE_SAFE
  #undef CS_QUERY_PLUGIN_CLASS
  #undef CS_LOAD_PLUGIN
  #undef CS_GET_CHILD_OBJECT
  #undef CS_GET_NAMED_CHILD_OBJECT
  #undef CS_GET_FIRST_NAMED_CHILD_OBJECT

  csWrapPtr CS_QUERY_REGISTRY (iObjectRegistry *reg, const char *iface)
  {
    return csWrapPtr (iface, reg->Get
      (iface, iSCF::SCF->GetInterfaceID (iface), scfGetVersion (iface)));
  }
  csWrapPtr CS_QUERY_REGISTRY_TAG_INTERFACE (iObjectRegistry *reg,
    const char *tag, const char *iface)
  {
    return csWrapPtr (iface, reg->Get
      (tag, iSCF::SCF->GetInterfaceID (iface), scfGetVersion (iface)));
  }
  csWrapPtr SCF_QUERY_INTERFACE (iBase *obj, const char *iface)
  {
    return csWrapPtr (iface, (iBase *) obj->QueryInterface
      (iSCF::SCF->GetInterfaceID (iface), scfGetVersion (iface)));
  }
  csWrapPtr SCF_QUERY_INTERFACE_SAFE (iBase *obj, const char *iface)
  {
    return csWrapPtr (iface, (iBase *) iBase::QueryInterfaceSafe
      (obj, iSCF::SCF->GetInterfaceID (iface), scfGetVersion (iface)));
  }
  csWrapPtr CS_QUERY_PLUGIN_CLASS (iPluginManager *obj, const char *id,
    const char *iface)
  {
    return csWrapPtr (iface, obj->QueryPlugin
      (id, iface, scfGetVersion (iface)));
  }
  csWrapPtr CS_LOAD_PLUGIN (iPluginManager *obj, const char *id,
    const char *iface)
  {
    return csWrapPtr (iface, obj->LoadPlugin
      (id, iface, scfGetVersion (iface)));
  }
  csWrapPtr CS_GET_CHILD_OBJECT (iObject *obj, const char *iface)
  {
    return csWrapPtr (iface, (iBase *) obj->GetChild
      (iSCF::SCF->GetInterfaceID (iface), scfGetVersion (iface)));
  }
  csWrapPtr CS_GET_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
    const char *name)
  {
    return csWrapPtr (iface, (iBase *) obj->GetChild
      (iSCF::SCF->GetInterfaceID (iface), scfGetVersion (iface), name));
  }
  csWrapPtr CS_GET_FIRST_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
    const char *name)
  {
    return csWrapPtr (iface, (iBase *) obj->GetChild
      (iSCF::SCF->GetInterfaceID (iface), scfGetVersion (iface), name, true));
  }
%}

/****************************************************************************
 * Helper function for the macros defined below.
 ****************************************************************************/
%{
  void csRequestPlugin (char *iface, int &idnum, int &ver)
  {
    idnum = iSCF::SCF->GetInterfaceID (iface);
    ver = scfGetVersion (iface);
  }
%}

/****************************************************************************
 * Custom INPUT/OUTPUT/INOUT typemaps like those imported from typemaps.i,
 * needed since typemaps.i doesn't define any for strings.
 ****************************************************************************/
%typemap(in) char *& INPUT (char * tmp)
{
  tmp = (char *) SvPV_nolen ($input);
  $1 = & tmp;
}
%typemap(in, numinputs = 0) char *& OUTPUT (char * tmp)
{
  $1 = & tmp;
}
%typemap(argout) char *& OUTPUT
{
  if (argvi >= items) { EXTEND (sp, 1); }
  $result = sv_newmortal ();
  sv_setpv ($result, * ($1));
  argvi++;
}
%typemap(in) char *& INOUT = char *& INPUT;
%typemap(argout) char *& INOUT = char *& OUTPUT;
%typemap(typecheck) char *& INPUT = char *;
%typemap(typecheck) char *& OUTPUT = char *;
%typemap(typecheck) char *& INOUT = char *;

/****************************************************************************
 * Apply typemaps to arguments of macro replacement functions below.
 ****************************************************************************/
%apply char *& OUTPUT { char *& __scfid__ };
%apply char *& OUTPUT { char *& __iface__ };
%apply char *& INOUT { char *& __Iscfid__ };
%apply char *& INOUT { char *& __Iiface__ };
%apply int * OUTPUT { int & __idnum__ };
%apply int * OUTPUT { int & __ver__ };

/****************************************************************************
 * These functions are replacements for CS's macros of the same names.
 * These functions can be wrapped by Swig but the macros can't.
 * They use csRequestPlugin defined above, and the typemaps.
 ****************************************************************************/
%inline %{
  #undef CS_REQUEST_PLUGIN
  #undef CS_REQUEST_VFS
  #undef CS_REQUEST_FONTSERVER
  #undef CS_REQUEST_IMAGELOADER
  #undef CS_REQUEST_NULL3D
  #undef CS_REQUEST_SOFTWARE3D
  #undef CS_REQUEST_OPENGL3D
  #undef CS_REQUEST_ENGINE
  #undef CS_REQUEST_LEVELLOADER
  #undef CS_REQUEST_LEVELSAVER
  #undef CS_REQUEST_REPORTER
  #undef CS_REQUEST_REPORTERLISTENER
  #undef CS_REQUEST_CONSOLEOUT

  void CS_REQUEST_PLUGIN
    (char *& __Iscfid__, char *& __Iiface__, int & __idnum__, int & __ver__)
  {
    __idnum__ = iSCF::SCF->GetInterfaceID (__Iiface__);
    __ver__ = scfGetVersion (__Iiface__);
    csRequestPlugin (__Iiface__, __idnum__, __ver__);
  }
  void CS_REQUEST_VFS
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.kernel.vfs";
    __iface__ = "iVFS";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_FONTSERVER
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.font.server.default";
    __iface__ = "iFontServer";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_IMAGELOADER
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.graphic.image.io.multiplex";
    __iface__ = "iImageIO";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_NULL3D
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.graphics3d.null";
    __iface__ = "iGraphics3D";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_SOFTWARE3D
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.graphics3d.software";
    __iface__ = "iGraphics3D";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_OPENGL3D
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.graphics3d.opengl";
    __iface__ = "iGraphics3D";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_ENGINE
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.engine.3d";
    __iface__ = "iEngine";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_LEVELLOADER
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.level.loader";
    __iface__ = "iLoader";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_LEVELSAVER
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.level.saver";
    __iface__ = "iSaver";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_REPORTER
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.utilities.reporter";
    __iface__ = "iReporter";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_REPORTERLISTENER
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.utilities.stdrep";
    __iface__ = "iStandardReporterListener";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
  void CS_REQUEST_CONSOLEOUT
    (char *& __scfid__, char *& __iface__, int & __idnum__, int & __ver__)
  {
    __scfid__ = "crystalspace.console.output.simple";
    __iface__ = "iConsoleOutput";
    csRequestPlugin (__iface__, __idnum__, __ver__);
  }
%}

/****************************************************************************
 * Wrapper function to get the array returned by GetCollisionPairs().
 ****************************************************************************/
%native(iCollideSystem_GetCollisionPairs) _GetCollisionPairs;
%{
  void _GetCollisionPairs (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    dTARG;
    SV *sys_ref = ST (0);
    if (! sys_ref)
      croak("No self parameter passed to GetCollisionPairs");

    SV *sys_obj = SvRV (sys_ref);
    if (! sv_isa (sys_obj, "cspace::iCollideSystem"))
      croak("Self parameter of GetCollisionPairs must be an iCollideSystem");

    iCollideSystem *sys = (iCollideSystem *) SvIV (sys_obj);

    csCollisionPair *pairs = sys->GetCollisionPairs ();
    int num = sys->GetCollisionPairCount ();
    AV *av = newAV ();
    for (int i = 0; i < num; i++)
    {
      SV *rv = newSViv (0);
      sv_setref_iv (rv, "cspace::csCollisionPair", (int) pairs++);
      av_push (av, rv);
      SvREFCNT_dec (rv);
    }

    SV *rv = newRV ((SV *) av);
    SvREFCNT_dec ((SV *) av);
    ST (0) = rv;
    XSRETURN (1);
  }
%}

/****************************************************************************
 * Typemaps used by the csVector2/3 wrappings below.
 ****************************************************************************/
%apply float * OUTPUT { float & __v1__ };
%apply float * OUTPUT { float & __v2__ };
%apply float * OUTPUT { float & __v3__ };

/****************************************************************************
 * Some extra operator overloads for csVector2.
 ****************************************************************************/
%extend csVector2
{
  float __abs__ ()
  {
    return self->Norm ();
  }
  void __av__ (float &__v1__, float &__v2__)
  {
    __v1__ = self->x;
    __v2__ = self->y;
  }
}

/****************************************************************************
 * Some extra operator overloads for csVector3.
 ****************************************************************************/
%extend csVector3
{
  float __abs__ ()
  {
    return self->Norm ();
  }
  void __av__ (float &__v1__, float &__v2__, float &__v3__)
  {
    __v1__ = self->x;
    __v2__ = self->y;
    __v3__ = self->z;
  }
  bool __bool__ ()
  {
    return ! self->IsZero ();
  }
}

/****************************************************************************
 * These functions are replacements for CS's macros of the same names.
 * These functions can be wrapped by Swig, but the macros can't.
 ****************************************************************************/
%inline %{
  #undef CS_VEC_FORWARD
  #undef CS_VEC_BACKWARD
  #undef CS_VEC_RIGHT
  #undef CS_VEC_LEFT
  #undef CS_VEC_UP
  #undef CS_VEC_DOWN
  #undef CS_VEC_ROT_RIGHT
  #undef CS_VEC_ROT_LEFT
  #undef CS_VEC_TILT_RIGHT
  #undef CS_VEC_TILT_LEFT
  #undef CS_VEC_TILT_UP
  #undef CS_VEC_TILT_DOWN

  const csVector3& CS_VEC_FORWARD()
  {
    static const csVector3 v ( 0,  0,  1);
    return v;
  }
  const csVector3& CS_VEC_BACKWARD()
  {
    static const csVector3 v ( 0,  0, -1);
    return v;
  }
  const csVector3& CS_VEC_RIGHT()
  {
    static const csVector3 v ( 1,  0,  0);
    return v;
  }
  const csVector3& CS_VEC_LEFT()
  {
    static const csVector3 v (-1,  0,  0);
    return v;
  }
  const csVector3& CS_VEC_UP()
  {
    static const csVector3 v ( 0,  1,  0);
    return v;
  }
  const csVector3& CS_VEC_DOWN()
  {
    static const csVector3 v ( 0, -1,  0);
    return v;
  }
  const csVector3& CS_VEC_ROT_RIGHT()
  {
    static const csVector3 v ( 0,  1,  0);
    return v;
  }
  const csVector3& CS_VEC_ROT_LEFT()
  {
    static const csVector3 v ( 0, -1,  0);
    return v;
  }
  const csVector3& CS_VEC_TILT_RIGHT()
  {
    static const csVector3 v ( 0,  0, -1);
    return v;
  }
  const csVector3& CS_VEC_TILT_LEFT()
  {
    static const csVector3 v ( 0,  0,  1);
    return v;
  }
  const csVector3& CS_VEC_TILT_UP()
  {
    static const csVector3 v (-1,  0,  0);
    return v;
  }
  const csVector3& CS_VEC_TILT_DOWN()
  {
    static const csVector3 v ( 1,  0,  0);
    return v;
  }
%}

/****************************************************************************
 * Create a typemap for the i/csString wrappings below.
 ****************************************************************************/
TYPEMAP_OUTARG_ARRAY_PTR_CNT((char * & __chars__, int & __len__), 0, *)

/****************************************************************************
 * Some extra operator overloads for iString.
 ****************************************************************************/
%extend iString
{
  bool __seq__ (iString *other)
  {
    return self->Compare (other);
  }
  const char* __string__ ()
  {
    return self->GetData ();
  }
  const char* __sv__ ()
  {
    return self->GetData ();
  }
  void __av__ (char *&__chars__, int &__len__)
  {
    __chars__ = self->GetData ();
    __len__ = self->Length ();
  }
  int length ()
  {
    return self->Length ();
  }
  csPtr<iString> __add__ (const char *other)
  {
    iString *result = new scfString (* self);
    result->Append (other);
    return csPtr<iString> (result);
  }
  csPtr<iString> __concat__ (const char *other)
  {
    iString *result = new scfString (* self);
    result->Append (other);
    return csPtr<iString> (result);
  }
  iString* __concat_ass__ (const char *other)
  {
    self->Append (other);
    return self;
  }
  csPtr<iString> __concat__ (iString *other)
  {
    iString *result = new scfString (* self);
    result->Append (other);
    return csPtr<iString> (result);
  }
  iString* __concat_ass__ (iString *other)
  {
    self->Append (other);
    return self;
  }
}

/****************************************************************************
 * Some extra operator overloads for csString.
 ****************************************************************************/
%extend csString
{
  bool __eq__ (const char *other)
  {
    return self->Compare (other);
  }
  bool __seq__ (csString &other)
  {
    return self->Compare (other);
  }

  const char* __string__ ()
  {
    return self->GetData ();
  }
  const char* __sv__ ()
  {
    return self->GetData ();
  }
  void __av__ (char *&__chars__, int &__len__)
  {
    __chars__ = self->GetData ();
    __len__ = self->Length ();
  }
  int length ()
  {
    return self->Length ();
  }

  csString __add__ (const char *other)
  {
    csString result (* self);
    result.Append (other);
    return result;
  }
  csString __concat__ (const char *other)
  {
    csString result (* self);
    result.Append (other);
    return result;
  }
  csString* __concat_ass__ (const char *other)
  {
    self->Append (other);
    return self;
  }
  csString __concat__ (csString &other)
  {
    csString result (* self);
    result.Append (other);
    return result;
  }
  csString* __concat_ass__ (csString &other)
  {
    self->Append (other);
    return self;
  }
}

/****************************************************************************
 * Some extra operator overloads for iDataBuffer.
 ****************************************************************************/
%extend iDataBuffer
{
  const char* __string__ ()
  {
    return self->operator* ();
  }
  const char* __sv__ ()
  {
    return self->operator* ();
  }
}

/*****************************************************************************
 * Define macros to create classes that are inheritable by script classes,
 * to allow scripts to write their own implementations of interfaces.
 *****************************************************************************/
%define WRAP_SCRIPT_CLASS(TYPE, BODY)
  %{
    class csWrap_##TYPE : public TYPE
    {
      SV *sv;
     public:
      SCF_DECLARE_IBASE;
      csWrap_##TYPE (SV *sv0) : sv (sv0)
      {
        SCF_CONSTRUCT_IBASE (0);
        SvREFCNT_inc (sv);
      }
      virtual ~csWrap_##TYPE ()
      {
        SvREFCNT_dec (sv);
      }

      BODY
    };

    SCF_IMPLEMENT_IBASE (csWrap_##TYPE)
      SCF_IMPLEMENTS_INTERFACE (TYPE)
    SCF_IMPLEMENT_IBASE_END
  %}

  %typemap(in) TYPE*
  {
    if (sv_isa ($input, "cspace::" #TYPE))
      $1 = (TYPE *) SvIV (SvRV ($input));
    else if (sv_inherits ($input, "cspace::" #TYPE))
      $1 = new csWrap_##TYPE ($input);
    else
      croak ("Expected object of type %s", #TYPE);
  }
  %typemap(freearg) TYPE*
  {
    $1->DecRef ();
  }
%enddef

%define WRAP_FUNC(TYPE, FUNC, ARGS, PUSHARGS, POPRETVAL)
  virtual TYPE FUNC ARGS
  {
    dTARG;
    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK (SP);
    XPUSHs (sv);

    PUSHARGS

    PUTBACK;
    call_method (#FUNC, G_SCALAR);
    SPAGAIN;

    TYPE retval = (TYPE) (POPRETVAL);

    PUTBACK;
    FREETMPS;
    LEAVE;
    return retval;
  }
%enddef

%define WRAP_VOID(FUNC, ARGS, PUSHARGS)
  virtual void FUNC ARGS
  {
    dTARG;
    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK (SP);
    XPUSHs (sv);

    PUSHARGS

    PUTBACK;
    call_method (#FUNC, G_DISCARD);
    SPAGAIN;

    PUTBACK;
    FREETMPS;
    LEAVE;
  }
%enddef

/*****************************************************************************
 * Use the macros we defined above for iEventHandler and iAwsSink.
 *****************************************************************************/
WRAP_SCRIPT_CLASS (iEventHandler,
  WRAP_FUNC (bool, HandleEvent, (iEvent &ev),
    SV *ev_rv = newSViv (0);
    sv_setref_iv (ev_rv, "cspace::iEvent", (int) & ev);
    XPUSHs (sv_2mortal (ev_rv));,

    POPi
  )
)

WRAP_SCRIPT_CLASS (iEventPlug,
  WRAP_FUNC (unsigned int, GetPotentiallyConflictingEvents, (),
    /**/,

    POPu
  )
  WRAP_FUNC (unsigned int, QueryEventPriority, (unsigned int type),
    XPUSHu (type);,

    POPu
  )
  WRAP_VOID (EnableEvents, (unsigned int type, bool enabled),
    XPUSHu (type);
    XPUSHi (enabled ? 1 : 0);
  )
)

WRAP_SCRIPT_CLASS (iAwsSink,
  WRAP_FUNC (unsigned long, GetTriggerID, (const char *name),
    XPUSHp (name, strlen (name));,

    POPu
  )
  WRAP_VOID (HandleTrigger, (int id, iAwsSource *src),
    XPUSHi (id);
    SV *ev_rv = newSViv (0);
    sv_setref_iv (ev_rv, "cspace::iAwsSource", (int) src);
    XPUSHs (sv_2mortal (ev_rv));
  )
  WRAP_VOID (RegisterTrigger, (const char *n, void (*t) (void*, iAwsSource*)),
    /**/
  )
  WRAP_FUNC (unsigned int, GetError, (),
    /**/,

    POPu
  )
)

#endif // SWIGPERL5
