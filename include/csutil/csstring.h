/*
    Copyright (C) 1998 by Jorrit Tyberghein
		CSScript module created by Brandon Ehle (Azverkan)
  
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

#ifndef __CSSTRING_H__
#define __CSSTRING_H__

#include "sysdef.h"
#include "string.h"
#include "stdlib.h"

#define c_csString Data(NULL), Size(0), MaxSize(0)

#define NO_EXCEPTIONS

#define PRINTF printf
#ifndef NO_EXCEPTIONS
#define csTHROW(Exception) do { throw csException Exception; } while(0)
#else
#define csTHROW(Exception) do { PRINTF((csException Exception).GetExceptionMessage()); exit(1); } while(0)
#endif

#ifndef CHK
#define CHK(arg) arg
#endif

#define csSize signed long
#define DATA char
#define DATANULL 0x00
#define DATALEN strlen

#define csSTR csString
#define csCSTR const csSTR

class csString;

class csException {
public:
	const char* Error, *Hint;
	unsigned long int ErrorVal;

	csException(const char* tError, const char* tHint="", unsigned long int tErrorVal=0)
		:Error(tError), Hint(tHint), ErrorVal(tErrorVal) {}

	const char* GetExceptionMessage();
};

#ifndef NO_STRING_COM
#include "cscom/com.h"
#include "css/cssdefs.h"

extern const GUID IID_IString;

interface IString:public IUnknown {
	STDMETHOD(xLength)(unsigned long int* Size) PURE;

	STDMETHOD (xData)(DATA** data) PURE;
};
class csString:public IString {
#else
class csString {
#endif
	DATA *Data;
	int Size, MaxSize;

	void DoSetAt(csSize Pos, const DATA& NewData) { 
		if(Pos>Size)
			csTHROW(("csString: Place Past End"));

		Data[Pos]=NewData;
	}

	void AppendNull() {
		DoSetAt(Size, DATANULL);
	}

	void MemCopy(DATA* Dest, const DATA* Source, csSize Size) const {
		memcpy(Dest, Source, sizeof(DATA)*Size);
	}

	void AllocData(csSize NewSize) {
		if(NewSize==MaxSize)
			return;

		if(Data==NULL) {
			CHK(Data=new DATA[NewSize]);
			MaxSize=NewSize;
			Size=0;
			return;
		}
		
		if(Size<NewSize)
			Size=NewSize-1; //Store NULLDATA

		DATA *Old=Data;
		CHK(Data=new DATA[NewSize]);

		if(Size) {
			MemCopy(Data, Old, Size);
		} 

		CHK(delete [] Old);
		MaxSize=NewSize;
	}

	void DoSetSize(csSize NewSize) {
		if(NewSize>=MaxSize) {
			AllocData(NewSize+1);
		}

		Size=NewSize;
		AppendNull();
	}

	csSTR& DoAppend(csCSTR& op, csSize Num=-1) {
		csSize Pos=Size;

		if(Num==-1)
			Num=op.Size;

		DoSetSize(Size+Num);
		
		MemCopy(Data+Pos, op.Data, op.Size);

		return *this;
	}

	csSTR& DoAppend(const DATA* op, csSize Num=-1) {
		csSize Pos=Size;

		if(Num==-1)
			Num=DATALEN (op);

		DoSetSize(Size+Num);
		
		MemCopy(Data+Pos, op, Num);

		return *this;
	}

public:
	csString():c_csString {}

	csString(csCSTR& op):c_csString { DoAppend(op); }
	csString(const DATA* op):c_csString { DoAppend(op); }

	void Free() {
		if(Data!=NULL) {
			Size=MaxSize=0;
			CHK(delete [] Data);
			Data=NULL;
		}
	}

	virtual ~csString() {
		Free();
	}

	void SetSize(csSize NewSize) { DoSetSize(NewSize); }

	void Reclaim() {
		SetSize(Size);
	}

	csSTR& Clear() {
		SetSize(0);
		return *this;
	}

	csSTR Copy() const {
		csSTR New;
		New.SetSize(Size);
		MemCopy(New.Data, Data, Size);
		return New;
	}

	DATA* GetData() const { return Data; } 

	csSize Length() const { return Size; }

	bool IsNull() const { return !Size; }

	void SetAt(csSize Pos, const DATA& NewData) { DoSetAt(Pos, NewData); }

	DATA& GetAt(csSize Pos) {
		if(Pos>Size)
			csTHROW(("csString: GetAt Past End"));

		return Data[Pos];
	}

	const DATA& GetAt(csSize Pos) const {
		if(Pos>Size)
			csTHROW(("csString: Overwrite Past End"));

		return Data[Pos];
	}

	csSTR& Append(csCSTR& op, csSize Num=-1) {
		return DoAppend(op, Num);
	}

	csSTR& Append(const DATA* op, csSize Num=-1) {
		return DoAppend(op, Num);
	}

	csSTR& Insert(csSize Pos, csCSTR& op) {
		if(Data==NULL)
		{
			Append(op);
			return *this;
		}

		if(Pos>Size)
			csTHROW(("csString: Insert Past End"));

		DATA *Old=Data;
		csSize NewSize=Size+op.Length()+1;
		CHK(Data=new DATA[NewSize]);

		MemCopy(Data, Old, Pos);
		MemCopy(Data+Pos, op.Data, op.Length());
		MemCopy(Data+Pos+op.Length(), Old+Pos, Size-Pos);

		MaxSize=NewSize;
		Size=NewSize-1;
		AppendNull();

		CHK(delete [] Old);

		return *this;
	}

	csSTR& Overwrite(csSize Pos, csCSTR& op) {
		if(Data==NULL)
			Append(op);
			
		if(Pos>Size)
			csTHROW(("csString: Overwrite Past End"));

		DATA *Old=Data;
		csSize NewSize=Pos+MAX(Size-Pos, op.Length())+1;
		CHK(Data=new DATA[NewSize]);

		MemCopy(Data, Old, Pos);
		MemCopy(Data+Pos, op.Data, op.Length());

		if((Size-Pos)>op.Length())
			MemCopy(Data+Pos+op.Length(), Old+Pos+op.Length(), Size-Pos-op.Length());

		MaxSize=NewSize;
		Size=NewSize-1;
		AppendNull();

		CHK(delete [] Old);

		return *this;
	}

	csSTR& Replace(csCSTR& op, csSize Num=-1) {
		Clear();
		Append(op, Num);
		return *this;
	}

	bool Comp(csCSTR& op) {
		if(Size!=op.Length())
			return 0;

		return memcmp(Data, op.Data, Size)==0;
	}

	bool CompNoCase(csCSTR& op) {
		if(Size!=op.Length())
			return 0;

		return strncasecmp(Data, op.Data, Size)==0;
	}

	csSTR& operator=(csCSTR& op) { return Replace(op); }
	csSTR& operator+=(csCSTR& op) { return Append(op); }
	csSTR& operator+=(const DATA* op) { return Append(op); }

	csSTR operator+(csCSTR& op) const { return Copy().Append(op); }

	operator DATA*() { return Data; }
	operator const DATA*() const { return Data; }

	bool operator==(csCSTR& op) { return Comp(op); }

#ifndef NO_STRING_COM
	csString(IString *op):c_csString { xCopy(op); }
	operator IString*() { return GetIString(); }

	GUID ToGuid() {
		GUID guid;
		unsigned short tmp [8];
		sscanf(Data+1, "%08lx-%04hx-%04hx-%02hx%02hx-%02hx%02hx%02hx%02hx%02hx%02hx", &guid.Data1, &guid.Data2, &guid.Data3, &tmp [0], &tmp [1], &tmp [2], &tmp [3], &tmp [4], &tmp [5], &tmp [6], &tmp [7]);
		for (int i = 0; i <= 7; i++)
	    guid.Data4 [i] = tmp [i];
		return guid;
	};
 
	DECLARE_IUNKNOWN();
	DECLARE_INTERFACE_TABLE(csString);
	QUERY_DERIVED(IString)

	void xCopy(IString *istring) {
		Clear();
		DATA *data=NULL;
		istring->xData(&data);
		unsigned long int NewSize=0;
		istring->xLength(&NewSize);
		Append(data, NewSize);
	}

	STDMETHODIMP xLength(unsigned long int* Size);
	STDMETHODIMP xData(DATA** data);
#endif
};

inline csSTR operator+(const DATA* op, const csSTR& op2) { return csSTR(op).Append(op2); }

inline csSTR IntToStr(unsigned long int Value) {
	csSTR str;
	str.SetSize(33);
	sprintf(str.GetData(), "%ld", Value);
	return str;
}

#endif
