/*
  Hand made interface for scf templates.
  Swig will choke when trying to parse the scfImplementation macros
  so we use this to inform swig about their workings.
  Instance necessary templates as follows:
  %template(scfPath) scfImplementation1<csPath,iPath >;

  For the moment only scfImplementation1 and scfImplementationExt0
  are wrapped.
*/



template<class If>
class scfFakeInterface
{
};

%define SWIG_SCF_IMPL(macro_name)
	protected:
	virtual ~ ## macro_name ();
	public:
	virtual void IncRef ();
	virtual void DecRef ();
	virtual int GetRefCount ();
	virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);
	virtual void AddRefOwner (void** ref_owner, CS::Threading::Mutex*);
	virtual void RemoveRefOwner (void** ref_owner);
        scfInterfaceMetadataList* GetInterfaceMetadata ();
%enddef

template <class T,class K> class scfImplementation1 : public K
{
	SWIG_SCF_IMPL(scfImplementation1)
};

template <class T,class K,class J> class scfImplementation2 : public K, public J
{
	SWIG_SCF_IMPL(scfImplementation2)
};

template <class T,class I1,class I2,class I3> class scfImplementation3 : public I1, public I2, public I3
{
	SWIG_SCF_IMPL(scfImplementation3)
};

template <class T,class K> class scfImplementationExt0 : public K
{
	SWIG_SCF_IMPL(scfImplementationExt0)
};
template <class T,class K,class J> class scfImplementationExt1 : public K, public J
{
	SWIG_SCF_IMPL(scfImplementationExt1)
	protected:
	virtual macro_name ();
};

template <class T,class K,class J,class J2> class scfImplementationExt2 : 
public K, public J, public J2
{
	SWIG_SCF_IMPL(scfImplementationExt2)
	protected:
	virtual macro_name ();
};

template <class T,class K,class J,class J2,class J3> class scfImplementationExt3 : 
public K, public J, public J2, public J3
{
	SWIG_SCF_IMPL(scfImplementationExt3)
	protected:
	virtual macro_name ();
};


/* This ones are known (and chosen) not to be wrapped for the moment 
   if you need to use any of these delete the warnfilter and add the
   necessary %template directive. */
%warnfilter(401) csView; 		// iView
//%warnfilter(401) csGeomDebugHelper;  	// iDebugHelper
%warnfilter(401) csEventNameRegistry; 	// iEventNameRegistry
%warnfilter(401) csTinyDocumentSystem; 	// iDocumentSystem

