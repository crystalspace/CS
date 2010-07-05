#ifndef _CHCPP_H_
#define _CHCPP_H_

class csVisibilityObjectHistory :
    public scfImplementation1<csVisibilityObjectHistory, iKDTreeUserData>
{
public:
  bool bVisible;

  csVisibilityObjectHistory () :
  scfImplementationType (this), bVisible(false)
  {
  }

  virtual ~csVisibilityObjectHistory()
  {
  }
};

#endif
