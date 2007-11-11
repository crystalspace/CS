
%define INTERFACE_MODIFIERS(T)
%csmethodmodifiers T::scfGetVersion() "public new";
%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(T) INTERFACE_MODIFIERS(T)

APPLY_FOR_ALL_INTERFACES

%csmethodmodifiers iBase::scfGetVersion() "public";


