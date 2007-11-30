
%define INTERFACE_MODIFIERS(T)
%csmethodmodifiers T::scfGetVersion() "public new";
%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(T) INTERFACE_MODIFIERS(T)

APPLY_FOR_ALL_INTERFACES

%csmethodmodifiers iBase::scfGetVersion() "public";

%csmethodmodifiers csCollisionPairArrayChangeElements::Get() "public override";
%csmethodmodifiers csCollisionPairArrayChangeElements::Top() "public override";
%csmethodmodifiers csShaderVariableArrayChangeElements::Get() "public override";
%csmethodmodifiers csShaderVariableArrayChangeElements::Top() "public override";
%csmethodmodifiers csSprite2DVertexArrayChangeElements::Get() "public override";
%csmethodmodifiers csSprite2DVertexArrayChangeElements::Top() "public override";
%csmethodmodifiers csVector3ArrayChangeElements::Get() "public override";
%csmethodmodifiers csVector3ArrayChangeElements::Top() "public override";

%csmethodmodifiers csTriangleMesh::GetTriangleCount() "public override";

