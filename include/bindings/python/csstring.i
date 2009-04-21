/* CS String Typemaps */

/*
 * Base for csString* typemaps. Note (in) typemap cant be declared here
 * because it works differently for virtual and non virtual types.
*/
%define TYPEMAP_STRING_PTR_BASE(Type)
%typemap(out) Type *
{
        const char *res = $1->GetData();
        $result = PyString_FromString(res);
}

%typecheck(SWIG_TYPECHECK_INT8) Type *
{
  $1 = PyString_Check($input);
}

%typemap(freearg) Type *
{
   delete $1;
}
%enddef

/*
 * Typemap for iString and similar 
*/
#undef TYPEMAP_STRING_PTR
%define TYPEMAP_STRING_PTR(Type,Type2)
%ignore Type;
TYPEMAP_STRING_PTR_BASE(Type)
%typemap(in) Type *
{
        $1 = new Type2 (PyString_AsString($input));
}
%enddef

/*
 * Typemap for csString and similar 
*/
#undef TYPEMAP_STRING
%define TYPEMAP_STRING(Type)
%ignore Type;
TYPEMAP_STRING_PTR_BASE(Type)
%typemap(out) Type
{
        const char *res = $1.GetData();
        $result = PyString_FromString(res);
}

%typemap(in) Type
{
        $1 = PyString_AsString($input);
}

%typecheck(SWIG_TYPECHECK_INT8) Type
{
  $1 = PyString_Check($input);
}
%typemap(in) Type *
{
        $1 = new $1_basetype(PyString_AsString($input));
}
%enddef
