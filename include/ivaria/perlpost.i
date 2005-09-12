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

/*
    Recent releases of Swig contain a nasty bug where invocations of
    function-like macros taking zero arguments are incorrectly transformed into
    non-function-like macros. For instance, when Swig sees SCF_DESTRUCT_IBASE()
    in inline code, it transforms it to SCF_DESTRUCT_IBASE, which later results
    in a compilation error since the C-preprocessor does not have knowledge of
    this non-function-like macro. We work around the problem by preventing Swig
    from knowing that SCF_DESTRUCT_IBASE is a macro.
*/
#undef SCF_DESTRUCT_IBASE

%{
#include <csutil/csstring.h>
%}

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
    scfInitialize (1, argv);
  }
%}

/****************************************************************************
 * This is CS's interface to the Perl script's event handler.
 ****************************************************************************/
#ifndef CS_MICRO_SWIG
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
    SV *event_obj = newSViv (0);
    SWIG_MakePtr (event_obj, &event, SWIGTYPE_p_iEvent, 0);

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

    iObjectRegistry* reg;
    if (SWIG_ConvertPtr(reg_ref,(void**)&reg,SWIGTYPE_p_iObjectRegistry,0) < 0)
      croak("Type error. Argument 1 must be iObjectRegistry.\n");

    unsigned int mask;
    if (mask_sv)
      mask = SvUV (mask_sv);
    else
      mask = csInitializer_SetupEventHandler_DefaultMask;

    SV *func = SvRV (func_rv);
    pl_csInitializer_EventHandler = func;

    bool ok = csInitializer::SetupEventHandler
      (reg, csInitializer_EventHandler, mask);
    XSRETURN_IV (ok ? 1 : 0);
  }
%}
#endif // CS_MICRO_SWIG

/****************************************************************************
 * This is a wrapper function for RequestPlugin's variable argument list.
 ****************************************************************************/
%native(csInitializer_RequestPlugins) _csInitializer_RequestPlugins;
%{
  void _csInitializer_RequestPlugins (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    dTARG;
    iObjectRegistry* reg;
    if (SWIG_ConvertPtr(ST(0),(void**)&reg,SWIGTYPE_p_iObjectRegistry,0) < 0)
      croak("Type error. Argument 1 must be iObjectRegistry.\n");

    csArray<csPluginRequest> plugins;
    for (int arg = 1; arg + 4 <= items; arg += 4)
    {
      SV *plug_sv = ST (arg);
      SV *iface_sv = ST (arg + 1);
      SV *scfid_sv = ST (arg + 2);
      SV *ver_sv = ST (arg + 3);

      const char *plug = SvPV_nolen (plug_sv);
      const char *iface = SvPV_nolen (iface_sv);
      int scfid = SvIV (scfid_sv);
      int ver = SvIV (ver_sv);

      plugins.Push(csPluginRequest(plug, iface, scfid, ver));
    }

    bool ok = true;
    if (plugins.Length() != 0)
      ok = csInitializer::RequestPlugins(reg, plugins);
    XSRETURN_IV (ok ? 1 : 0);
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
    __scfid__ = "crystalspace.graphic.image.io.multiplexer";
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

#ifndef CS_MINI_SWIG
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

    iCollideSystem* sys;
    if (SWIG_ConvertPtr(sys_ref,(void**)&sys,SWIGTYPE_p_iCollideSystem,0) < 0)
      croak("Self parameter of GetCollisionPairs must be an iCollideSystem\n");

    csCollisionPair *pairs = sys->GetCollisionPairs ();
    int num = sys->GetCollisionPairCount ();
    AV *av = newAV ();
    for (int i = 0; i < num; i++)
    {
      SV *rv = newSViv (0);
      SWIG_MakePtr (rv, pairs++, SWIGTYPE_p_csCollisionPair, 0);
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
#endif // CS_MINI_SWIG

/****************************************************************************
 * Create a typemap for the i/csString wrappings below.
 ****************************************************************************/
TYPEMAP_OUTARG_ARRAY_PTR_CNT((char * & __chars__, int & __len__), 0, *)

#ifndef CS_MINI_SWIG
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
  void __av__ (const char *&__chars__, int &__len__)
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
#endif // CS_MINI_SWIG

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
  void __av__ (const char *&__chars__, int &__len__)
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

#ifndef CS_MINI_SWIG
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
#endif // CS_MINI_SWIG

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
	SCF_DESTRUCT_IBASE();
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
    {
      SWIG_ConvertPtr($input,(void**)&($1),SWIG_TypeQuery("cspace::"#TYPE),0);
      $1->IncRef();
    }
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
#ifndef CS_MICRO_SWIG
WRAP_SCRIPT_CLASS (iEventHandler,
  WRAP_FUNC (bool, HandleEvent, (iEvent &ev),
    SV *ev_rv = newSViv (0);
    SWIG_MakePtr (ev_rv, &ev, SWIGTYPE_p_iEvent, 0);
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
#endif // CS_MICRO_SWIG

#ifndef CS_MINI_SWIG
WRAP_SCRIPT_CLASS (iAwsSink,
  WRAP_FUNC (unsigned long, GetTriggerID, (const char *name),
    XPUSHp (name, strlen (name));,

    POPu
  )
  WRAP_VOID (HandleTrigger, (int id, iAwsSource *src),
    XPUSHi (id);
    SV *ev_rv = newSViv (0);
    SWIG_MakePtr (ev_rv, src, SWIGTYPE_p_iAwsSource, 0);
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
#endif // CS_MINI_SWIG

#endif // SWIGPERL5
