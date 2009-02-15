%{
	typedef unsigned int StringIDValue;
%}

#undef TYPEMAP_CSEVENTID_ARRAY
%define TYPEMAP_CSEVENTID_ARRAY
%typemap(in) csEventID[]
{
	jlongArray longEventsArray = (jlongArray)$input;
	jlong * larray = JCALL2(GetLongArrayElements,jenv,longEventsArray,0);
	int totalElem = JCALL1(GetArrayLength,jenv,longEventsArray);
	csEventID * csEvents = new csEventID[totalElem];
	for (int j=0;j<totalElem;j++) 
	{
		csEvents[j] = csEventID(larray[j]);
	}
	JCALL3(ReleaseLongArrayElements,jenv,longEventsArray,larray,JNI_ABORT);
	$1 = csEvents;
}
%typemap(jni) csEventID[] "jlongArray"
%typemap(jtype) csEventID[] "long[]"
%typemap(jstype) csEventID[] "long[]"
%typemap(javain) csEventID[] "$javainput"
%typemap(javaout) csEventID[] { return $jnicall; }
%typemap(freearg) csEventID[]
{
  delete[]($1);
}
%enddef
TYPEMAP_CSEVENTID_ARRAY
#undef TYPEMAP_CSEVENTID_ARRAY

#undef TYPEMAP_CSEVENTID
%define TYPEMAP_CSEVENTID
%typemap(in) csEventID
{
	csEventID csEvent = csEventID((long)$input);
	$1 = csEvent;
}
%typemap(out) csEventID
{
	$result = (jlong)(StringIDValue)(csEventID)$1;
}
%typemap(jni) csEventID "jlong"
%typemap(jtype) csEventID "long"
%typemap(jstype) csEventID "long"
%typemap(javain) csEventID "$javainput"
%typemap(javaout) csEventID { return $jnicall; }
%enddef
TYPEMAP_CSEVENTID
#undef TYPEMAP_CSEVENTID

// Typemap to handle csRef<xxx> & OUTPUT parameters
#undef OUTPUT_TYPEMAP_CSREF
%define OUTPUT_TYPEMAP_CSREF(type)
	%typemap(jni) csRef<type> &OUTPUT "jobjectArray"
	%typemap(jtype) csRef<type> &OUTPUT "type[]"
	%typemap(jstype) csRef<type> &OUTPUT "type[]"
	%typemap(javain) csRef<type> &OUTPUT "$javainput"
	%typemap(in) csRef<type> &OUTPUT($*1_ltype temp)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$1 = &temp;
	}
	%typemap(argout) csRef<type> &OUTPUT 
	{
		JCALL3(SetObjectArrayElement, jenv, $input, 0, _csRef_to_Java(csRef<iBase>(
		(type*)temp$argnum), (void*)(type*)temp$argnum, #type " *", "org/crystalspace3d/" #type, jenv));
	}
%enddef
#undef INTERFACE_APPLY
%define INTERFACE_APPLY(T) 
	OUTPUT_TYPEMAP_CSREF(T) 
%enddef
APPLY_FOR_ALL_INTERFACES
#undef INTERFACE_APPLY
#undef OUTPUT_TYPEMAP_CSREF

#undef OUTPUT_TYPEMAP_STRING
%define OUTPUT_TYPEMAP_STRING
	%typemap(jni) char *OUTPUT "jobjectArray"
	%typemap(jtype) char *OUTPUT "String[]"
	%typemap(jstype) char *OUTPUT "String[]"
	%typemap(javain) char *OUTPUT "$javainput"
	%typemap(in) char *OUTPUT($*1_ltype temp)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$1 = &temp;
	}
	%typemap(argout) char *OUTPUT
	{
		JCALL3(SetObjectArrayElement, jenv, $input, 0, JCALL1(NewStringUTF, jenv, temp$argnum));
	}
	%typemap(freearg) char *OUTPUT ""
%enddef
OUTPUT_TYPEMAP_STRING
#undef OUTPUT_TYPEMAP_STRING

#undef INOUT_TYPEMAP_STRING
%define INOUT_TYPEMAP_STRING
	%typemap(jni) char *INOUT "jobjectArray"
	%typemap(jtype) char *INOUT "String[]"
	%typemap(jstype) char *INOUT "String[]"
	%typemap(javain) char *INOUT "$javainput"
	%typemap(in) char *INOUT($*1_ltype temp,jstring theString)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		theString = (jstring)JCALL2(GetObjectArrayElement,jenv,$input,0);
		if (theString) {
			temp = (char *)(jbyte *)JCALL2(GetStringUTFChars,jenv,theString,0);
		}
		$1 = &temp;
	}
	%typemap(argout) char *INOUT
	{
		JCALL3(SetObjectArrayElement, jenv, $input, 0, JCALL1(NewStringUTF, jenv, temp$argnum));
		
	}
	%typemap(freearg) char *INOUT 
	{
		if (theString$argnum) {
			jenv->ReleaseStringUTFChars(theString$argnum,temp$argnum);
		}
	}
%enddef
INOUT_TYPEMAP_STRING
#undef INOUT_TYPEMAP_STRING

#undef INPUT_TYPEMAP_CSTYPE_ARRAY
%define INPUT_TYPEMAP_CSTYPE_ARRAY(type,size_type)
	%typemap(jni) (type *INPUT,size_type INPUT) "jobjectArray"
	%typemap(jtype) (type *INPUT,size_type INPUT) "long[]"
	%typemap(jstype) (type *INPUT,size_type INPUT) "type[]"
	%typemap(javain) (type *INPUT,size_type INPUT) "cspaceUtils._ConvertArrayToNative($javainput)"
	%typemap(in) (type *INPUT,size_type INPUT) (type ** temp,size_type size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		size = JCALL1(GetArrayLength, jenv, $input);
		if (size == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		jlongArray jlarray = (jlongArray)$input;
		jlong * larray = JCALL2(GetLongArrayElements,jenv,jlarray,0);
		temp = new type*[size];
		for (size_type i=0;i<size;i++) {
			temp[i] = (type*)(void*)(long)larray[i];
		}
		JCALL3(ReleaseLongArrayElements,jenv,jlarray,larray,JNI_ABORT);
		$1 = *temp;
		$2 = size;
	}
	%typemap(freearg) (type *INPUT,size_type INPUT)
	{
		delete[](temp$argnum);
	}
%enddef
INPUT_TYPEMAP_CSTYPE_ARRAY(csTriangleMinMax,size_t)
INPUT_TYPEMAP_CSTYPE_ARRAY(csTriangleMeshEdge,size_t) 
INPUT_TYPEMAP_CSTYPE_ARRAY(csVector2,int) 
INPUT_TYPEMAP_CSTYPE_ARRAY(csPlane3,size_t) 
#undef INPUT_TYPEMAP_CSTYPE_ARRAY


#undef OUTPUT_TYPEMAP_CSTYPE_ARRAY
%define OUTPUT_TYPEMAP_CSTYPE_ARRAY(type)
	%typemap(jni) (type *&OUTPUT,size_t &OUTPUT) "jobjectArray"
	%typemap(jtype) (type *&OUTPUT,size_t &OUTPUT) "type[][]"
	%typemap(jstype) (type *&OUTPUT,size_t &OUTPUT) "type[][]"
	%typemap(javain) (type *&OUTPUT,size_t &OUTPUT) "$javainput"
	%typemap(in) (type *&OUTPUT,size_t &OUTPUT) (type * temp,size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$1 = (type**)&temp;
		$2 = &size;
	}
	%typemap(argout) (type *&OUTPUT,size_t &OUTPUT)
	{
		jclass clazz = jenv->FindClass("org/crystalspace3d/" #type);
		jmethodID mid = jenv->GetMethodID(clazz, "<init>", "(JZ)V");
		jobjectArray oarray = JCALL3(NewObjectArray, jenv, (long)size$argnum, clazz, 0);
		for (unsigned int i = 0;i<size$argnum;i++) {
			JCALL3(SetObjectArrayElement, jenv, oarray, i, jenv->NewObject(clazz, mid, (jlong)(void*)(temp$argnum+i), true));
		}
		JCALL3(SetObjectArrayElement, jenv, $input, 0, oarray);
	}
	%typemap(freearg) (void *&OUTPUT,size_t &OUTPUT) ""
%enddef
OUTPUT_TYPEMAP_CSTYPE_ARRAY(csTriangleMinMax)
#undef OUTPUT_TYPEMAP_CSTYPE_ARRAY

#undef INPUT_TYPEMAP_CSTYPE_ARRAY
%define INPUT_TYPEMAP_CSTYPE_ARRAY(type,size_type)
	%typemap(jni) (type *INPUT,size_type &INPUT) "jobjectArray"
	%typemap(jtype) (type *INPUT,size_type &INPUT) "long[]"
	%typemap(jstype) (type *INPUT,size_type &INPUT) "type[]"
	%typemap(javain) (type *INPUT,size_type &INPUT) "cspaceUtils._ConvertArrayToNative($javainput)"
	%typemap(in) (type *INPUT,size_type &INPUT) (type ** temp,size_type size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		size = JCALL1(GetArrayLength, jenv, $input);
		if (size == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		jlongArray jlarray = (jlongArray)$input;
		jlong * larray = JCALL2(GetLongArrayElements,jenv,jlarray,0);
		temp = new type*[size];
		for (size_type i=0;i<size;i++) {
			temp[i] = (type*)(void*)(long)larray[i];
		}
		JCALL3(ReleaseLongArrayElements,jenv,jlarray,larray,JNI_ABORT);
		$1 = *temp;
		$2 = &size;
	}
	%typemap(freearg) (type *INPUT,size_type &INPUT)
	{
		delete[](temp$argnum);
	}
%enddef
INPUT_TYPEMAP_CSTYPE_ARRAY(iTriangleMesh,size_t) 
INPUT_TYPEMAP_CSTYPE_ARRAY(csVector3,int) 
#undef INPUT_TYPEMAP_CSTYPE_ARRAY

%define OUTPUT_TYPEMAP_CSTYPE_ARRAY(type)
	%typemap(jni) (type *OUTPUT,size_t &OUTPUT) "jobjectArray"
	%typemap(jtype) (type *OUTPUT,size_t &OUTPUT) "type[][]"
	%typemap(jstype) (type *OUTPUT,size_t &OUTPUT) "type[][]"
	%typemap(javain) (type *OUTPUT,size_t &OUTPUT) "$javainput"
	%typemap(in) (type *OUTPUT,size_t &OUTPUT) (size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		size = JCALL1(GetArrayLength, jenv, $input);
		if (size == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$1 = 0;
		$2 = &size;
	}
	%typemap(argout) (type *OUTPUT,size_t &OUTPUT)
	{
		jclass clazz = jenv->FindClass("org/crystalspace3d/" #type);
		jmethodID mid = jenv->GetMethodID(clazz, "<init>", "(JZ)V");
		jobjectArray oarray = JCALL3(NewObjectArray, jenv, (long)size$argnum, clazz, 0);
		for (unsigned int i = 0;i<size$argnum;i++) {
			JCALL3(SetObjectArrayElement, jenv, oarray, i, jenv->NewObject(clazz, mid, (jlong)(void*)($1+i), true));
		}
		JCALL3(SetObjectArrayElement, jenv, $input, 0, oarray);
	}
	%typemap(freearg) (type *OUTPUT,size_t &OUTPUT) ""
%enddef
OUTPUT_TYPEMAP_CSTYPE_ARRAY(csTriangleMeshEdge) 
#undef OUTPUT_TYPEMAP_CSTYPE_ARRAY

%define OUTPUT_TYPEMAP_CSTYPE_ARRAY(type)
	%typemap(jni) (type *OUTPUT) "jobjectArray"
	%typemap(jtype) (type *OUTPUT) "type[]"
	%typemap(jstype) (type *OUTPUT) "type[]"
	%typemap(javain) (type *OUTPUT) "$javainput"
	%typemap(in) (type *OUTPUT) (size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		size = JCALL1(GetArrayLength, jenv, $input);
		if (size == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$1 = new type[size];
		for (unsigned int i=0;i<size;i++) {
			jobject obj = JCALL2(GetObjectArrayElement,jenv,$input,i);
			if (obj) {
				$1[i] = *((type*)(void*)_cs_get_swig_pointer(obj,jenv));
			}
		}
	}
	%typemap(argout) (type *OUTPUT)
	{
		jclass clazz = jenv->FindClass("org/crystalspace3d/" #type);
		jmethodID mid = jenv->GetMethodID(clazz, "<init>", "(JZ)V");
		for (unsigned int i = 0;i<size$argnum;i++) {
			JCALL3(SetObjectArrayElement, jenv, $input, i, jenv->NewObject(clazz, mid, (jlong)(void*)($1+i), true));
		}
		delete[]($1);
	}
%enddef
OUTPUT_TYPEMAP_CSTYPE_ARRAY(csPlane3) 
OUTPUT_TYPEMAP_CSTYPE_ARRAY(csVector3) 
#undef OUTPUT_TYPEMAP_CSTYPE_ARRAY

#undef OUTPUT_TYPEMAP_VOIDP
%define OUTPUT_TYPEMAP_VOIDP
	%typemap(jni) (void *&OUTPUT,size_t &OUTPUT) "jobjectArray"
	%typemap(jtype) (void *&OUTPUT,size_t &OUTPUT) "byte[][]"
	%typemap(jstype) (void *&OUTPUT,size_t &OUTPUT) "byte[][]"
	%typemap(javain) (void *&OUTPUT,size_t &OUTPUT) "$javainput"
	%typemap(in) (void *&OUTPUT,size_t &OUTPUT) (char * temp,size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$1 = (void**)&temp;
		$2 = &size;
	}
	%typemap(argout) (void *&OUTPUT,size_t &OUTPUT)
	{
		jbyteArray barray = JCALL1(NewByteArray, jenv, (long)size$argnum);
		jbyte * pbarray = JCALL2(GetByteArrayElements,jenv,barray,0);
		memcpy(pbarray,temp$argnum,size$argnum);
		JCALL3(ReleaseByteArrayElements, jenv, barray, pbarray, JNI_COMMIT);
		JCALL3(SetObjectArrayElement, jenv, $input, 0, barray);
	}
	%typemap(freearg) (void *&OUTPUT,size_t &OUTPUT) ""
%enddef
OUTPUT_TYPEMAP_VOIDP
#undef OUTPUT_TYPEMAP_VOIDP

#undef OUTPUT_TYPEMAP_TYPEP
%define OUTPUT_TYPEMAP_TYPEP(type)
	%typemap(jni) (type *&OUTPUT,size_t &OUTPUT) "jobjectArray"
	%typemap(jtype) (type *&OUTPUT,size_t &OUTPUT) "type[][]"
	%typemap(jstype) (type *&OUTPUT,size_t &OUTPUT) "type[][]"
	%typemap(javain) (type *&OUTPUT,size_t &OUTPUT) "$javainput"
	%typemap(in) (type *&OUTPUT,size_t &OUTPUT) (type * temp,size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$1 = (type**)&temp;
		$2 = &size;
	}
	%typemap(argout) (type *&OUTPUT,size_t &OUTPUT)
	{
		jclass clazz = jenv->FindClass("org/crystalspace3d/" #type);
		jmethodID mid = jenv->GetMethodID(clazz, "<init>", "(JZ)V");
		jobjectArray oarray = JCALL3(NewObjectArray, jenv, (long)size$argnum, clazz, 0);
		for (unsigned int i = 0;i<size$argnum;i++) {
			JCALL3(SetObjectArrayElement, jenv, oarray, i, jenv->NewObject(clazz, mid, (jlong)(void*)($1+i), true));
		}
		JCALL3(SetObjectArrayElement, jenv, $input, 0, oarray);
	}
	%typemap(freearg) (type *&OUTPUT,size_t &OUTPUT) ""
%enddef
OUTPUT_TYPEMAP_TYPEP(csTriangleMinMax)
OUTPUT_TYPEMAP_TYPEP(csPlane3)
#undef OUTPUT_TYPEMAP_TYPEP


#undef OUTPUT_TYPEMAP_BOOL_P
%define OUTPUT_TYPEMAP_BOOL_P
	%typemap(jni) (bool * OUTPUT,size_t &OUTPUT) "jobjectArray"
	%typemap(jtype) (bool * OUTPUT,size_t &OUTPUT) "boolean[][]"
	%typemap(jstype) (bool * OUTPUT,size_t &OUTPUT) "boolean[][]"
	%typemap(javain) (bool * OUTPUT,size_t &OUTPUT) "$javainput"
	%typemap(in) (bool * OUTPUT,size_t &OUTPUT) (size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$2 = &size;
	}
	%typemap(argout) (bool * OUTPUT,size_t &OUTPUT)
	{
		jbooleanArray barray = JCALL1(NewBooleanArray, jenv, (long)size$argnum);
		jboolean * pbarray = JCALL2(GetBooleanArrayElements,jenv,barray,0);
		memcpy(pbarray,$1,size$argnum);
		JCALL3(ReleaseBooleanArrayElements, jenv, barray, pbarray, JNI_COMMIT);
		JCALL3(SetObjectArrayElement, jenv, $input, 0, barray);
	}
	%typemap(freearg) (bool * OUTPUT,size_t &OUTPUT) ""
%enddef
OUTPUT_TYPEMAP_BOOL_P
#undef OUTPUT_TYPEMAP_BOOL_P

#undef OUTPUT_TYPEMAP_SIZET_P
%define OUTPUT_TYPEMAP_SIZET_P
	%typemap(jni) (size_t * OUTPUT,size_t &OUTPUT) "jobjectArray"
	%typemap(jtype) (size_t * OUTPUT,size_t &OUTPUT) "long[][]"
	%typemap(jstype) (size_t * OUTPUT,size_t &OUTPUT) "long[][]"
	%typemap(javain) (size_t * OUTPUT,size_t &OUTPUT) "$javainput"
	%typemap(in) (size_t * OUTPUT,size_t &OUTPUT) (size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		if (JCALL1(GetArrayLength, jenv, $input) == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		$2 = &size;
	}
	%typemap(argout) (size_t * OUTPUT,size_t &OUTPUT)
	{
		jlongArray barray = JCALL1(NewLongArray, jenv, (long)size$argnum);
		jlong * pbarray = JCALL2(GetLongArrayElements,jenv,barray,0);
		memcpy(pbarray,$1,size$argnum);
		JCALL3(ReleaseLongArrayElements, jenv, barray, pbarray, JNI_COMMIT);
		JCALL3(SetObjectArrayElement, jenv, $input, 0, barray);
	}
	%typemap(freearg) (size_t * OUTPUT,size_t &OUTPUT) ""
%enddef
OUTPUT_TYPEMAP_SIZET_P
#undef OUTPUT_TYPEMAP_SIZET_P


#undef INPUT_TYPEMAP_VOIDP
%define INPUT_TYPEMAP_VOIDP
	%typemap(jni) (void *INPUT,size_t INPUT) "jbyteArray"
	%typemap(jtype) (void *INPUT,size_t INPUT) "byte[]"
	%typemap(jstype) (void *INPUT,size_t INPUT) "byte[]"
	%typemap(javain) (void *INPUT,size_t INPUT) "$javainput"
	%typemap(in) (void *INPUT,size_t INPUT) (char * temp,size_t size)
	{
		if (!$input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
			return $null;
		}
		size = JCALL1(GetArrayLength, jenv, $input);
		if (size == 0) {
			SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
			return $null;
		}
		temp = (char *)JCALL2(GetByteArrayElements,jenv,$input,0);
		$1 = (void *)temp;
		$2 = size;
	}
	%typemap(freearg) (void *INPUT,size_t INPUT)
	{
		JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte *)temp$argnum, JNI_ABORT);
	}
%enddef
INPUT_TYPEMAP_VOIDP
#undef INPUT_TYPEMAP_VOIDP

#undef INPUT_BONE_INDICES
%define INPUT_BONE_INDICES
%typemap(in) csArray<csArray<unsigned int> >& INPUT
{
	csArray<csArray<unsigned int> > * uiaa = new csArray<csArray<unsigned int> >();
	jobjectArray boneIndicesarray = (jobjectArray)$input;
	int length = JCALL1(GetArrayLength,jenv,boneIndicesarray);
	for (int i=0;i<length;i++) 
	{
		csArray<unsigned int> uia;
		jlongArray indice = (jlongArray)JCALL2(GetObjectArrayElement,jenv,boneIndicesarray,i);
		jlong * larray = JCALL2(GetLongArrayElements,jenv,indice,0);
		int lengthinner = JCALL1(GetArrayLength,jenv,indice);
		int j = 0;
		for (j=0;j<lengthinner;j++) 
		{
			uia.Push((unsigned int)larray[j]);
		}
		uiaa->Push(uia);
		JCALL3(ReleaseLongArrayElements,jenv,indice,larray,JNI_ABORT);
	}
	$1 = uiaa;
}
%typemap(jni) csArray<csArray<unsigned int> >& INPUT "jobjectArray"
%typemap(jtype) csArray<csArray<unsigned int> >& INPUT "long[][]"
%typemap(jstype) csArray<csArray<unsigned int> >& INPUT "long[][]"
%typemap(javain) csArray<csArray<unsigned int> >& INPUT "$javainput"
%typemap(freearg) csArray<csArray<unsigned int> >& INPUT
{
  delete($1);
}
%enddef
INPUT_BONE_INDICES
#undef INPUT_BONE_INDICES

#undef INPUT_IRENDERBUFFER_INDICES
%define INPUT_IRENDERBUFFER_INDICES
%typemap(in) csArray<iRenderBuffer*>& INPUT
{
	csArray<iRenderBuffer*> * ira = new csArray<iRenderBuffer*>();
	// Fills the arrays
	jlongArray indicesarray = (jlongArray)$input;
	jlong * larray = JCALL2(GetLongArrayElements,jenv,indicesarray,0);
	int length = JCALL1(GetArrayLength,jenv,indicesarray);
	int i = 0;
	for (i=0;i<length;i++) 
	{
		iRenderBuffer* ir = (iRenderBuffer*)(void*)(long)(larray[i]);
		ira->Push(ir);
	}
	JCALL3(ReleaseLongArrayElements,jenv,indicesarray,larray,JNI_ABORT);
	$1 = ira;
}
%typemap(jni) csArray<iRenderBuffer*>& INPUT "jlongArray"
%typemap(jtype) csArray<iRenderBuffer*>& INPUT "long[]"
%typemap(jstype) csArray<iRenderBuffer*>& INPUT "SWIGTYPE_p_iRenderBuffer []"
%typemap(javain) csArray<iRenderBuffer*>& INPUT "cspaceUtils._ConvertArrayToNative($javainput)"
%typemap(freearg) csArray<iRenderBuffer*>& INPUT
{
  delete($1);
}
%enddef
INPUT_IRENDERBUFFER_INDICES
#undef INPUT_IRENDERBUFFER_INDICES

#undef INOUT_TYPEMAP_CSARRAY_TYPE
%define INOUT_TYPEMAP_CSARRAY_TYPE(type)
%typemap(in) csArray<type> & INOUT (size_t size)
{
	csArray<type> * ira = new csArray<type>();
	if (!$input) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
		return $null;
	}
	size = JCALL1(GetArrayLength, jenv, $input);
	if (size == 0) {
		SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
		return $null;
	}
	for (unsigned int i=0;i<size;i++) {
		jobject obj = JCALL2(GetObjectArrayElement,jenv,$input,i);
		if (obj) {
			ira->Put(i,*((type*)(void*)_cs_get_swig_pointer(obj,jenv)));
		}
	}
	$1 = ira;
}
%typemap(argout) csArray<type>& INOUT 
{
	jclass clazz = jenv->FindClass("org/crystalspace3d/" #type);
	jobjectArray joarray = JCALL3(NewObjectArray, jenv, $1->GetSize(), clazz, 0);
	jmethodID mid = jenv->GetMethodID(clazz, "<init>", "(JZ)V");
	for (unsigned int i = 0;i<$1->GetSize();i++) {
		JCALL3(SetObjectArrayElement, jenv, joarray, i, jenv->NewObject(clazz, mid, (jlong)(void*)&($1->Get(i)), true));
	}
	JCALL3(SetObjectArrayElement, jenv, $input, 0, joarray);
}
%typemap(jni) csArray<type>& INOUT "jobjectArray"
%typemap(jtype) csArray<type>& INOUT "type[][]"
%typemap(jstype) csArray<type>& INOUT "type[][]"
%typemap(javain) csArray<type>& INOUT "$javainput"
%typemap(freearg) csArray<type>& INOUT
{
  delete($1);
}
%enddef
INOUT_TYPEMAP_CSARRAY_TYPE(csTriangle)
#undef INOUT_TYPEMAP_CSARRAY_TYPE

#undef OUTPUT_TYPEMAP_CSARRAY_INT
%define OUTPUT_TYPEMAP_CSARRAY_INT
%ignore csArray<csArray<int> >;
%typemap(out) csArray<csArray<int> > * OUTPUT
{
	jclass clazz = jenv->FindClass("[I");
	jobjectArray joarray = JCALL3(NewObjectArray, jenv, $1->GetSize(), clazz, 0);
	for (unsigned int i=0;i<$1->GetSize();i++) {
		jintArray jiarray = JCALL1(NewIntArray, jenv, $1->Get(i).GetSize());
		jint * iarray = JCALL2(GetIntArrayElements,jenv,jiarray,0);
		for (unsigned int j=0;j<$1->Get(i).GetSize();j++) {
			iarray[j] = (jint)$1->Get(i).Get(j);
		}
		JCALL3(ReleaseIntArrayElements, jenv, jiarray, iarray, JNI_COMMIT);
		JCALL3(SetObjectArrayElement, jenv, joarray, i, jiarray);
	}
	delete ($1);
}
%typemap(jni) csArray<csArray<int> > * OUTPUT "jobjectArray"
%typemap(jtype) csArray<csArray<int> > * OUTPUT "int[][]"
%typemap(jstype) csArray<csArray<int> > * OUTPUT "int[][]"
%typemap(javaout) csArray<csArray<int> > * OUTPUT { return $jnicall; }
%enddef
OUTPUT_TYPEMAP_CSARRAY_INT
#undef OUTPUT_TYPEMAP_CSARRAY_INT

#undef TYPEMAP_CSUTILS
%define TYPEMAP_CSUTILS
%typemap(javacode) cspaceUtils 
%{
	public static long[] _ConvertArrayToNative(final Object [] array) 
	{
		if (array == null) 
		{
			return new long[0];
		}
		final long[] pointers = new long[array.length];
		for (int i=0;i<array.length;i++) 
		{
			try 
			{
				if (array[i] == null) continue;
				java.lang.reflect.Field swigCPtr = array[i].getClass().getDeclaredField("swigCPtr");
				swigCPtr.setAccessible(true);
				pointers[i] = ((Long)swigCPtr.get(array[i])).longValue();
			}
			catch (Exception ignore){}
		}
		return pointers;
	}
%}
%enddef
TYPEMAP_CSUTILS
#undef TYPEMAP_CSUTILS
