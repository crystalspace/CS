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
  AV *argv = get_av ("_", 1);
  SV *reg_ref = av_shift (argv);
  SV *func_rv = av_shift (argv);
  SV *mask_sv = av_shift (argv);
  if (! SvTRUE (reg_ref) || ! SvTRUE (func_rv))
  {
    croak ("SetupEventHandler needs at least 2 arguments");
    SvREFCNT_dec (reg_ref);
    SvREFCNT_dec (func_rv);
    return;
  }
  unsigned int mask;
  if (! SvTRUE (mask_sv))
  {
    mask = SvUV (mask_sv);
    SvREFCNT (mask_sv);
  }
  else
    mask = csInitializer_SetupEventHandler_DefaultMask;

  SV *reg_obj = SvRV (reg_ref);
  SvREFCNT_dec (reg_ref);
  if (! sv_isa (reg_obj, "cspace::iObjectRegistry"))
  {
    croak ("SetupEventHandler argument 1 must be iObjectRegistry");
    SvREFCNT_dec (reg_obj);
    SvREFCNT_dec (func_rv);
    return;
  }
  iObjectRegistry *reg = (iObjectRegistry *) SvIV (reg_obj);
  SvREFCNT_dec (reg_obj);

  SV *func = SvRV (func_rv);
  SvREFCNT_inc (func);
  pl_csInitializer_EventHandler = func;
  SvREFCNT_dec (func_rv);

  bool ok = csInitializer::SetupEventHandler
    (reg, csInitializer_EventHandler, mask);
  SV *ret = get_sv ("_", 1);
  sv_setiv (ret, ok ? 1 : 0);
}
%}

%native(csInitializer_RequestPlugins) _csInitializer_RequestPlugins;
%{
void _csInitializer_RequestPlugins (pTHXo_ CV *thisfunc)
{
  AV *argv = get_av ("_", 1);
  SV *reg_ref = av_shift (argv);
  SV *reg_obj = SvRV (reg_ref);
  SvREFCNT_dec (reg_ref);
  if (! sv_isa (reg_obj, "cspace::iObjectRegistry"))
  {
    croak ("RequestPlugins argument 1 must be iObjectRegistry");
    SvREFCNT_dec (reg_obj);
    return;
  }
  iObjectRegistry *reg = (iObjectRegistry *) SvIV (reg_obj);
  SvREFCNT_dec (reg_obj);

  bool ok;
  while (true)
  {
    if (! SvTRUE (reg_ref)) break;
    SV *plug_sv = av_shift (argv);
    SV *iface_sv = av_shift (argv);
    SV *scfid_sv = av_shift (argv);
    SV *ver_sv = av_shift (argv);

    const char *plug = SvPV_nolen (plug_sv);
    const char *iface = SvPV_nolen (iface_sv);
    int scfid = SvIV (scfid_sv);
    int ver = SvIV (ver_sv);

    SvREFCNT_dec (plug_sv);
    SvREFCNT_dec (iface_sv);
    SvREFCNT_dec (ver_sv);
    SvREFCNT_dec (scfid_sv);

    bool ok1 = csInitializer::RequestPlugins
      (reg, plug, iface, scfid, ver, 0);
    if (! ok1) ok = false;
  }

  SV *ret = get_sv ("_", 1);
  sv_setiv (ret, ok /*? 1 : 0*/);
}
%}

%inline %{
  csWrapPtr _CS_QUERY_REGISTRY (iObjectRegistry *reg, const char *iface,
    int iface_ver)
  {
    return csWrapPtr (iface, reg->Get
      (iface, iSCF::SCF->GetInterfaceID (iface), iface_ver));
  }

  csWrapPtr _CS_QUERY_REGISTRY_TAG_INTERFACE (iObjectRegistry *reg,
    const char *tag, const char *iface, int iface_ver)
  {
    return csWrapPtr (iface, reg->Get
      (tag, iSCF::SCF->GetInterfaceID (iface), iface_ver));
  }

  csWrapPtr _SCF_QUERY_INTERFACE (iBase *obj, const char *iface, int iface_ver)
  {
    return csWrapPtr (iface, (iBase *) obj->QueryInterface
      (iSCF::SCF->GetInterfaceID (iface), iface_ver));
  }

  csWrapPtr _SCF_QUERY_INTERFACE_SAFE (iBase *obj, const char *iface,
    int iface_ver)
  {
    return csWrapPtr (iface, (iBase *) iBase::QueryInterfaceSafe
      (obj, iSCF::SCF->GetInterfaceID (iface), iface_ver));
  }

  csWrapPtr _CS_QUERY_PLUGIN_CLASS (iPluginManager *obj, const char *id,
    const char *iface, int iface_ver)
  {
    return csWrapPtr (iface, obj->QueryPlugin (id, iface, iface_ver));
  }

  csWrapPtr _CS_LOAD_PLUGIN (iPluginManager *obj, const char *id,
    const char *iface, int iface_ver)
  {
    return csWrapPtr (iface, obj->LoadPlugin (id, iface, iface_ver));
  }

  csWrapPtr _CS_GET_CHILD_OBJECT (iObject *obj, const char *iface,
    int iface_ver)
  {
    return csWrapPtr (iface, (iBase *) obj->GetChild
      (iSCF::SCF->GetInterfaceID (iface), iface_ver));
  }

  csWrapPtr _CS_GET_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
    int iface_ver, const char *name)
  {
    return csWrapPtr (iface, (iBase *) obj->GetChild
      (iSCF::SCF->GetInterfaceID (iface), iface_ver, name));
  }

  csWrapPtr _CS_GET_FIRST_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
    int iface_ver, const char *name)
  {
    return csWrapPtr (iface, (iBase *) obj->GetChild
      (iSCF::SCF->GetInterfaceID (iface), iface_ver, name, true));
  }
%}

#if 0
%perl5code %{
sub CS_REQUEST_VFS
  { CS_REQUEST_PLUGIN ('crystalspace.kernel.vfs', iVFS) }

sub CS_REQUEST_FONTSERVER
  { CS_REQUEST_PLUGIN ('crystalspace.font.server.default', iFontServer) }

sub CS_REQUEST_IMAGELOADER
  { CS_REQUEST_PLUGIN ('crystalspace.graphic.image.io.multiplex', iImageIO) }

sub CS_REQUEST_NULL3D
  { CS_REQUEST_PLUGIN ('crystalspace.graphics3d.null', iGraphics3D) }

sub CS_REQUEST_SOFTWARE3D
  { CS_REQUEST_PLUGIN ('crystalspace.graphics3d.software', iGraphics3D) }

sub CS_REQUEST_OPENGL3D
  { CS_REQUEST_PLUGIN ('crystalspace.graphics3d.opengl', iGraphics3D) }

sub CS_REQUEST_ENGINE
  { CS_REQUEST_PLUGIN ('crystalspace.engine.3d', iEngine) }

sub CS_REQUEST_LEVELLOADER
  { CS_REQUEST_PLUGIN ('crystalspace.level.loader', iLoader) }

sub CS_REQUEST_LEVELSAVER
  { CS_REQUEST_PLUGIN ('crystalspace.level.saver', iSaver) }

sub CS_REQUEST_REPORTER
  { CS_REQUEST_PLUGIN ('crystalspace.utilities.reporter', iReporter) }

sub CS_REQUEST_REPORTERLISTENER
  { CS_REQUEST_PLUGIN ('crystalspace.utilities.stdrep', iStandardReporterListener) }

sub CS_REQUEST_CONSOLEOUT
  { CS_REQUEST_PLUGIN ('crystalspace.console.output.simple', iConsoleOutput) }
%}
#endif // 0

#if 0
%extend iCollideSystem
{
  %native(GetCollisionPairs) _GetCollisionPairs;
  %{
  void _GetCollisionPairs (pTHXo_ CV *thisfunc)
  {
    AV *argv = get_av ("_", 1);
    SV *sys_ref = av_shift (argv);

    if (! SvTRUE (sys_ref))
    {
      croak("No self parameter passed to GetCollisionPairs");
      return;
    }

    SV *sys_obj = SvRV (sys_ref));
    SvREFCNT_dec (sys_ref);
    if (! sv_isa (sys_obj, "cspace::iCollideSystem"))
    {
      croak("Self parameter of GetCollisionPairs must be an iCollideSystem");
      return;
    }

    iCollideSystem *sys = (iCollideSystem *) SvPV_nolen (sys_obj);

    int num = sys->GetCollisionPairCount ();
    AV *av = newAV ();
    for (int i = 0; i < num; i++)
      av_push (av, sys->GetCollisionPairs () + i);

    SV *rv = newRV ((SV *) av);
    HvREFCNT_dec (av);
    SV *ret = get_sv ("_", 1);
    sv_setsv (ret, rv);
    SvREFCNT_dec (rv);
  }
  %}
}
#endif // 0

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

#if 0
%{
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
%}
csVector3 CS_VEC_FORWARD()	{ return csVector3 ( 0,  0,  1); }
csVector3 CS_VEC_BACKWARD()	{ return csVector3 ( 0,  0, -1); }
csVector3 CS_VEC_RIGHT()	{ return csVector3 ( 1,  0,  0); }
csVector3 CS_VEC_LEFT()		{ return csVector3 (-1,  0,  0); }
csVector3 CS_VEC_UP()		{ return csVector3 ( 0,  1,  0); }
csVector3 CS_VEC_DOWN()		{ return csVector3 ( 0, -1,  0); }
csVector3 CS_VEC_ROT_RIGHT()	{ return csVector3 ( 0,  1,  0); }
csVector3 CS_VEC_ROT_LEFT()	{ return csVector3 ( 0, -1,  0); }
csVector3 CS_VEC_TILT_RIGHT()	{ return csVector3 ( 0,  0, -1); }
csVector3 CS_VEC_TILT_LEFT()	{ return csVector3 ( 0,  0,  1); }
csVector3 CS_VEC_TILT_UP()	{ return csVector3 (-1,  0,  0); }
csVector3 CS_VEC_TILT_DOWN()	{ return csVector3 ( 1,  0,  0); }
#endif // 0

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

#if 0
%extend csHashMapReversible
{
  %native(__hv__) ___hv__;
  %{
  void ___hv__ (pTHXo_ CV *thisfunc)
  {
    AV *argv = get_av ("_", 1);
    SV *hm_ref = av_shift (argv);
    if (! SvTRUE (hm_ref))
    {
      croak("Malformed \%{} request of csHashMapReversible");
      return;
    }

    SV *hm_obj = SvRV (hm_ref));
    SvREFCNT_dec (hm_ref);
    if (! sv_isa (hm_obj, "cspace::csHashMapReversible"))
    {
      croak("Erroneous \%{} request of non-csHashMapReversible");
      return;
    }

    csHashMapReversible *hm = (csHashMapReversible *) SvPV_nolen (hm_obj);
  
    csHashIteratorReversible (hm);
    HV *hv = newHV ();
    void *value;
    while ((value = iter->Next ()))
    {
      const char *key = iter->GetKey ();
 
      SV *sv = newSViv ((int) value);
      hv_store (hv, key, strlen (key), sv);
      SvREFCNT_dec (sv);
    }
    RV *rv = newRV ((SV *) hv);
    HvREFCNT_dec (hv);
    SV *ret = get_sv ("_", 1);
    sv_setsv (ret, rv);
    SvREFCNT_dec (rv);
  }
  %}
}
#endif // 0

#endif // SWIGPERL5
