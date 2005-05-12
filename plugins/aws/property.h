/*
    Copyright (C) 2000-2001 by Christopher Nelson

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

#ifndef _AWS_PROPERTY_H__
#define _AWS_PROPERTY_H__


#include "registrar.h"
#include "csgeom/csrect.h"
#include "csutil/snprintf.h"
#include "csutil/scanstr.h"
#include <map>

class awsPropertyBase;

/** A bag of properties.  This lets us search for them.  The actual properties are automatically registered when they're created. */
class awsPropertyBag
{
	typedef std::map<std::string, awsPropertyBase *> property_map;

	/** The map of properties. */
	property_map props;
public:
	awsPropertyBag() {}
	~awsPropertyBag() {}

	/** Registers a given property. */
	void Register(const std::string &name, awsPropertyBase *p)
	{
		props.insert(std::make_pair(name, p));
	}

	/** Finds the pointer to some property. It returns a raw pointer because it is not expected that
	 * you will ever create properties on new.  */
	awsPropertyBase *Find(const std::string &name)
	{
		property_map::iterator pos = props.find(name);

		if (pos==props.end()) return 0;
		else return pos->second;
	}
};

class awsPropertyBase
{
protected:
	/** True if it is possible to read this property. */
	bool readable;

	/** True if it is possible to write this property. */
	bool writeable;

public:
	awsPropertyBase(bool _writeable=true, bool _readable=true):readable(_readable), writeable(_writeable) {}
	virtual ~awsPropertyBase() {}

	/** Sets the value of this property (if allowed.) Returns true on success, else false. */
	virtual bool Set(autom::keeper &_value)=0;

	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(autom::keeper &_value)=0;

	/** Binds this property to the property bag. */
	virtual void Bind(const std::string &name, awsPropertyBag &bag)
	{		
		bag.Register(name, this);
	}
};


/** A property is intended to be a class member.  You should never create a property via new.  It maintains a value of some type, either string, int, float, or list. */
class awsProperty : virtual public awsPropertyBase
{
	/** This is the value of the property. */
	autom::keeper value;		
	
public:
	/** Creates a new property, registers it with a bag, and optionally sets it's access. */
	awsProperty(bool _writeable=true, bool _readable=true):awsPropertyBase(_writeable, _readable)
	{	
	}

	virtual ~awsProperty() 
	{
	}

	/** Sets the value of this property (if allowed.) Returns true on success, else false. */
	virtual bool Set(autom::keeper &_value)
	{
		if (writeable) 
		{ 
			value=_value; 
			return true; 
		}
		else return false;
	}

	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(autom::keeper &_value)
	{
		if (readable)
		{
			_value=value;
			return true;
		}
		else return false;
	}	
};

/**  A bound property.  This property is bound to some csRect. */
class awsRectProperty : virtual public awsPropertyBase
{
	/** The rect we store. */
	csRect &value;

public:
	awsRectProperty(csRect &_value, bool _writeable=true, bool _readable=true):awsPropertyBase(_writeable, _readable), value(_value)
	{
	}

	virtual ~awsRectProperty()
	{
	}

	/** Sets the value of this property (if allowed.) Returns true on success, false on failure.  Particularly, it may return
	 * false if the rectangle string is not in the format (0, 0)-(100, 100).  It should be obvious that a float, int, or list 
	 * cannot be successfully converted to a rect.  */
	virtual bool Set(autom::keeper &_value)
	{
		if (writeable)
		{
			std::string s(_value->toString().Value());
			if (csScanStr(s.c_str(), "(%d, %d)-(%d, %d)", &value.xmin, &value.ymin, &value.xmax, &value.ymax)!=-1) return true;
			else return false;
		}
		else return false;
	}

	/** Sets the value of the rect directly. Returns true on sucess, false on failure. */
	virtual bool Set(const csRect &_value)
	{
		if (writeable)
		{
			value = _value;
            return true;
		}
		else return false;		
	}
	
	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(autom::keeper &_value)
	{
		char buf[128];

        if (readable)
		{
			cs_snprintf(buf, sizeof(buf), "(%d, %d)-(%d, %d)", value.xmin, value.ymin, value.xmax, value.ymax);
			
			_value.AttachNew(new autom::string(buf));

			return true;
		}       
		else return false;
	}

	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(csRect &_value)
	{
		if (readable)
		{
			value = _value;
            return true;
		}
		else return false;		
	}

};

/**  A bound property.  This property is bound to some int. */
class awsIntProperty : virtual public awsPropertyBase
{
	/** The int we store. */
	int &value;

public:
	awsIntProperty(int &_value, bool _writeable=true, bool _readable=true):awsPropertyBase(_writeable, _readable), value(_value)
	{
	}

	virtual ~awsIntProperty()
	{
	}

	/** Sets the value of this property (if allowed.) Returns true on success, false on failure.  Particularly, it may return
	 * false if the rectangle string is not in the format (0, 0)-(100, 100).  It should be obvious that a float, int, or list 
	 * cannot be successfully converted to a rect.  */
	virtual bool Set(autom::keeper &_value)
	{
		if (writeable)
		{
			value = (int)_value->toInt().Value();
			return true;
		}
		else return false;
	}

	/** Sets the value of the rect directly. Returns true on sucess, false on failure. */
	virtual bool Set(const int &_value)
	{
		if (writeable)
		{
			value = _value;
            return true;
		}
		else return false;		
	}
	
	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(autom::keeper &_value)
	{
        if (readable)
		{			
			_value.AttachNew(new autom::integer(value));
			return true;
		}       
		else return false;
	}

	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(int &_value)
	{
		if (readable)
		{
			value = _value;
            return true;
		}
		else return false;		
	}

};

/**  A bound property.  This property is bound to some float. */
class awsFloatProperty : virtual public awsPropertyBase
{
	/** The int we store. */
	float &value;

public:
	awsFloatProperty(float &_value, bool _writeable=true, bool _readable=true):awsPropertyBase(_writeable, _readable), value(_value)
	{
	}

	virtual ~awsFloatProperty()
	{
	}

	/** Sets the value of this property (if allowed.) Returns true on success, false on failure.  Particularly, it may return
	 * false if the rectangle string is not in the format (0, 0)-(100, 100).  It should be obvious that a float, int, or list 
	 * cannot be successfully converted to a rect.  */
	virtual bool Set(autom::keeper &_value)
	{
		if (writeable)
		{
			value = (float)_value->toFloat().Value();
			return true;
		}
		else return false;
	}

	/** Sets the value of the rect directly. Returns true on sucess, false on failure. */
	virtual bool Set(const float &_value)
	{
		if (writeable)
		{
			value = _value;
            return true;
		}
		else return false;		
	}
	
	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(autom::keeper &_value)
	{
        if (readable)
		{			
			_value.AttachNew(new autom::floating(value));
			return true;
		}       
		else return false;
	}

	/** Gets the value of this property if allowed, returns false if not allowed. */
	virtual bool Get(float &_value)
	{
		if (readable)
		{
			value = _value;
            return true;
		}
		else return false;		
	}

};

#endif
