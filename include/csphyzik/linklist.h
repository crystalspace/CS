#ifndef CTLINKLIST_H
#define CTLINKLIST_H

#include <stdlib.h>

// Should be a template, but the big bullies of the Crytstal Space team
// don't like templates :)

class llLink{
public:
  llLink(){ contents = NULL; next = NULL; }
  llLink( void * c ){ contents = c; next = NULL; }
  llLink( void * c, llLink *n ){ contents = c; next = n; }
  ~llLink(){  }
  void delete_contents(){ if( contents ) delete contents; }  //!me safe??
  void * contents;
  llLink *next;
};

// single link list. Uses a sentinel at head. 
// caches link immediately before most recent access for ease of deletion
// caches more recent link for ease of iteration.
class ctLinkList{
public:
  ctLinkList(){ size = 0; head = new llLink(); prev = NULL; current = head; }

  void reset(){ prev = NULL; current = head; }

  void * get_first(){  if( head->next != NULL ){
            prev = head; 
            current = head->next; 
            return head->next->contents; 
          } else return NULL; 
        }

  void * get_next(){  
    if( current->next ){
      prev = current;
      current = current->next;
      return current->contents;
    }else{
      return NULL;
    }
  }

  void add_link( void * c ){  head->next = new llLink( c, head->next );  current = head->next; prev = head; size++; }
  
  // remove and delete link, delete contents
  void delete_link( void * c ){ 
    if( c == NULL ) return;
    if( prev && prev->next && c == prev->next->contents ){
      current = prev->next->next;
      delete prev->next;
      size--;
      prev->next = current;
    }else{
      llLink *ll = head;
      while( ll->next ){
        if( ll->next->contents == c ){
          prev = ll;
          current = ll->next->next;
          ll->next->delete_contents();
          delete ll->next;
          size--;
          prev->next = current;
          return;
        }
        ll = ll->next;
      }
    }
  }

  // remove and delete link, DOESN'T delete contents
  void remove_link( void * c ){ 
    if( c == NULL ) return;
    if( prev && prev->next && c == prev->next->contents ){
      current = prev->next->next;
      delete prev->next;
      size--;
      prev->next = current;
    }else{
      llLink *ll = head;
      while( ll->next ){
        if( ll->next->contents == c ){
          prev = ll;
          current = ll->next->next;
          delete ll->next;
          size--;
          prev->next = current;
          return;
        }
        ll = ll->next;
      }
    }
  }

  long get_size(){ return size; }

protected:
  llLink *head;
  llLink *prev;  // cached the last link accessed
  llLink *current;  // cached the last link accessed

  long size;
};

class ctEntity;

class ctLinkList_ctEntity : public ctLinkList
{
public:
  ctEntity * get_first(){
    return (ctEntity *)ctLinkList::get_first();
  }

  ctEntity * get_next(){
    return (ctEntity *)ctLinkList::get_next();
  }

  void add_link( ctEntity * plink ){
    ctLinkList::add_link( (void *)plink );
  }

  void remove_link( ctEntity * plink ){
    ctLinkList::remove_link( (void *)plink );
  }

  void delete_link( ctEntity * plink ){
    ctLinkList::delete_link( (void *)plink );
  }

};

class ctPhysicalEntity;

class ctLinkList_ctPhysicalEntity : public ctLinkList
{
public:
  ctPhysicalEntity * get_first(){
    return (ctPhysicalEntity *)ctLinkList::get_first();
  }

  ctPhysicalEntity * get_next(){
    return (ctPhysicalEntity *)ctLinkList::get_next();
  }

  void add_link( ctPhysicalEntity * plink ){
    ctLinkList::add_link( (void *)plink );
  }

  void remove_link( ctPhysicalEntity * plink ){
    ctLinkList::remove_link( (void *)plink );
  }

  void delete_link( ctPhysicalEntity * plink ){
    ctLinkList::delete_link( (void *)plink );
  }

};

class ctPointObj;

class ctLinkList_ctPointObj : public ctLinkList
{
public:
  ctPointObj * get_first(){
    return (ctPointObj *)ctLinkList::get_first();
  }

  ctPointObj * get_next(){
    return (ctPointObj *)ctLinkList::get_next();
  }

  void add_link( ctPointObj * plink ){
    ctLinkList::add_link( (void *)plink );
  }

  void remove_link( ctPointObj * plink ){
    ctLinkList::remove_link( (void *)plink );
  }

  void delete_link( ctPointObj * plink ){
    ctLinkList::delete_link( (void *)plink );
  }

};

class ctForce;

class ctLinkList_ctForce : public ctLinkList
{
public:
  ctForce * get_first(){
    return (ctForce *)ctLinkList::get_first();
  }

  ctForce * get_next(){
    return (ctForce *)ctLinkList::get_next();
  }

  void add_link( ctForce * plink ){
    ctLinkList::add_link( (void *)plink );
  }

  void remove_link( ctForce * plink ){
    ctLinkList::remove_link( (void *)plink );
  }

  void delete_link( ctForce * plink ){
    ctLinkList::delete_link( (void *)plink );
  }

};

class ctArticulatedBody;

class ctLinkList_ctArticulatedBody : public ctLinkList
{
public:
  ctArticulatedBody * get_first(){
    return (ctArticulatedBody *)ctLinkList::get_first();
  }

  ctArticulatedBody * get_next(){
    return (ctArticulatedBody *)ctLinkList::get_next();
  }

  void add_link( ctArticulatedBody * plink ){
    ctLinkList::add_link( (void *)plink );
  }

  void remove_link( ctArticulatedBody * plink ){
    ctLinkList::remove_link( (void *)plink );
  }

  void delete_link( ctArticulatedBody * plink ){
    ctLinkList::delete_link( (void *)plink );
  }

};


class ctVector3;

class ctLinkList_ctVector3 : public ctLinkList
{
public:
  ctVector3 * get_first(){
    return (ctVector3 *)ctLinkList::get_first();
  }

  ctVector3 * get_next(){
    return (ctVector3 *)ctLinkList::get_next();
  }

  void add_link( ctVector3 * plink ){
    ctLinkList::add_link( (void *)plink );
  }

  void remove_link( ctVector3 * plink ){
    ctLinkList::remove_link( (void *)plink );
  }

  void delete_link( ctVector3 * plink ){
    ctLinkList::delete_link( (void *)plink );
  }

};

class test_body;

class ctLinkList_test_body : public ctLinkList
{
public:
  test_body * get_first(){
    return (test_body *)ctLinkList::get_first();
  }

  test_body * get_next(){
    return (test_body *)ctLinkList::get_next();
  }

  void add_link( test_body * plink ){
    ctLinkList::add_link( (void *)plink );
  }

  void remove_link( test_body * plink ){
    ctLinkList::remove_link( (void *)plink );
  }

  void delete_link( test_body * plink ){
    ctLinkList::delete_link( (void *)plink );
  }

};


#endif
