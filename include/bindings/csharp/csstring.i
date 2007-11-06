// CS string typemaps 


//
// Base for the different string pointers
//
%define TYPEMAP_STRING_PTR_BASE(Type)
%typemap (out) (Type*)
{
  $result = $1->GetData();
}
%typemap (freearg) (Type*)
{
  delete $1;
}

%typemap (ctype) (Type*) "const char *"
%typemap (imtype, out="IntPtr") (Type*) "HandleRef"
%typemap (csin) (Type*) "new HandleRef(null, CrystalSpace.InteropServices.csArgsUtils.String2ASCII($csinput))"
%typemap (cstype) (Type*) "string"
%typemap (csout) (Type*)
{
  IntPtr _retptr = $imcall;
  return CrystalSpace.InteropServices.csArgsUtils.ASCII2String(_retptr);
}
%enddef

//
// Typemap for iString* and similar pointers
//
#undef TYPEMAP_STRING_PTR
%define TYPEMAP_STRING_PTR(Type, Type2)
//%ignore Type;
TYPEMAP_STRING_PTR_BASE(Type)

%typemap (in) (Type *)
{
  $1 = new Type2($input);
  // The input is allocated from the managed side, and it side expects that we
  //free it
  free((void*)$input);
}
%enddef

//
// Typemap for csString and similar
//
#undef TYPEMAP_STRING
%define TYPEMAP_STRING(Type)
//%ignore Type;
TYPEMAP_STRING_PTR_BASE(Type)

%typemap (in) (Type*)
{
  $1 = new $1_basetype($input);
  // The input is allocated from the managed side, and it side expects that we
  //free it
  free((void*)$input);
}

%typemap (out) Type
{
  $result = $1.GetData();
}

%typemap (in) Type
{
  $1 = Type($input);
  // The input is allocated from the managed side, and it side expects that we
  //free it
  free((void*)$input);
}

%typemap (ctype) Type "const char *"
%typemap (imtype, out="IntPtr") Type "HandleRef"
%typemap (cstype) Type  "string"
%typemap (csin) Type "new HandleRef(null, CrystalSpace.InteropServices.csArgsUtils.String2ASCII($csinput))"
%typemap (csout) Type
{
  IntPtr _retptr = $imcall;
  return CrystalSpace.InteropServices.csArgsUtils.ASCII2String(_retptr);
}


%enddef

