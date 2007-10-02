
/* REST */
/*
%ignore csInitializer::RequestPlugins(iObjectRegistry*, ...);
%ignore csInitializer::RequestPluginsV;
%rename (_RequestPlugins) csInitializer::RequestPlugins(iObjectRegistry*,
  csArray<csPluginRequest> const&);

%ignore csInitializer::SetupEventHandler (iObjectRegistry*, csEventHandlerFunc,
  const csEventID events[]);
%ignore csInitializer::SetupEventHandler (iObjectRegistry*, csEventHandlerFunc);
%rename (_SetupEventHandler) csInitializer::SetupEventHandler (iObjectRegistry*,
  iEventHandler *, const csEventID[]);
%typemap(default) const char * configName { $1 = 0; }

// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") csInitializer::SetupConfigManager;
%include "cstool/initapp.h"
%typemap(default) const char * configName;
*/
%ignore csColliderHelper::TraceBeam (iCollideSystem*, iSector*,
  const csVector3&, const csVector3&, bool, csIntersectingTriangle&,
  csVector3&, iMeshWrapper**);
%template(pycsColliderWrapper) scfImplementationExt1<csColliderWrapper,csObject,scfFakeInterface<csColliderWrapper> >;
%include "cstool/collider.h"
%include "cstool/csview.h"
%include "cstool/csfxscr.h"

%include "cstool/cspixmap.h"
%include "cstool/enginetools.h"

%include "cstool/genmeshbuilder.h"

/*%template(csPluginRequestArray) csArray<csPluginRequest>;*/
%ignore iPen::Rotate;

%include "cstool/pen.h"

%include "cstool/primitives.h"

/* pythpost */
/* work around broken Rotate function with swig 1.3.28 */
/*
%pythoncode %{
  def _csInitializer_RequestPlugins (reg, plugins):
    """Replacement of C++ version."""
    def _get_tuple (x):
      if callable(x):
        return tuple(x())
      else:
        return tuple(x)
    requests = csPluginRequestArray()
    for cls, intf, ident, ver in map(
        lambda x: _get_tuple(x), plugins):
      requests.Push(csPluginRequest(
        csString(cls), csString(intf), ident, ver))
    return csInitializer._RequestPlugins(reg, requests)

  csInitializer.RequestPlugins = staticmethod(_csInitializer_RequestPlugins)
%}
%pythoncode %{
  def _csInitializer_SetupEventHandler (reg, obj,
      eventids=None):
    """Replacement of C++ versions."""
    if callable(obj):
      # obj is a function
      hdlr = _EventHandlerFuncWrapper(obj)
      hdlr.thisown = 1
    else:
      # assume it is a iEventHandler
      hdlr = obj
    if eventids==None:
      eventids=[csevFrame(reg), csevInput(reg), csevKeyboardEvent(reg), \
                csevMouseEvent(reg), csevQuit(reg), CS_EVENTLIST_END]
    return csInitializer._SetupEventHandler(reg, hdlr, eventids)

  csInitializer.SetupEventHandler = \
    staticmethod(_csInitializer_SetupEventHandler)
%}

*/

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST CSTOOL_APPLY_FOR_EACH_INTERFACE
%include "bindings/common/basepost.i"
cs_lang_include(cstoolpost.i)
#endif
