/*
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

#include <stdlib.h>
#include <string.h>

#include "csterr/worditer.h"


WordIterator::WordIterator(char* string, const char* delim)
{
	string_pos = string;
	delimiters = delim;
	token_length = 64;
	token = new char[token_length];
};

WordIterator::~WordIterator()
{
	if (token)
	{
		delete token;
	};
};

char* WordIterator::nextWord()
{
	if (string_pos)
	{
		int len = 0;

		//Strip delims
		len = strspn(string_pos, delimiters);
		string_pos += len;

		//Read in non-delims
		len = strcspn(string_pos, delimiters);
		if (token_length <= len)
		{
			delete token;
			token = new char[len + 1];
		};

		if (len == 0)
		{
			return 0;
		};
		
		strncpy(token, string_pos, len);
		token[len] = '\0';

		string_pos += len;

		return token;
	}
	else
	{
		return 0;
	};
};
