// cuscomp.h: interface for the CustomComponent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUSCOMP_H__C8C58B44_6906_4474_B1E2_37F085A11DC7__INCLUDED_)
#define AFX_CUSCOMP_H__C8C58B44_6906_4474_B1E2_37F085A11DC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "iaws/awsecomp.h"

class CustomComponent : public awsEmbeddedComponent  
{
public:
  CustomComponent();
  ~CustomComponent();

  SCF_DECLARE_IBASE;

  void OnDraw(csRect clip);
  const char* Type();
  virtual bool Setup(iAws* manager, awsComponentNode* settings);
};


class CustomComponentFactory : public awsEmbeddedComponentFactory
{
public:
  CustomComponentFactory(iAws *manager);

  SCF_DECLARE_IBASE;

  iAwsComponent* Create();
};


#endif // !defined(AFX_CUSCOMP_H__C8C58B44_6906_4474_B1E2_37F085A11DC7__INCLUDED_)
