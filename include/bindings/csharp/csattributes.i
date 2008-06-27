
%define INTERFACE_MODIFIERS(T)
%csmethodmodifiers T::scfGetVersion() "public new";
%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(T) INTERFACE_MODIFIERS(T)

APPLY_FOR_ALL_INTERFACES

%csmethodmodifiers iBase::scfGetVersion() "public";

%csmethodmodifiers csPath::Length() "public virtual";
%csmethodmodifiers csPath::QueryObject() "public virtual";
%csmethodmodifiers csPath::CalculateAtTime(float) "public virtual";
%csmethodmodifiers csPath::GetCurrentIndex() "public virtual";
%csmethodmodifiers csPath::GetTime(int) "public virtual";
%csmethodmodifiers csPath::SetTime(int, float) "public virtual";
%csmethodmodifiers csPath::SetTimes(float *) "public virtual";
%csmethodmodifiers csPath::SetPositionVectors(csVector3 *) "public virtual";
%csmethodmodifiers csPath::SetUpVectors(csVector3 *) "public virtual";
%csmethodmodifiers csPath::SetForwardVectors(csVector3 *) "public virtual";
%csmethodmodifiers csPath::SetPositionVector(int, const csVector3 &) "public virtual";
%csmethodmodifiers csPath::SetUpVector(int, const csVector3 &) "public virtual";
%csmethodmodifiers csPath::SetForwardVector(int, const csVector3 &) "public virtual";
%csmethodmodifiers csPath::GetPositionVector(int, csVector3 &) "public virtual";
%csmethodmodifiers csPath::GetUpVector(int, csVector3 &) "public virtual";
%csmethodmodifiers csPath::GetForwardVector(int, csVector3 &) "public virtual";
%csmethodmodifiers csPath::GetInterpolatedPosition(csVector3 &) "public virtual";
%csmethodmodifiers csPath::GetInterpolatedUp(csVector3 &) "public virtual";
%csmethodmodifiers csPath::GetInterpolatedForward(csVector3 &) "public virtual";

%csmethodmodifiers csCollisionPairArrayChangeElements::Get() "public override";
%csmethodmodifiers csCollisionPairArrayChangeElements::Top() "public override";
%csmethodmodifiers csShaderVariableArrayChangeElements::Get() "public override";
%csmethodmodifiers csShaderVariableArrayChangeElements::Top() "public override";
%csmethodmodifiers csSprite2DVertexArrayChangeElements::Get() "public override";
%csmethodmodifiers csSprite2DVertexArrayChangeElements::Top() "public override";
%csmethodmodifiers csVector3ArrayChangeElements::Get() "public override";
%csmethodmodifiers csVector3ArrayChangeElements::Top() "public override";

%csmethodmodifiers csTriangleMesh::GetTriangleCount() "public override";

