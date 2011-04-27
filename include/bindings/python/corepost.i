/*
  Copyright (C) 2003 Rene Jager <renej_frog@users.sourceforge.net>

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

/*
  Python specific stuff for SWIG interface in post-include phase.
  See include/bindings/cspace.i
*/

%include "pyeventh.i"
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
        cls, intf, ident, ver))
    return csInitializer._RequestPlugins(reg, requests)

  csInitializer.RequestPlugins = staticmethod(_csInitializer_RequestPlugins)

  def _csInitializer_CreateEnvironment (*args):
    oreg = csInitializer._CreateEnvironment(*args)
    SyncSCFPointers()
    return oreg
  csInitializer.CreateEnvironment = staticmethod(_csInitializer_CreateEnvironment)

  def _csInitializer_InitializeSCF (*args):
    res = _core.csInitializer__InitializeSCF(*args)
    SyncSCFPointers()
    return res
  csInitializer.InitializeSCF = staticmethod(_csInitializer_CreateEnvironment)

%}

%pythoncode %{
  corecvar = cvar
  def CS_REQUEST_PLUGIN (name, intf):
    return (name, intf.__name__, 
       corecvar.iSCF_SCF.GetInterfaceID(intf.__name__),intf.scfGetVersion())
  def CS_REQUEST_PLUGIN_TAG (name, intf, tag):
    return (name+":"+tag, intf.__name__, 
       corecvar.iSCF_SCF.GetInterfaceID(intf.__name__),intf.scfGetVersion())
  def CS_REQUEST_VFS ():
    return CS_REQUEST_PLUGIN("crystalspace.kernel.vfs", iVFS)
  def CS_REQUEST_LEVELSAVER ():
    return CS_REQUEST_PLUGIN("crystalspace.level.saver", iSaver)
  def CS_REQUEST_REPORTER ():
    return CS_REQUEST_PLUGIN("crystalspace.utilities.reporter", iReporter)
%}

%extend iBase {
  %pythoncode %{
      def __eq__(self,other):
          if isinstance(other,iBase):
              return self.this == other.this
          return False
      def __ne__(self,other):
          if isinstance(other,iBase):
              return not self.this == other.this
          return True
    %}
}

%extend csKeyModifiers {
  unsigned int __getitem__ (size_t i) const
  {
      if (i<csKeyModifierTypeLast)
          return self->modifiers[i];
      return 0;
  }
}

%extend csColor
{
  csColor __rmul__ (float f) const { return f * *self; }
}

/*
 * We introduce the following python class to serve as a pseudo-list for
 *  those cases where the C++ code returns a pointer which is actually
 *  a pointer to an array.  Of course, in order for this to work, we
 *  must use the shadow feature to override the default wrapper code
 *  that is generated.  Such features must go in pythpre.i, placing them
 *  here is too late.
 */
%pythoncode %{
class CSMutableArrayHelper:
  def __init__(self, getFunc, lenFunc):
    self.getFunc = getFunc
    self.lenFunc = lenFunc

  def __len__(self):
    return self.lenFunc()

  def __getitem__(self, key):
    if type(key) != type(0):
      raise TypeError()
    arrlen = self.lenFunc()
    if key < 0 or key >= arrlen:
      raise IndexError('Length is ' + str(arrlen) + ', you asked for ' +
        str(key))
    return self.getFunc(key)
  def content_iterator(self):
    for idx in xrange(len(self)):
      yield self.__getitem__(idx)
  def __iter__(self): return self.content_iterator() 
  # We do not implement __setitem__ because the only legal action is to
  #  overwrite the object at the given location.  (The contents of the
  #  array are mutable, but the array is a single allocation of a single
  #  type.)  Callers should be using the contained objects' own
  #  __setitem__ or mutation methods.

  # We do not implement __delitem__ because we cannot delete items.
%}

// iutil/event.h
// the property doesnt work well by itself otherwise.
%extend iEvent
{
  %pythoncode %{
  Name = property(GetName)
  %}
}

// csutil/hash.h
%extend csHash
{
  const T& __getitem__(const K& key) { return self->Get(key,T()); }
  bool __delitem__(const K& key)
  { return self->DeleteAll(key); }
  void clear() { self->Empty(); }
  bool __nonzero__ () { return self->IsEmpty(); }
  void __setitem__(const K& key, const T &value) 
  { self->PutUnique(key,value); }
  size_t __len__() { return self->GetSize(); }
}

//csutil/scf_implementation.h
%extend scfInterfaceMetadataList
{
  size_t __len__() { return self->metadataCount; }
  scfInterfaceMetadata __getitem__ (size_t i) const { return self->metadata[i]; }
}
PYITERATOR_PROTOCOL(scfInterfaceMetadataList)

%pythoncode %{
  csReport = csReporterHelper.Report
  _csmodules = []
  def AddSCFLink(csmodule):
      """Add a callback to set SCF pointer from a module"""
      _csmodules.append(csmodule)
      csmodule(corecvar.iSCF_SCF)
  def SyncSCFPointers():
      """Sync the SCF pointers for all registered modules"""
      for csmodule in _csmodules:
          csmodule(corecvar.iSCF_SCF)
  def SetSCFPointer(scf_pointer):
      """Set SCF Pointer to all CrystalSpace modules"""
      SetCoreSCFPointer(scf_pointer)
      SyncSCFPointers()
  def GetSCFPointer():
      """Get SCF Pointer"""
      return corecvar.iSCF_SCF
%}

/*
 csWrapTypedObject is used to wrap a c++ object and pass it around as a Python 
 object. As an example think of passing the iObjectRegistry* from the main c++
 program to your python code.
*/

%{

#ifdef __cplusplus
extern "C" {
#endif

// Not `static' because it is published for use by other clients in same DLL.
CS_EXPORT_SYM PyObject* csWrapTypedObject(void* objectptr, const char *typetag,
  int own)
{
  swig_type_info *ti = SWIG_TypeQuery (typetag);
  PyObject *obj = SWIG_NewPointerObj (objectptr, ti, own);
  return obj;
}
#ifdef __cplusplus
}
#endif

%}

