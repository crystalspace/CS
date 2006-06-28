/*
  Hand made interface for scf templates.
  Swig will choke when trying to parse the scfImplementation macros
  so we use this to inform swig about their workings.
  Instance necessary templates as follows:
  %template(scfPath) scfImplementation1<csPath,iPath >;

  For the moment only scfImplementation1 and scfImplementationExt0
  are wrapped.
*/

%define SWIG_SCF_IMPL(macro_name)
	protected:
	virtual ~ ## macro_name ();
	public:
	virtual void IncRef ();
	virtual void DecRef ();
	virtual int GetRefCount ();
	virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);
	virtual void AddRefOwner (void** ref_owner);
	virtual void RemoveRefOwner (void** ref_owner);
%enddef

template <class T,class K> class scfImplementation1 : public K
{
	SWIG_SCF_IMPL(scfImplementation1)
};

template <class T,class K> class scfImplementationExt0 : public K
{
	SWIG_SCF_IMPL(scfImplementationExt0)
};

