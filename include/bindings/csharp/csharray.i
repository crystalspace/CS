// We aren't supporting out args array, but we are supporting in args arrays

#undef TYPEMAP_IN_ARRAY_BODY
%define TYPEMAP_IN_ARRAY_BODY(array_type, cnt, ptr, to_item)
  cnt = $input.count;
  ptr = (array_type*)$input.array;
%enddef

#undef TYPEMAP_IN_ARRAY_CNT_PTR
%define TYPEMAP_IN_ARRAY_CNT_PTR(pattern, to_item)
  %typemap(in) pattern
  {
    TYPEMAP_IN_ARRAY_BODY($*2_type, $1,$2, to_item)
  }
  %typemap(ctype) pattern "csArrayPackData"
  %typemap(imtype) pattern "CrystalSpace.InteropServices.csArrayPackData"
  %typemap(cstype) pattern "$2_basetype[]"
  %typemap(csin) pattern "CrystalSpace.InteropServices.csArgsUtils.PackArrayData($csinput)"

  %typemap(freearg) pattern
  {
    if($input.free)
    {
      free((void*)$input.array);
    }
  }
%enddef

#undef TYPEMAP_IN_ARRAY_PTR_CNT
%define TYPEMAP_IN_ARRAY_PTR_CNT(pattern, to_item)
  %typemap(in) pattern
  {
    TYPEMAP_IN_ARRAY_BODY($*1_type, $2,$1, to_item)
  }

  %typemap(ctype) pattern "csArrayPackData"
  %typemap(imtype) pattern "CrystalSpace.InteropServices.csArrayPackData"
  %typemap(cstype) pattern "$1_basetype[]"
  %typemap(csin) pattern "CrystalSpace.InteropServices.csArgsUtils.PackArrayData($csinput)"

  %typemap(freearg) pattern
  {
    if($input.free)
    {
      free((void*)$input.array);
    }
  }
%enddef

