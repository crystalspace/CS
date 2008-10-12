#undef INTERFACE_BASE
%define INTERFACE_BASE
	public boolean equals (Object obj)
	{
		if (obj == this) return true;
		boolean equal = false;
		if (obj instanceof $javaclassname)
			equal = ((($javaclassname)obj).swigCPtr == this.swigCPtr);
		return equal;
	}

	public long getPointer() 
	{
		return this.swigCPtr;
	}
%enddef

#undef INTERFACE_APPLY
%define INTERFACE_APPLY(T) 
%typemap(javacode) T
%{
	INTERFACE_BASE 
%}
%enddef

APPLY_FOR_ALL_INTERFACES

#undef INTERFACE_APPLY
