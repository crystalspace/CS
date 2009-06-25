/* 
 * Typemaps for removing csstring types from bindings 
 *
 * The objective is to remove csString types from scripting languages
 * and use their native string types instead. This should be possible
 * by implementing the following templates for each language.
 *
 * You should then also ignore the types in the typemap in order to remove
 * the cs string types from scripting languages completely.
 *
 * TYPEMAP_STRING is a typemap working for csString types and similar. 
 * It should typemap both the passed type and a pointer to it.
 *
 * TYPEMAP_STRING_PTR is a typemap for iString. It uses a second parameter 
 * otherwise it can't know how to initialize a new object.
 * 
 * Both typemaps define in, out, typecheck.
 * 
 * Check common/python/csstring.i for python implementation.
 */
TYPEMAP_STRING(csStringFast)
TYPEMAP_STRING(csString)
TYPEMAP_STRING(csStringBase)
%apply csStringBase * { const csStringBase& };
TYPEMAP_STRING(scfString)
TYPEMAP_STRING_PTR(iString,scfString)

/*
 * We include the string headers for scripting languages
 * which don't implement the former defines.
 */
%ignore iString::SetCapacity;
%ignore iString::GetCapacity;
%ignore iString::SetGrowsBy;
%ignore iString::GetGrowsBy;
%ignore iString::Truncate;
%ignore iString::Reclaim;
%ignore iString::ShrinkBestFit;
%ignore iString::Clear;
%ignore iString::Empty;
%ignore iString::Clone;
%ignore iString::GetData (); // Non-const.
%ignore iString::Length;
%ignore iString::IsEmpty;
%ignore iString::SetAt;
%ignore iString::GetAt;
%ignore iString::Insert;
%ignore iString::Overwrite;
%ignore iString::Append;
%ignore iString::Slice;
%ignore iString::SubString;
%ignore iString::FindFirst;
%ignore iString::FindLast;
%ignore iString::Find;
%ignore iString::ReplaceAll;
%ignore iString::Format;
%ignore iString::FormatV;
%ignore iString::Replace;
%ignore iString::Compare;
%ignore iString::CompareNoCase;
%ignore iString::Downcase;
%ignore iString::Upcase;
%ignore iString::operator+=;
%ignore iString::operator+;
%ignore iString::operator==;
%ignore iString::operator [] (size_t);
%ignore iString::operator [] (size_t) const;
%ignore iString::operator char const*;
%include "iutil/string.h"

%ignore csStringBase;
%ignore csStringBase::operator [] (size_t);
%ignore csStringBase::operator [] (size_t) const;
%ignore csString::csString (size_t);
%ignore csString::csString (char);
%ignore csString::csString (unsigned char);
%ignore csString::SetCapacity;
%ignore csString::GetCapacity;
%ignore csString::SetGrowsBy;
%ignore csString::GetGrowsBy;
%ignore csString::SetGrowsExponentially;
%ignore csString::GetGrowsExponentially;
%ignore csString::Truncate;
%ignore csString::Free;
%ignore csString::Reclaim;
%ignore csString::ShrinkBestFit;
%ignore csString::Clear;
%ignore csString::Empty;
%ignore csString::SetAt;
%ignore csString::GetAt;
%ignore csString::DeleteAt;
%ignore csString::Insert;
%ignore csString::Overwrite;
%ignore csString::Append;
%ignore csString::AppendFmt;
%ignore csString::AppendFmtV;
%ignore csString::Slice;
%ignore csString::SubString;
%ignore csString::FindFirst;
%ignore csString::FindLast;
%ignore csString::Find;
%ignore csString::ReplaceAll;
%ignore csString::Replace;
%ignore csString::Compare;
%ignore csString::CompareNoCase;
%ignore csString::Clone;
%ignore csString::LTrim;
%ignore csString::RTrim;
%ignore csString::Trim;
%ignore csString::Collapse;
%ignore csString::Format;
%ignore csString::FormatV;
%ignore csString::PadLeft;
%ignore csString::PadRight;
%ignore csString::PadCenter;
%ignore csString::GetData (); // Non-const.
%ignore csString::Detach;
%ignore csString::Upcase;
%ignore csString::Downcase;
%ignore csString::operator=;
%ignore csString::operator+=;
%ignore csString::operator+;
%ignore csString::operator==;
%ignore csString::operator!=;
%ignore csString::operator [] (size_t);
%ignore csString::operator [] (size_t) const;
%ignore csString::operator const char*;
%ignore operator+ (const char*, const csString&);
%ignore operator+ (const csString&, const char*);
%ignore operator<<;
%include "csutil/csstring.h"
