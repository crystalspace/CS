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
