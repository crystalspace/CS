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

%native(AUTOLOAD) AutoLoad;
%{
  void AutoLoad (pTHXo_ CV *thisfunc)
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
    XSRETURN_SV (* valp);
  }
%}

%{
  static const int csInitializer_SetupEventHandler_DefaultMask
    = CSMASK_Nothing
    | CSMASK_Broadcast
    | CSMASK_MouseUp
    | CSMASK_MouseDown
    | CSMASK_MouseMove
    | CSMASK_KeyDown
    | CSMASK_KeyUp
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

    for (int ok = 1, arg = 1; ok; arg += 4)
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

%{
  void csRequestPlugin (char *iface, int &idnum, int &ver)
  {
    idnum = iSCF::SCF->GetInterfaceID (iface);
    ver = scfGetVersion (iface);
  }
%}

%apply char * OUTPUT { char *& __scfid__ };
%apply char * OUTPUT { char *& __iface__ };
%apply char * INOUT { char *& __Iscfid__ };
%apply char * INOUT { char *& __Iiface__ };
%apply int * OUTPUT { int & __idnum__ };
%apply int * OUTPUT { int & __ver__ };

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
    XSRETURN_SV (rv);
  }
%}

%apply float * OUTPUT { float & __v1__ };
%apply float * OUTPUT { float & __v2__ };
%apply float * OUTPUT { float & __v3__ };

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

TYPEMAP_OUTARG_ARRAY_PTR_CNT((char * & __chars__, int & __len__), 0, *)

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
    iString *result = new scfString (self);
    result->Append (other);
    return csPtr<iString> (result);
  }
  csPtr<iString> __concat__ (const char *other)
  {
    iString *result = new scfString (self);
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
    iString *result = new scfString (self);
    result->Append (other);
    return csPtr<iString> (result);
  }
  iString* __concat_ass__ (iString *other)
  {
    self->Append (other);
    return self;
  }
}

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
    csString result (self);
    result->Append (other);
    return result;
  }
  csString* __add_ass__ (const char *other)
  {
    self->Append (other);
    return self;
  }
  csString* __add_ass__ (csString &other)
  {
    self->Append (other);
    return self;
  }
  csString __concat__ (const char *other)
  {
    csString result (self);
    result->Append (other);
    return result;
  }
  csString* __concat_ass__ (const char *other)
  {
    self->Append (other);
    return self;
  }
  csString __concat__ (csString &other)
  {
    csString result (self);
    result->Append (other);
    return result;
  }
  csString* __concat_ass__ (csString &other)
  {
    self->Append (other);
    return self;
  }
}

%native(csHashMapReversible___hv__) ___hv__;
%{
  void ___hv__ (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    dTARG;
    SV *hm_ref = ST (0);
    if (! hm_ref)
      croak("Malformed \%{} request of csHashMapReversible");

    SV *hm_obj = SvRV (hm_ref);
    if (! sv_isa (hm_obj, "cspace::csHashMapReversible"))
      croak("Erroneous \%{} request of non-csHashMapReversible");

    csHashMapReversible *hm = (csHashMapReversible *) SvIV (hm_obj);
  
    csGlobalHashIteratorReversible iter (hm);
    HV *hv = newHV ();
    void *value;
    while ((value = iter.Next ()))
    {
      const char *key = iter.GetKey ();
 
      SV *sv = newSViv ((int) value);
      hv_store (hv, key, strlen (key), sv, 0);
      SvREFCNT_dec (sv);
    }
    SV *rv = newRV ((SV *) hv);
    SvREFCNT_dec ((SV *) hv);
    XSRETURN_SV (rv);
  }
%}

#endif // SWIGPERL5
