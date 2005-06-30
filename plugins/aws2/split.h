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

#ifndef __DSL_SPLIT_FUNC_H__
#define __DSL_SPLIT_FUNC_H__

#include <string>
#include <vector>

namespace std
{
	/** Split's the string s by the token sc. */
	template<typename charT>
	typename vector< basic_string<charT> >::size_type 
	split(basic_string<charT> &s, const basic_string<charT> &sc, vector< basic_string<charT> > &sv)
	{		
		typename basic_string<charT>::size_type i=s.find(sc), last_i=0;
		
		if (i==basic_string<charT>::npos)
		{
			sv.push_back(s);
			
			return sv.size();	
		}
		
		for(; i!=basic_string<charT>::npos; last_i=i+sc.size(), i=s.find(sc,last_i))
		{						
			sv.push_back(s.substr(last_i, i-last_i));			
		}
		
		if (last_i!=basic_string<charT>::npos && last_i<s.size()) sv.push_back(s.substr(last_i));
								
		return sv.size();		
	}
	
	/** Split's the string s by the token sc. */
	template<typename charT>
	typename vector< basic_string<charT> >::size_type 
	split(basic_string<charT> &s, charT sc, vector< basic_string<charT> > &sv)
	{		
		typename basic_string<charT>::size_type i=s.find(sc), last_i=0;
		
		if (i==basic_string<charT>::npos)
		{
			sv.push_back(s);
			
			return sv.size();	
		}
		
		for(; i!=basic_string<charT>::npos; last_i=i+1, i=s.find(sc,last_i))
		{						
			sv.push_back(s.substr(last_i, i-last_i));			
		}
		
		if (last_i!=basic_string<charT>::npos && last_i<s.size()) sv.push_back(s.substr(last_i));
						
		return sv.size();		
	}

}

#endif
