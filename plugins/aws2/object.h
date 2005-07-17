/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef _KEILA_OBJECT_H__
#define _KEILA_OBJECT_H__

#include "csutil/ref.h"
#include "csutil/refcount.h"
#include "iaws/aws2.h"

#include <string>
#include <vector>

namespace aws
{

namespace autom
{
	class object;
	class string;
	class integer;
	class floating;
	class list;
	class nil;
	
	/** The reference counted pointer class that lets keila objects go away when they're not needed anymore. */
	typedef csRef<object> keeper;
		
	/** Base class for all Keila objects. */
	class object : virtual public iObject
	{	
	private:
		csString name;		
		TYPE otype;
		
	public:
		SCF_DECLARE_IBASE;

		object(TYPE _otype);
		virtual ~object();
		
		object(const object& o, TYPE _otype):name(o.name), otype(_otype) {}		
		
		/** Returns the type of the object, a member of the object::TYPE enumeration. */
		virtual TYPE ObjectType() { return otype; }
		
		/** Sets the name of the object. */
		virtual void SetName(iString* _name) { name=*_name; }
		
		/** Gets the name of the object. */
		virtual csRef<iString> GetName() 
		{ return csPtr<iString> (new scfString (name)); }
		
		/** Converts the object into a string object if possible. */
		virtual string ToString()=0;
		
		/** Converts the object into an integer object, if possible. */
		virtual integer ToInt()=0;
		
		/** Converts the object into a float object, if possible. */
		virtual floating ToFloat()=0;	
		
		/** Converts the object into the text representation of it. This is the inverse of parsing. */
		virtual csRef<iString> ReprObject()=0;
		
		/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
		virtual bool parseObject(std::string::iterator &pos, const std::string::iterator &end)=0;
	};
	
	/** Encapsulates a string object. */
	class string : public object
	{
		std::string value;
		
	public:	
		string():object(T_STRING) {}
		virtual ~string() {}
		
		string(const string& s):object(s, T_STRING), value(s.value) {}		
				
		string(const std::string &s):object(T_STRING), value(s) {}
		
		explicit string(object& o):object(o, T_STRING) 
		{
			string s = o.ToString();
			value = s.value;				
		}
		
		/** Returns a string containing the contents of this string object. */
		const std::string& Value() { return value; }
		
		/** Returns a quoted string, with all double quotes properly escaped. */
		std::string QuotedValue() 
		{
			std::string temp;
			
			temp+='"';
			for(std::string::iterator it=value.begin(); it!=value.end(); ++it)
			{
				if (*it=='"') temp+='\\';
				
				temp+=(*it);	
			}	
			
			temp+='"';
			
			return temp;
		}
		
		/** Converts the object into a string object if possible. */
		virtual string ToString();
		
		/** Converts the object into an integer object, if possible. */
		virtual integer ToInt();
		
		/** Converts the object into a float object, if possible. */
		virtual floating ToFloat();	
		
		/** Converts the object into the text representation of it. This is the inverse of parsing. */
		virtual csRef<iString> ReprObject()
		{
			return csPtr<iString> (new scfString(QuotedValue().c_str()));	
		}		
		
		/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
		virtual bool parseObject(std::string::iterator &pos, const std::string::iterator &end);
		
		/** Concatenates the strings. */
		string operator+(const string& o)
		{
			return string(value + o.value);	
		}				
	};
	
	/** Encapsulates an integer value. */
	class integer : public object
	{
		longlong value;
		
	public:
		integer():object(T_INT) {}
		
		/** Copy constructor. */
		integer(const integer& i):object(i, T_INT), value(i.value) {}
		
		integer(longlong i):object(T_INT), value(i) {}
		
		longlong Value() { return value; }
		
		explicit integer(object& o):object(o, T_INT)
		{
			integer i = o.ToInt();
			value=i.value;	
		}
		
		/** Converts the object into a string object if possible. */
		virtual string ToString();
		
		/** Converts the object into an integer object, if possible. */
		virtual integer ToInt();
		
		/** Converts the object into a float object, if possible. */
		virtual floating ToFloat();	
		
		/** Converts the object into the text representation of it. This is the inverse of parsing. */
		virtual csRef<iString> ReprObject();
		
		/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
		virtual bool parseObject(std::string::iterator &pos, const std::string::iterator &end);
		
		
		/** Adds two integers together. */
		integer operator+(const integer &o)
		{
			return integer(value+o.value);	
		}
		
		/** Subtracts two integers. */
		integer operator-(const integer &o)
		{
			return integer(value-o.value);	
		}
		
		/** Multiplies two integers. */
		integer operator*(const integer &o)
		{
			return integer(value*o.value);	
		}
		
		/** Divides two integers. */
		integer operator/(const integer &o)
		{
			return integer(value/o.value);	
		}
		
		/** Modular division of two integers. */
		integer operator%(const integer &o)
		{
			return integer(value%o.value);	
		}
		
		/** Less comparison of two integers. */
		integer operator<(const integer &o)
		{
			return integer(value<o.value);	
		}
		
		/** Less comparison of two integers. */
		integer operator>(const integer &o)
		{
			return integer(value>o.value);	
		}
		
		/** Equal comparison of two integers. */
		integer operator==(const integer &o)
		{
			return integer(value==o.value);	
		}
		
	};	
	
	/** Encapsulates a floating point value. */
	class floating : public object
	{
		double value;
		
	public:
		floating():object(T_FLOAT) {}
		
		/** Copy constructor. */
		floating(const floating& i):object(i, T_FLOAT), value(i.value) {}
		
		floating(double i):object(T_FLOAT), value(i) {}
		
		double Value() { return value; }
		
		explicit floating(object& o):object(o)
		{
			floating f = o.ToFloat();
			
			value=f.value;			
		}
		
		/** Converts the object into a string object if possible. */
		virtual string ToString();
		
		/** Converts the object into an floating object, if possible. */
		virtual integer ToInt();
		
		/** Converts the object into a float object, if possible. */
		virtual floating ToFloat();	
		
		/** Converts the object into the text representation of it. This is the inverse of parsing. */
		virtual csRef<iString> ReprObject();		
		
		/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
		virtual bool parseObject(std::string::iterator &pos, const std::string::iterator &end);
		
		
		/** Adds two floatings together. */
		floating operator+(const floating &o)
		{
			return floating(value+o.value);	
		}
		
		/** Subtracts two floatings. */
		floating operator-(const floating &o)
		{
			return floating(value-o.value);	
		}
		
		/** Multiplies two floatings. */
		floating operator*(const floating &o)
		{
			return floating(value*o.value);	
		}
		
		/** Divides two floatings. */
		floating operator/(const floating &o)
		{
			return floating(value/o.value);	
		}		
				
	};	
	
			
	/** Encapsulates a string object. */
	class list : public object
	{
		typedef std::vector<keeper> list_type;
		 
		/** The list of objects. */
		list_type value;
		
	public:	
		list():object(T_LIST) {}
		virtual ~list() {}
		
		/** Copy constructor. */
		list(const list& s):object(s, T_LIST)
		{			
			value.insert(value.end(), s.value.begin(), s.value.end());	
		}		
				
		/** Assumes that the string holds the text representation of a list, and parses it. */
		list(std::string &s);				
		
		/** Returns a reference to the list. */
		list_type &Value() { return value; }
				
		/** Converts the object into a string object if possible. */
		virtual string ToString();
		
		/** Converts the object into an integer object, if possible. */
		virtual integer ToInt();
		
		/** Converts the object into a float object, if possible. */
		virtual floating ToFloat();	
		
		/** Converts the object into the text representation of it. This is the inverse of parsing. */
		virtual csRef<iString> ReprObject();
				
		/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
		virtual bool parseObject(std::string::iterator &pos, const std::string::iterator &end);
				
		/** Concatenates the lists. */
		list operator+(const list& o)
		{
			list nl;
			
			nl.value.insert(nl.value.end(), value.begin(), value.end());
			nl.value.insert(nl.value.end(), o.value.begin(), o.value.end());
			
			return nl;
		}	
		
		/** Appends an object to this list. */
		list operator+=(const keeper &k);
		
		/** Returns a keeper to the object at index i of the list.  If I is out of bounds, then a keeper to NIL is returned.  */
		keeper at(size_t i);
		
		/** Returns the size of the list. */
		size_t size() { return value.size(); }
	};	
	
	class function;
	
	/** Encapsulates a string object. */
	class reference : public object
	{
		csString value;
		function *fn;
		
	public:	
		reference():object(T_REFERENCE), fn(0) {}
		virtual ~reference() {}
		
		reference(const reference& s):object(s, T_REFERENCE), value(s.value), fn(s.fn) {}	
		
		void setParent(function *_fn) { fn=_fn; }					
		
		/** Returns a string containing the contents of this reference object. */
		const csString& Value() { return value; }
				
		/** Converts the object into a string object if possible. */
		virtual string ToString();
		
		/** Converts the object into an integer object, if possible. */
		virtual integer ToInt();
		
		/** Converts the object into a float object, if possible. */
		virtual floating ToFloat();	
		
		/** Converts the object into the text representation of it. This is the inverse of parsing. */
		virtual csRef<iString> ReprObject();
				
		/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
		virtual bool parseObject(std::string::iterator &pos, const std::string::iterator &end);								
	};
	
	/** Encapsulates a string object. */
	class nil: public object
	{		
	public:	
		nil():object(T_NIL) {}
		virtual ~nil() {}
		
		/** Copy constructor. */
		nil(const nil& s):object(s, T_NIL) {}		
						
		/** Converts the object into a string object if possible. */
		virtual string ToString();
		
		/** Converts the object into an integer object, if possible. */
		virtual integer ToInt();
		
		/** Converts the object into a float object, if possible. */
		virtual floating ToFloat();	
		
		/** Converts the object into the text representation of it. This is the inverse of parsing. */
		virtual csRef<iString> ReprObject();
				
		/** Parses an object out of a string.  The string is known to hold the whole representation of some object. */
		virtual bool parseObject(std::string::iterator &pos, const std::string::iterator &end);				
						
	};	
		
	
	
		
	
	
	
	
} // namespace autom

} // namespace aws

#endif
