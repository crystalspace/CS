/*
 * Script override of many functions in cs dealing with querying
 * interfaces from different kinds of objects (plugins/iobjects/ibases).
 * By using (const char *iface, int iface_ver) typemap and returning
 * csWrapPtr we can do all these functions work "naturally", which means
 * we will get the object wrapped in its interface already without need
 * to do any kind of cast.
 *
 * Note languages that want to use these extends will need to define
 * a typecheck for (const char *iface, int iface_ver) to be able to cope
 * with overloaded methods correctly.
*/

/* iutil/object.h */
%extend iObject
{
  csWrapPtr GetChild( const char *iface, int iface_ver, const char *name=0 )
  {
    return csWrapPtr (iface, iface_ver, csRef<iBase> (
      self->GetChild(iSCF::SCF->GetInterfaceID (iface), iface_ver, name)));
  }
}

/* csutil/scf_interface.h */
%extend iBase
{
  csWrapPtr QueryInterface( const char *iface, int iface_ver )
  {
    if (self->QueryInterface(iSCF::SCF->GetInterfaceID(iface), iface_ver))
      return csWrapPtr (iface, iface_ver, csPtr<iBase> (self));
    else
      return csWrapPtr (iface, iface_ver, csPtr<iBase> (0));
  }
}

/* iutil/objreg.h */
%extend iObjectRegistry
{
  csWrapPtr Get(const char *iface, int iface_ver)
  {
    csPtr<iBase> b (self->Get(iface, iSCF::SCF->GetInterfaceID(iface), 
                                iface_ver));
    return csWrapPtr (iface, iface_ver, b);
  }
  csWrapPtr Get(const char *tag, const char *iface, int iface_ver)
  {
    csPtr<iBase> b (self->Get(tag, iSCF::SCF->GetInterfaceID(iface),
                                iface_ver));
    return csWrapPtr (iface, iface_ver, b);
  }
}

/* iutil/plugin.h */
%extend iPluginManager
{
  csWrapPtr LoadPlugin (const char *id, const char *iface, int iface_ver,
                        bool init = true, bool report = true)
  {
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (self->LoadPlugin (id,
                                init,report)));
  }
  csWrapPtr QueryPlugin(const char *id, const char *iface, int iface_ver)
  {
    return csWrapPtr (iface, iface_ver,
                csPtr<iBase> (self->QueryPlugin (id, iface, iface_ver)));
  }
  csWrapPtr QueryPlugin(const char *iface, int iface_ver)
  {
    return csWrapPtr (iface, iface_ver,
                csPtr<iBase> (self->QueryPlugin (iface, iface_ver)));
  }
}

/*
 * Old deprecated stuff
*/
// iutil/plugin.h
%inline
%{
  csPtr<iBase> CS_LOAD_PLUGIN_ALWAYS (iPluginManager *p, const char *i)
  {
    printf("CS_LOAD_PLUGIN_ALWAYS is deprecated, use \
csLoadPluginAlways instead\n");
    csRef<iComponent> c (csLoadPluginAlways(p,i));
    csRef<iBase> b ((iBase*)c);
    return csPtr<iBase> (b);
  }
%}

/* List Methods */
/****************************************************************************
 * These functions are replacements for CS's macros of the same names.
 * These functions can be wrapped by Swig but the macros can't.
 ****************************************************************************/

%inline %{
#undef SCF_QUERY_INTERFACE
#undef SCF_QUERY_INTERFACE_SAFE
#undef CS_QUERY_REGISTRY
#undef CS_QUERY_REGISTRY_TAG_INTERFACE
#undef CS_QUERY_PLUGIN_CLASS
#undef CS_LOAD_PLUGIN
#undef CS_GET_CHILD_OBJECT
#undef CS_GET_NAMED_CHILD_OBJECT
#undef CS_GET_FIRST_NAMED_CHILD_OBJECT

csWrapPtr CS_QUERY_REGISTRY (iObjectRegistry *reg, const char *iface,
  int iface_ver)
{
  printf("CS_QUERY_REGISTRY is deprecated, use \
iObjectRegistry->Get instead\n");
  csPtr<iBase> b (reg->Get(iface, iSCF::SCF->GetInterfaceID(iface), iface_ver));
  return csWrapPtr (iface, iface_ver, b);
}


csWrapPtr CS_QUERY_REGISTRY_TAG_INTERFACE (iObjectRegistry *reg,
  const char *tag, const char *iface, int iface_ver)
{
  printf("CS_QUERY_REGISTRY_TAG_INTERFACE is deprecated, use \
iObjectRegistry->Get instead\n");
  csPtr<iBase> b (reg->Get(tag, iSCF::SCF->GetInterfaceID(iface), iface_ver));
  return csWrapPtr (iface, iface_ver, b);
}

csWrapPtr SCF_QUERY_INTERFACE (iBase *obj, const char *iface, int iface_ver)
{
  // This call to QueryInterface ensures that IncRef is called and that
  // the object supports the interface.  However, for type safety and
  // object layout reasons the void pointer returned by QueryInterface
  // can't be wrapped inside the csWrapPtr so obj must be wrapped.
  printf("SCF_QUERY_INTERFACE is deprecated, use \
object->QueryInterface instead\n");
  if (obj->QueryInterface(iSCF::SCF->GetInterfaceID(iface), iface_ver))
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (obj));
  else
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (0));
}

csWrapPtr SCF_QUERY_INTERFACE_SAFE (iBase *obj, const char *iface,
  int iface_ver)
{
  printf("SCF_QUERY_INTERFACE_SAFE is deprecated, use \
object->QueryInterface instead\n");
  if (!obj)
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (0));

  // This call to QueryInterface ensures that IncRef is called and that
  // the object supports the interface.  However, for type safety and
  // object layout reasons the void pointer returned by QueryInterface
  // can't be wrapped inside the csWrapPtr so obj must be wrapped.
  if (obj->QueryInterface(iSCF::SCF->GetInterfaceID(iface), iface_ver))
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (obj));
  else
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (0));
}

csWrapPtr CS_QUERY_PLUGIN_CLASS (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  printf("CS_QUERY_PLUGIN_CLASS is deprecated, use \
iPluginManager->QueryPlugin instead\n");
  return csWrapPtr (iface, iface_ver,
    csPtr<iBase> (obj->QueryPlugin (id, iface, iface_ver)));
}

csWrapPtr CS_LOAD_PLUGIN (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  printf("CS_LOAD_PLUGIN is deprecated, use \
iPluginManager->LoadPlugin instead\n");
  return csWrapPtr (iface, iface_ver, csPtr<iBase> (obj->LoadPlugin (id)));
}

csWrapPtr CS_GET_CHILD_OBJECT (iObject *obj, const char *iface, int iface_ver)
{
  printf("CS_GET_CHILD_OBJECT is deprecated, use \
object->GetChild instead\n");
  return csWrapPtr (iface, iface_ver, csRef<iBase> (
    obj->GetChild(iSCF::SCF->GetInterfaceID (iface), iface_ver)));
}

csWrapPtr CS_GET_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  printf("CS_GET_NAMED_CHILD_OBJECT is deprecated, use \
object->GetChild instead\n");
  return csWrapPtr (iface, iface_ver, csRef<iBase> (
    obj->GetChild(iSCF::SCF->GetInterfaceID (iface), iface_ver, name)));
}

csWrapPtr CS_GET_FIRST_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  printf("CS_GET_FIRST_NAMED_CHILD_OBJECT is deprecated, use \
object->GetChild instead\n");
  return csWrapPtr (iface, iface_ver, csRef<iBase> (
    obj->GetChild(iSCF::SCF->GetInterfaceID (iface), iface_ver, name, true)));
}
%}
