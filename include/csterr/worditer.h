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

#ifndef HEADER_WORDITER_H
#define HEADER_WORDITER_H

/*
	Breaks a string in words seperated by user-defined characters.
*/
class WordIterator
{
	protected:
		char* string_pos;
		char* token;
		int token_length;
		const char* delimiters;
		
	public:
		/*
			Creates a new iterator using string as the source
			and delim as the list of characters that seperate
			words
		*/
		WordIterator(char* string, const char* delim);
		~WordIterator();
		
		/*
			Returns the next word, or 0 if there are none.
		*/
		char* nextWord();
};


#endif
