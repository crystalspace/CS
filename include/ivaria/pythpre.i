/*
    Copyright (C) 2003 by Rene Jager (renej@frog.nl, renej.frog@yucom.be)

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
	Python specific stuff for SWIG interface in pre-include phase.

	See include/ivaria/cspace.i
*/

#ifdef SWIGPYTHON

%rename(assign) *::operator=;

/*
	ptr  : either a csRef<type> or csPtr<type>
	name : type name, e.g. "iEngine *"
	type : type of pointer
*/
%define TYPEMAP_OUT_csRef_BODY(ptr, name, type)
	csRef<type> ref(ptr);
	if (ref.IsValid())
	{
		ref->IncRef();
		$result = SWIG_NewPointerObj((void*)(type*)ref, SWIG_TypeQuery(name), 1);
	}
	else
	{
		Py_INCREF(Py_None);
		$result = Py_None;
	}
%enddef

#undef TYPEMAP_OUT_csRef
%define TYPEMAP_OUT_csRef(T)
	%typemap(out) csRef<T>
	{
		TYPEMAP_OUT_csRef_BODY($1, #T " *", T)
	}
%enddef

#undef TYPEMAP_OUT_csPtr
%define TYPEMAP_OUT_csPtr(T)
	%typemap(out) csPtr<T>
	{
		TYPEMAP_OUT_csRef_BODY($1, #T " *", T)
	}
%enddef

#undef TYPEMAP_OUT_csWrapPtr
%define TYPEMAP_OUT_csWrapPtr
	%typemap(out) csWrapPtr
	{
		char type_name[strlen($1.Type)+3];
		strcat(strcpy(type_name, $1.Type), " *");
		TYPEMAP_OUT_csRef_BODY($1.Ref, type_name, iBase)
	}
%enddef

%typemap(in) (int argc, char const * const argv[])
{
	if (!PyList_Check($input))
	{
		PyErr_SetString(PyExc_TypeError, "not a list");
		return NULL;
	}
	$1 = PyList_Size($input);
	typedef char * Char_Ptr;
	$2 = new Char_Ptr[$1 + 1];
	for (int i = 0; i < $1; ++i)
	{
		PyObject * o = PyList_GetItem($input, i);
		if (!PyString_Check(o))
		{
			PyErr_SetString(PyExc_TypeError, "list must contain strings");
			delete [] $2;
			return NULL;
		}
		$2[i] = PyString_AsString(o);
	}
	$2[$1] = 0;
}

%typemap(freearg) (int argc, char const * const argv[])
{
	delete [] $2;
}

%typemap(in) (const char * description, ...)
{
	$1 = "%s";
	$2 = (void *) PyString_AsString($input);
}

#undef TYPEMAP_OUTARG_ARRAY_BODY
%define TYPEMAP_OUTARG_ARRAY_BODY(array_type, base_type, cnt, ptr, to_item)
	PyObject * result_tuple;
	if (!PyTuple_Check($result))
	{
		result_tuple = $result;
		$result = PyTuple_New(1);
		PyTuple_SetItem($result, 0, $result);
	}
	PyObject * arr = PyList_New(cnt);
	for (int i = 0; i < cnt; ++i)
	{
		base_type * item = new base_type(to_item ptr[i]);
		PyObject * o = SWIG_NewPointerObj(
			(void *) item, $descriptor(basetype *), 1
		);
		PyList_SetItem(arr, i, o);
	}
	PyList_Append(l, arr);
	PyObject * arr_tuple = PyTuple_New(1);
	PyTuple_SetItem(arr_tuple, 0, arr);
	result_tuple = $result;
	$result = PySequence_Concat(result_tuple, arr_tuple);
	Py_DECREF(result_tuple);
	Py_DECREF(arr_tuple);
%enddef

#undef TYPEMAP_OUTARG_ARRAY_CNT_PTR
%define TYPEMAP_OUTARG_ARRAY_CNT_PTR(pattern, ptr_init, to_item)
	%typemap(in, numinputs=0) pattern ($2_type ptr, $1_basetype cnt)
	{
		$1 = &cnt;
		$2 = ($2_type) ptr_init;
	}
	%typemap(outarg) pattern
	{
		TYPEMAP_OUTARG_ARRAY_BODY($*2_type, $2_basetype, $1, $2, to_item)
	}
	%typemap(freearg) pattern
	{
		delete [] $2;
	}
%enddef

#undef TYPEMAP_OUTARG_ARRAY_PTR_CNT
%define TYPEMAP_OUTARG_ARRAY_PTR_CNT(pattern, ptr_init, to_item)
	%typemap(in, numinputs=0) pattern ($1_type ptr, $2_basetype cnt)
	{
		$1 = ($1_type) ptr_init;
		$2 = &cnt;
	}
	%typemap(outarg) pattern
	{
		TYPEMAP_OUTARG_ARRAY_BODY($*1_type, $1_basetype, $2, $1, to_item)
	}
	%typemap(freearg) pattern
	{
		delete [] $1;
	}
%enddef

#undef TYPEMAP_IN_ARRAY_BODY
%define TYPEMAP_IN_ARRAY_BODY(array_type, base_type, cnt, ptr, to_item)
	if (PyList_Check($input))
	{
		PyErr_SetString(PyExc_TypeError, "not a list");
		return NULL;
	}
	cnt = PyList_Size($input);
	typedef array_type Array_Type;
	ptr = new Array_Type[cnt];
	for (int i = 0; i < cnt; ++i)
	{
		base_type * p;
		PyObject * o = PyList_GetItem($input, i);
		if (SWIG_ConvertPtr(o, (void **) &p, $descriptor(base_type *),
			SWIG_POINTER_EXCEPTION))
		{
			PyErr_SetString(
				PyExc_TypeError, "list must contain " #base_type "'s"
			);
			delete [] ptr;
			return NULL;
		}
		ptr[i] = to_item p;
	}
%enddef

#undef TYPEMAP_IN_ARRAY_CNT_PTR
%define TYPEMAP_IN_ARRAY_CNT_PTR(pattern, to_item)
	%typemap(in) pattern
	{
		TYPEMAP_IN_ARRAY_BODY($*2_type, $2_basetype, $1, $2, to_item)
	}
	%typemap(freearg) pattern
	{
		delete [] $2;
	}
%enddef

#undef TYPEMAP_IN_ARRAY_PTR_CNT
%define TYPEMAP_IN_ARRAY_PTR_CNT(pattern, to_item)
	%typemap(in) pattern
	{
		TYPEMAP_IN_ARRAY_BODY($*1_type, $1_basetype, $2, $1, to_item)
	}
	%typemap(freearg) pattern
	{
		delete [] $1;
	}
%enddef

#endif // SWIGPYTHON

