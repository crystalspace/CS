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

#include "cssysdef.h"
#include "registrar.h"
#include "csutil/snprintf.h"
#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <stdlib.h>

namespace aws
{

namespace autom
{

  SCF_IMPLEMENT_IBASE(object)
    SCF_IMPLEMENTS_INTERFACE(iObject)  
  SCF_IMPLEMENT_IBASE_END
	
/////////////////////////////////// Object - Record Allocation with GC ///////////////////////////////////////

object::object(TYPE _otype):otype(_otype) 
{
  
}
	

object::~object() 
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	

/** Converts the object into a string object if possible. */
string 
string::ToString()
{
	return string(value);	
}

/** Converts the object into an integer object, if possible. */
integer 
string::ToInt()
{
	return integer(strtol(value.GetData(), 0, 10));
}

/** Converts the object into a float object, if possible. */
floating 
string::ToFloat()
{
	return floating(strtod(value.GetData(), 0));	
}

bool
string::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	value = "";
	
	if (pos==end || *pos != '"') return false;
	
	++pos;
	for(;pos!=end && *pos!='"'; ++pos)
	{
		if (*pos=='\\')
		{
			++pos;
			switch((*pos))
			{
				case 'n': value+=('\n'); break;	
				case 'r': value+=('\r'); break;	
				case 't': value+=('\t'); break;					
				default: value+=(*pos);
			}	
			
			continue;
		}
		
		value+=(*pos);	
	}
	
	return true;
}


//////////////////// Integer Object //////////////////////////////////////

/** Converts the object into a string object if possible. */
string 
integer::ToString()
{
	char buf[128]={0};
		
	return string(csString(buf, cs_snprintf(buf, sizeof(buf), "%lld", value)));
}

/** Converts the object into an integer object, if possible. */
integer 
integer::ToInt()
{
	return integer(value);	
}

/** Converts the object into a float object, if possible. */
floating 
integer::ToFloat()
{
	return floating((float)value);	
}

csRef<iString>
integer::ReprObject()
{
	return csPtr<iString> (new scfString (ToString().Value()));
}

bool
integer::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	if (pos==end || !isdigit(*pos)) return false;
	
	std::string temp;
	
	for(; pos!=end && isdigit(*pos); ++pos)	
		temp+=(*pos);			

	value = strtoll(temp.c_str(), 0, 10);
	
	return true;
}

//////////////////// Floating Point Object //////////////////////////////////////

/** Converts the object into a string object if possible. */
string 
floating::ToString()
{
  csString buf;
  buf.Format ("%g", value);
  
  return string(buf);
}

/** Converts the object into an integer object, if possible. */
integer 
floating::ToInt()
{
	return integer((int)value);	
}

/** Converts the object into a float object, if possible. */
floating 
floating::ToFloat()
{
	return floating(value);	
}

csRef<iString>
floating::ReprObject()
{
	return csPtr<iString> (new scfString (ToString().Value()));
}

bool
floating::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	std::string::iterator start = pos;
	
	if (pos==end || !isdigit(*pos)) return false;
	
	std::string temp;
	bool have_radix_point=false;
	
	while(pos!=end && (isdigit(*pos) || (*pos=='.' && have_radix_point==false)))
	{
		if (*pos=='.') have_radix_point=true;
		
		temp+=(*pos);
		++pos;	
	}
	
	if (have_radix_point==false)
	{
		pos=start;
		return false;	
	}
	
	value = strtod(temp.c_str(), 0);
	
	return true;
}

//////////////////// List Object //////////////////////////////////////

list::list(std::string &s):object(T_LIST) 
{
	std::string::iterator pos = s.begin();
	
	parseObject(pos, s.end());	
}

/** Converts the object into a string object if possible. */
string 
list::ToString()
{
	std::string temp;
	
	temp+=('[');
	
	for(list_type::iterator it=value.begin(); it!=value.end(); ++it)
	{
		keeper k = *it;
		
		switch(k->ObjectType())
		{
			case object::T_STRING:
			{				
				temp+=(k->ToString().QuotedValue());				
			} break;
			
			default:
			{				
				temp+=(k->ToString().Value());
			}
		}
		
		temp+=(',');		
	}
	
	temp+=(']');
	
	return string(temp.c_str());
}

/** Converts the object into an integer object, if possible. */
integer 
list::ToInt()
{
	return integer(0);	
}

/** Converts the object into a float object, if possible. */
floating 
list::ToFloat()
{
	return floating(0.0);	
}

csRef<iString>
list::ReprObject()
{
	return csPtr<iString> (new scfString (ToString().Value()));
}

bool
list::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{	
	value.clear();
	
	if (pos==end || *pos!='[') return false;
	
	++pos;
	
	for(; pos!=end && *pos!=']'; ++pos)
	{
		// ignore whitespace
		if (isspace(*pos) || *pos==',') continue;
		
		keeper k(Parse(pos, end));	
		
		if (k.IsValid())
		{	
			value.push_back(k);
			(void)k->ToInt(); /// Why is this necessary?  I don't know.  I wish it wasn't.  It's definitely wierd.
		}				
		else
			value.push_back(keeper(Nil()));
	}	

	return true;
}


list 
list::operator+=(const keeper &k)
{
	value.push_back(k);
	
	return *this;	
}

keeper 
list::at(size_t index)
{
	if (index>value.size()) return keeper(Nil());
	else return keeper(value.at(index));	
}

//////////////////// Reference Object //////////////////////////////////////

/** Converts the object into a reference object if possible. */
string
reference::ToString()
{
	return (*fn)[value]->ToString();
}

/** Converts the object into an integer object, if possible. */
integer 
reference::ToInt()
{
	return (*fn)[value]->ToInt();
}

/** Converts the object into a float object, if possible. */
floating 
reference::ToFloat()
{
	return (*fn)[value]->ToFloat();
}

csRef<iString>
reference::ReprObject()
{
	return csPtr<iString> (new scfString (value));
}

bool
reference::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	value = "";
	
	if (pos==end || *pos != '$') return false;
		
	for(; pos!=end && (*pos=='$' || isalnum(*pos)); ++pos)			
		value+=(*pos);		
	
	return true;
}

//////////////////// Nil Object //////////////////////////////////////

/** Converts the object into a string object if possible. */
string 
nil::ToString()
{
	return string();
}

/** Converts the object into an integer object, if possible. */
integer 
nil::ToInt()
{
	return integer(0);	
}

/** Converts the object into a float object, if possible. */
floating 
nil::ToFloat()
{
	return floating(0.0);	
}

csRef<iString>
nil::ReprObject()
{
	return csPtr<iString> (new scfString ("nil"));
}

bool
nil::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	
	if (pos==end || *pos!='n') return false;
	if (pos==end || *(++pos)!='i') return false;
	if (pos==end || *(++pos)!='l') return false;
	
	++pos;	

	return true;	
}

//////////////////// Blob Object //////////////////////////////////////

static uint pow85[] = {
	85*85*85*85, 85*85*85, 85*85, 85, 1
};

/** Encodes a tuple into ascii85 format. */
void 
blob::encode_tuple(uint tuple, int count)
{
	//filter::console_output out;
	
	int i;
	char buf[5];
		
	for(i=0; i<5; ++i)
	{
		buf[i] = tuple % 85;
		tuple /= 85;		
	}
	
	for(i=4; i>=0; --i)	
	{		
		char c = buf[i];		
		encoded+=(c + '!');	
		
		//out << "blob: encoded " << c << ":" << (c + '!') << "\n";			
	} 
}
		
/** Encodes a data buffer into ascii85 format. */
void 
blob::encode(unsigned char *data, uint size)
{
	encoded.assign("<~", 2);
			
	uint tuple=0;
	uint i;
	int count=0;
	
	for(i=0; i<size; ++i)
	{
		unsigned char c = data[i];
		
		switch(count++)
		{
			case 0: tuple |= (c<<24); break;
			case 1: tuple |= (c<<16); break;
			case 2: tuple |= (c<<8); break;
			case 3:
					tuple |= c;
					
					if (tuple==0) encoded+=('z');
					else encode_tuple(tuple, count);
					
					tuple=0;
					count=0;
				break;
		}		
	}	
	
	if (count>0) encode_tuple(tuple, count);
	
	encoded.append("~>", 2);
}

void
blob::decode_tuple(uint tuple, int bytes, raw_data_t &output)
{
	switch (bytes) 
	{
		case 4:
			output.push_back(tuple >> 24);
			output.push_back(tuple >> 16);
			output.push_back(tuple >>  8);
			output.push_back(tuple);
			break;
		case 3:
			output.push_back(tuple >> 24);
			output.push_back(tuple >> 16);
			output.push_back(tuple >>  8);
			break;
		case 2:
			output.push_back(tuple >> 24);
			output.push_back(tuple >> 16);
			break;
		case 1:
			output.push_back(tuple >> 24);
			break;
	}
	
}

bool 
blob::decode(raw_data_t &output)
{
	output.clear();
	
	uint tuple=0;
	int count=0;
	
	unsigned char c;
	
	std::string::iterator pos=encoded.begin();
	
	if (encoded.size()<2) return false;
				
	// Check for the <~ opening sequence.
	if (*pos != '<') return false;
	else ++pos;
	
	if (*pos != '~') return false;
	else ++pos; 	
			
	for(; pos!=encoded.end(); ++pos)
	{
		c = *pos;
				
		switch(c)
		{			
			default:
				// Check to see if the character is in range.  If not, abort.
				if (c < '!' || c > 'u')
				{
		  			//out << "blob: bad character in the stream, char code=" << c << ".\n";
					return false;					
				}
				
				tuple += (c-'!') * pow85[count++];
				
// 				out << "blob: decode tuple: " << tuple << "\n";
				
				if (count==5)
				{
					decode_tuple(tuple, 4, output);
					count=0;
					tuple=0;	
				}				
			break;
			
			case 'z':
				// A 'z' should appear only by itself, not inside a 5-tuple.
				if (count!=0)
				{
					//out << "blob: decode error: 'z' occurs inside 5-tuple.\n";
					return false;	
				}
				
				output.push_back(0);
				output.push_back(0);
				output.push_back(0);
				output.push_back(0);			
			break;
			
			case '~':
				c= *(++pos);
				if (c=='>')
				{
					if (count>0)
					{
						--count;
						tuple+=pow85[count];
						decode_tuple(tuple, count, output);
					}	
					
					return true;
				}
				else
				{
					//out << "blob: decode: invalid terminating sequence. Data may be corrupt.\n";
					// Invalid terminating sequence.  A '~' must be followed by a '>' or preceded by a '<'
					// to begin or end an ascii85 sequence.
					return false;	
				}				
			break;
			
			// Ignore whitespace or other control characters.
			case '\n': case '\r': case '\t': case ' ':
			case '\0': case '\f': case '\b': case 0177:
			break;			
			
		}
	}	
			
	return true;
}

string 
blob::ToString()
{		
	return string(encoded.c_str());		
}		
		
integer 
blob::ToInt()
{
	return 0;	
}
		
		
floating 
blob::ToFloat()
{
	return 0.0;	
}
		
		
csRef<iString>
blob::ReprObject()
{
	char buf[128]={0};
		
	std::string tmp("/");
	tmp.append(std::string(buf, cs_snprintf(buf, sizeof(buf), "%ld", encoded.size())));		
	tmp.append(encoded);
	
	return csPtr<iString>(new scfString(tmp.c_str()));
}				
		
bool 
blob::parseObject(std::string::iterator &pos, const std::string::iterator &end)
{
	uint size;
	
	if (pos==end || *pos!='/') return false;
					
	++pos;
	
	// Get the size of the blob.
	std::string temp;
			
	for(; pos!=end && isdigit(*pos); ++pos)			
		temp+=(*pos);			
		
	// If there is no size, then abort.
	if (temp.size()==0) return false;
				
	size = strtoll(temp.c_str(), 0, 10);
	
	// Get the entire encoded string.
	for(uint i=0; i<size && pos!=end; ++i, ++pos)
	{
		encoded+=(*pos);	
	}
	
	return true;	
}		

	
} // namespace autom

} // namespace aws
