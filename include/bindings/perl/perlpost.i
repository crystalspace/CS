/*
    Copyright (C) 2003, 2007 Mat Sutcliffe <oktal@gmx.co.uk>

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
 * The 'import' class method allows the user to import symbols from cspace
 * into the global namespace e.g. "use cspace qw(csInitializer csVector3);"
 ****************************************************************************/
%perlcode %{
  use Carp;

  sub import
  {
    shift;

    my ($caller) = caller;
    $caller .= '::';

    foreach my $symbol (@_)
    {
      my $stash = $symbol . '::';

      if (exists $cspace::{$stash})
      {
        if (exists $::{$stash} and $::{$stash} != $cspace::{$stash})
          { carp "Namespace pollution: replacing existing package $symbol" }

        *{ $stash } = *{ "cspace::$stash" };
      }
      elsif (exists $cspace::{$symbol})
      {
        if (exists $caller->{$symbol})
          { carp "Namespace pollution: replacing existing symbol $symbol" }

        *{ "$caller$symbol" } = *{ "cspace::$symbol" };
      }
      else { croak "No such package or symbol cspace::$symbol" }
    }
  }
%}

/****************************************************************************
 * This implements the perl 'exists' built-in, for the tied hashes used by
 * SWIG for accessing object properties. It is used by AUTOLOAD below, and is
 * generally a good thing to have anyway.
 ****************************************************************************/
%perlcode %{
  sub EXISTS
  {
    my ($self, $prop) = @_;

    return $self->can('swig_' . $prop . '_get');
  }
%}

/****************************************************************************
 * The AUTOLOAD function is called by Perl if the user tries to call a
 * non-existant object method. We use this to add loads of custom methods
 * to every class without bloating the module. Specifically, we add object
 * property accessor methods.
 ****************************************************************************/
%perlcode %{
  sub AUTOLOAD
  {
    my ($self, $val) = @_;

    unless (@_ >= 1 and @_ <= 2 and ref $self and $self->isa('cspace'))
      { croak "No such subroutine $AUTOLOAD" }

    $AUTOLOAD =~ s/^.*:://;

    unless (exists $self->{$AUTOLOAD})
      { croak "No such member $AUTOLOAD in class " . ref $self }

    if (@_ == 2)
      { return $self->{$AUTOLOAD} = $val }
    else
      { return $self->{$AUTOLOAD} }
  }
%}

/****************************************************************************
 * This function returns the pointer of a SWIGified object, casted to an int.
 * It is used by csPerl5::Object::GetPointer() in the perl5 scripting plugin.
 ****************************************************************************/
%native(_GetCPointer) _GetCPointer;
%{
  void _GetCPointer (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    SV *obj = ST (0);

    if (! (items == 1 && SvOK (obj) && sv_isobject (obj)))
      croak ("Malformed call to _GetCPointer");

    void *ptr;
    int err = SWIG_ConvertPtr (obj, &ptr, 0, 0);

    if (! SWIG_IsOK (err))
      croak ("Failed to access pointer from wrapped object");

    XSRETURN_IV ((PTRV) ptr);
  }
%}

/****************************************************************************
 * This function allows the perl5 scripting plugin to store a reference to
 * the iObjectRegistry in the script. The pointer is first casted to an int.
 ****************************************************************************/
%native(_SetObjectReg) _SetObjectReg;
%{
  void _SetObjectReg (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    SV *objreg = ST (0);

    if (items != 1) croak ("Malformed call to _SetObjectReg");

    void *ptr = (void *) SvIV (objreg);
    SV *sv = get_sv ("cspace::object_reg", TRUE);
    SWIG_MakePtr (sv, ptr, SWIGTYPE_p_iObjectRegistry, 0);

    XSRETURN_EMPTY;
  }
%}

#ifndef CS_MINI_SWIG
/****************************************************************************
 * This is CS's interface to the Perl script's event handler.
 ****************************************************************************/
%{
  static SV * EventHandler_sv = 0;

  bool plEventHandler (iEvent &event)
  {
    if (! EventHandler_sv) return false;

    dSP;
    ENTER;
    SAVETMPS;

    SV *event_sv = sv_newmortal ();
    SWIG_MakePtr (event_sv, &event, SWIGTYPE_p_iEvent, 0);

    PUSHMARK (SP);
    XPUSHs (event_sv);
    PUTBACK;

    int n = call_sv (EventHandler_sv, G_SCALAR);

    SPAGAIN;
    if (n != 1) croak ("Perly event handler must return 1 value only");
    SV *result_sv = POPs;
    bool result = SvTRUE (result_sv);
    PUTBACK;

    FREETMPS;
    LEAVE;

    return result;
  }
%}

/****************************************************************************
 * This is the Perl script's interface to SetupEventHandler.
 ****************************************************************************/
%perlcode %{
  *cspace::csInitializer::SetupEventHandler = *_SetupEventHandler;
%}
%native(_SetupEventHandler) _SetupEventHandler;
%{
  void _SetupEventHandler (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    SV *reg_sv = ST (0);
    SV *func_rv = ST (1);
    SV *types_sv = ST (2);
    if (items < 2 || items > 3)
      croak ("Usage: SetupEventHandler(object_reg, coderef, [ event_types ])");

    iObjectRegistry* reg;
    int status = SWIG_ConvertPtr
	(reg_sv, (void**) &reg, SWIGTYPE_p_iObjectRegistry, 0);
    if (! SWIG_IsOK (status)) croak("Argument 1 must be iObjectRegistry");
    csRef<iEventNameRegistry> name_reg
      (csQueryRegistry<iEventNameRegistry> (reg));
    if (! name_reg.IsValid ()) croak("Can't find iEventNameRegistry");

    if (! (SvROK (func_rv) && SvTYPE (SvRV (func_rv)) == SVt_PVCV))
      croak("Argument 2 must be a code reference");

    csDirtyAccessArray<csEventID> types;
    if (items == 3)
    {
      if (! SvROK (types_sv)) croak("Argument 3 must be an array reference");
      AV *av = (AV *) SvRV (types_sv);
      if (SvTYPE(av)!=SVt_PVAV) croak("Argument 3 must be an array reference");
      for (int i = 0, last = av_len (av); i <= last; i++)
        types.Push (name_reg->GetID (SvPV_nolen (* av_fetch (av, i, 0))));
      types.Push (CS_EVENTLIST_END);
    }

    if (EventHandler_sv)
      SvSetSV (EventHandler_sv, func_rv);
    else
      EventHandler_sv = newSVsv (func_rv);

    bool ok = (items == 3)
      ? csInitializer::SetupEventHandler (reg, plEventHandler, types.GetArray())
      : csInitializer::SetupEventHandler (reg, plEventHandler);
    XSRETURN_IV (ok ? 1 : 0);
  }
%}
#endif // CS_MINI_SWIG

#ifndef CS_MICRO_SWIG
/****************************************************************************
 * Wrapper function to get the array returned by GetCollisionPairs().
 ****************************************************************************/
%perlcode %{
  *cspace::iCollideSystem::GetCollisionPairs = *_GetCollisionPairs;
%}
%native(_GetCollisionPairs) _GetCollisionPairs;
%{
  void _GetCollisionPairs (pTHXo_ CV *thisfunc)
  {
    dXSARGS;
    SV *sys_ref = ST (0);
    if (items != 1)
      croak ("Usage: $collideSystem->GetCollisionPairs()");

    iCollideSystem* sys;
    int status = SWIG_ConvertPtr
	(sys_ref, (void**) &sys, SWIGTYPE_p_iCollideSystem, 0);
    if (! SWIG_IsOK (status))
      croak ("iCollideSystem::GetCollisionPairs needs an iCollideSystem ref");

    csCollisionPair *pairs = sys->GetCollisionPairs ();
    int num = sys->GetCollisionPairCount ();

    AV *av = newAV ();
    for (int i = 0; i < num; i++)
    {
      SV *rv = newSViv (0);
      SWIG_MakePtr (rv, pairs + i, SWIGTYPE_p_csCollisionPair, 0);
      av_push (av, rv);
    }

    SV *rv = sv_2mortal (newRV_noinc ((SV *) av));
    ST (0) = rv;
    XSRETURN (1);
  }
%}

/****************************************************************************
 * Pure perl replacements for the CS_VEC_... macros.
 ****************************************************************************/
%perlcode %{
  sub CS_VEC_FORWARD	{ new cspace::csVector3 ( 0,  0,  1) }
  sub CS_VEC_BACKWARD	{ new cspace::csVector3 ( 0,  0, -1) }
  sub CS_VEC_RIGHT	{ new cspace::csVector3 ( 1,  0,  0) }
  sub CS_VEC_LEFT	{ new cspace::csVector3 (-1,  0,  0) }
  sub CS_VEC_UP		{ new cspace::csVector3 ( 0,  1,  0) }
  sub CS_VEC_DOWN	{ new cspace::csVector3 ( 0, -1,  0) }
  sub CS_VEC_ROT_RIGHT	{ new cspace::csVector3 ( 0,  1,  0) }
  sub CS_VEC_ROT_LEFT	{ new cspace::csVector3 ( 0, -1,  0) }
  sub CS_VEC_TILT_RIGHT	{ new cspace::csVector3 ( 0,  0, -1) }
  sub CS_VEC_TILT_LEFT	{ new cspace::csVector3 ( 0,  0,  1) }
  sub CS_VEC_TILT_UP	{ new cspace::csVector3 (-1,  0,  0) }
  sub CS_VEC_TILT_DOWN	{ new cspace::csVector3 ( 1,  0,  0) }
%}

/****************************************************************************
 * CSKEY_SHIFT/CTRL/ALT replacements.
 ****************************************************************************/
%perlcode %{
  sub CSKEY_SHIFT	{ CSKEY_SHIFT_NUM (csKeyModifierNumAny ()) }
  sub CSKEY_SHIFT_LEFT	{ CSKEY_SHIFT_NUM (csKeyModifierNumLeft ()) }
  sub CSKEY_SHIFT_RIGHT	{ CSKEY_SHIFT_NUM (csKeyModifierNumRight ()) }
  sub CSKEY_CTRL	{ CSKEY_CTRL_NUM  (csKeyModifierNumAny ()) }
  sub CSKEY_CTRL_LEFT	{ CSKEY_CTRL_NUM  (csKeyModifierNumLeft ()) }
  sub CSKEY_CTRL_RIGHT	{ CSKEY_CTRL_NUM  (csKeyModifierNumRight ()) }
  sub CSKEY_ALT		{ CSKEY_ALT_NUM   (csKeyModifierNumAny ()) }
  sub CSKEY_ALT_LEFT	{ CSKEY_ALT_NUM   (csKeyModifierNumLeft ()) }
  sub CSKEY_ALT_RIGHT	{ CSKEY_ALT_NUM   (csKeyModifierNumRight ()) }
%}

/****************************************************************************
 * Pure perl replacement for RequestPlugins' variadic argument list.
 ****************************************************************************/
%perlcode %{
  *cspace::csInitializer::RequestPlugins = *_RequestPlugins;

  sub _RequestPlugins
  {
    my $object_reg = shift;
    my $array = new cspace::csPluginRequestArray ();

    while (1)
    {
      my $plug = shift; my $iface = shift; my $scfid = shift; my $ver = shift;
      last unless
	(defined $plug and defined $iface and defined $scfid and defined $ver);

      $array->Push (new cspace::csPluginRequest ($plug,$iface,$scfid,$ver));
    }

    return cspace::csInitializer::_RequestPlugins ($object_reg, $array);
  }
%}

/****************************************************************************
 * Pure perl replacements for the CS_REQUEST_... macros.
 ****************************************************************************/
%perlcode %{
  sub CS_REQUEST_PLUGIN
  {
    my ($scfid, $iface) = @_;
    return (new cspace::csString ($scfid),
	    new cspace::csString ($iface),
	    $cspace::iSCF::SCF->GetInterfaceID ($iface),
	    ('cspace::' . $iface . '::scfGetVersion')->());
  }
  sub CS_REQUEST_VFS { CS_REQUEST_PLUGIN
	('crystalspace.kernel.vfs', 'iVFS') }
  sub CS_REQUEST_FONTSERVER { CS_REQUEST_PLUGIN
	('crystalspace.font.server.default', 'iFontServer') }
  sub CS_REQUEST_IMAGELOADER { CS_REQUEST_PLUGIN
	('crystalspace.graphic.image.io.multiplexer', 'iImageIO') }
  sub CS_REQUEST_NULL3D { CS_REQUEST_PLUGIN
	('crystalspace.graphics3d.null', 'iGraphics3D') }
  sub CS_REQUEST_SOFTWARE3D { CS_REQUEST_PLUGIN
	('crystalspace.graphics3d.software', 'iGraphics3D') }
  sub CS_REQUEST_OPENGL3D { CS_REQUEST_PLUGIN
	('crystalspace.graphics3d.opengl', 'iGraphics3D') }
  sub CS_REQUEST_ENGINE { CS_REQUEST_PLUGIN
	('crystalspace.engine.3d', 'iEngine') }
  sub CS_REQUEST_LEVELLOADER { CS_REQUEST_PLUGIN
	('crystalspace.level.loader', 'iLoader') }
  sub CS_REQUEST_LEVELSAVER { CS_REQUEST_PLUGIN
	('crystalspace.level.saver', 'iSaver') }
  sub CS_REQUEST_REPORTER { CS_REQUEST_PLUGIN
	('crystalspace.utilities.reporter', 'iReporter') }
  sub CS_REQUEST_REPORTERLISTENER { CS_REQUEST_PLUGIN
	('crystalspace.utilities.stdrep', 'iReporterListener') }
  sub CS_REQUEST_CONSOLEOUT { CS_REQUEST_PLUGIN
	('crystalspace.console.output.standard', 'iConsoleOutput') }
%}
#endif // CS_MICRO_SWIG

/****************************************************************************
 * Workaround iPen::Rotate broken with Swig 1.3.28
 ****************************************************************************/
%perlcode %{
  *cspace::iPen::Rotate = *cspace::iPen::_Rotate;
%}
%extend iPen
{
  void _Rotate (float a)
  {
    self->Rotate (a);
  }
}

/****************************************************************************
 * Custom INPUT/OUTPUT/INOUT typemaps like those imported from typemaps.i,
 * since typemaps.i doesn't define any for strings.
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
  $result = sv_2mortal (newSVpv (* $1, 0));
  argvi++;
}
%typemap(in) char *& INOUT = char *& INPUT;
%typemap(argout) char *& INOUT = char *& OUTPUT;
%typemap(typecheck) char *& INPUT = char *;
%typemap(typecheck) char *& OUTPUT = char *;
%typemap(typecheck) char *& INOUT = char *;

/****************************************************************************
 * Extra pure perl operator overloads.
 ****************************************************************************/
%perlcode %{
  package cspace::iDataBuffer;
    use overload '""'	=> sub { $_[0]->GetData () },
		 'fallback' => 1;

  package cspace::iString;
    use overload '""'	=> sub { $_[0]->GetData () },
		 'fallback' => 1;

  package cspace::csString;
    use overload '""'	=> sub { $_[0]->GetData () },
		 'fallback' => 1;

  package cspace::csVector2;
    use overload '@{}'	=> sub { [ $_[0]->x, $_[0]->y ] },
		 'fallback' => 1;

  package cspace::csVector3;
    use overload '@{}'	=> sub { [ $_[0]->x, $_[0]->y, $_[0]->z ] },
		 'fallback' => 1;

  package cspace::csMatrix2;
    use overload '@{}'	=> sub { [ [ $_[0]->m11, $_[0]->m12 ],
				   [ $_[0]->m21, $_[0]->m22 ] ] },
		 'fallback' => 1;

  package cspace::csMatrix3;
    use overload '@{}'	=> sub { [ [ $_[0]->m11, $_[0]->m12, $_[0]->m13 ],
				   [ $_[0]->m21, $_[0]->m22, $_[0]->m23 ],
				   [ $_[0]->m31, $_[0]->m23, $_[0]->m33 ] ] },
		 'fallback' => 1;

  package cspace::csColor;
    use overload '@{}'	=> sub { [ $_[0]->red, $_[0]->green, $_[0]->blue ] },
		 'fallback' => 1;

  package cspace::csColor4;
    use overload '@{}'	=> sub { [ $_[0]->red, $_[0]->green, $_[0]->blue,
				   $_[0]->alpha ] },
		 'fallback' => 1;

  package cspace::csRGBcolor;
    use overload '@{}'	=> sub { [ $_[0]->red, $_[0]->green, $_[0]->blue ] },
		 'fallback' => 1;

  package cspace::csRGBpixel;
    use overload '@{}'	=> sub { [ $_[0]->red, $_[0]->green, $_[0]->blue,
				   $_[0]->alpha ] },
		 'fallback' => 1;

  package cspace::csSphere;
    use overload '+='	=> sub { $_[0]->Union
					($_[1]->GetCenter(),$_[1]->GetRadius());
				 $_[0]; },
		 'fallback' => 1;
%}

#if 0 // very experimental
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
#endif // 0

#endif // SWIGPERL5
