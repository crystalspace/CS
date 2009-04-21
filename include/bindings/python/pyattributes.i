/* Macros used to generate the right set of accessor methods to transform
 * getters/setters into Python attributes.
 *
 * Class - the class name
 * type - the type of the variable
 * name - the name you wish to use in the python bindings
 * setmethod - the set method in Class
 * getmethod - the get method in Class
 * cs_attribute: Create a getter/setter. Both setmethod and getmethod are
    optional, if none is specified then Get and Set will be prepended to the
    attribute name to find the functions. If only setter is specified then
    the attribute will be read only. If both are specified then theyll be used.
 * cs_attribute_writeonly: As before method for attributes that are write only.
 * cs_multi_attr: For multiple arguments setter/getters.
 * cs_multi_attr_writeonly: For multiple arguments setter only.
 */

/* Helper pythoncode
 * All modules that want to use property macros in this file have to
 * declare CS_PROPERTY_HELPERS
 */

%define CS_PROPERTY_HELPERS

%pythoncode %{
def fix_args(funct):
    def _inner(self, args):
        if type(args) == tuple:
            args = (self,) + args
        else:
            args = (self, args)
        return funct(*args)
    return _inner
%}
%enddef

CS_PROPERTY_HELPERS

/* Read Write implementation */
%define %cs_attribute_impl(Module,Class, type, name, getmethod, ...)
#if #__VA_ARGS__ != ""
%feature("shadow") Class::name##_set {
  name = _swig_property(_##Module##.##Class##_##name##_get, _##Module##.##Class##_##name##_set, None,
                  "Class.name -> type\n\nThis is equivalent to calling the C++ cs methods:\n\tget: type Class::getmethod()\n\tset: void Class::__VA_ARGS__(type)")};
%feature("shadow") Class::name##_get {}
%extend Class
{
    void name##_set(type _val) { self->__VA_ARGS__(_val); }
}

#else
%feature("shadow") Class::name##_get {
  name = _swig_property(_##Module##.##Class##_##name##_get, None, None,
                  "Class.name -> type  (read-only)\n\nThis is equivalent to calling the C++ cs method:\n\tget: type Class::getmethod()")};
#endif
%extend Class
{
    type name##_get() { return (type)(self->getmethod()); }
}
%enddef

/* cs_attribute calling macro */
#undef %cs_attribute
%define %cs_attribute(Module,Class, type, name, ...)
#if #__VA_ARGS__ != ""
%cs_attribute_impl(Module, Class, type, name, __VA_ARGS__)
#else
%cs_attribute_impl(Module, Class, type, name, Get##name, Set##name)
#endif
%enddef

/* cs_attribute_writeonly calling macro */
%define %cs_attribute_writeonly(Module, Class, type, name, setmethod)
%feature("shadow") Class::name##_set {
  name = _swig_property(None, _##Module##.##Class##_##name##_set, None,
                  "Class.name (write only) -> type\n\nWriting to this is equivalent to calling the C++ cel method:\n\tvoid Class::setmethod(type)")};
%feature("shadow") Class::name##_get {}
%extend Class
{
    void name##_set(type _val) { self->setmethod (_val); }
}
%enddef

/* cs_multi_attr calling macro */
%define %cs_multi_attr(Module, Class, name, getmethod, setmethod)
%extend Class
{
%pythoncode %{
  name = _swig_property(_##Module##.##Class##_##getmethod, fix_args(_##Module##.##Class##_##setmethod), None,
                  "Class.name -> type\n\nThis is equivalent to calling the C++ cs methods:\n\tget: Class::getmethod()\n\tset: void Class::setmethod(...)")
%}
}
%enddef

/* cs_multi_attr_readonly calling macro */
%define %cs_multi_attr_readonly(Module, Class, name, getmethod)
%extend Class
{
%pythoncode %{
  name = _swig_property(_##Module##.##Class##_##getmethod, None, None,
                  "Class.name -> type\n\nThis is equivalent to calling the C++ cs method:\n\tget: Class::getmethod()")
%}
}
%enddef

/* cs_multi_attr_writeonly calling macro */
%define %cs_multi_attr_writeonly(Module, Class, name,  setmethod)
%extend Class
{
%pythoncode %{
  name = _swig_property(None, fix_args(_##Module##.##Class##_##setmethod), None,
                  "Class.name -> type\n\nThis is equivalent to calling the C++ cs methods:\n\tget: Class::getmethod()\n\tset: void Class::setmethod(...)")
%}
}
%enddef


