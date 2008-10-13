#undef JAVA_IEVENT_WRAPPER
%define JAVA_IEVENT_WRAPPER
%rename(AddInt64) iEvent::Add(const char *, int64);
%rename(AddUInt64) iEvent::Add(const char *, uint64);
%rename(RetrieveInt64) iEvent::Retrieve(const char *, int64 &) const;
%rename(RetrieveUInt64) iEvent::Retrieve(const char *, uint64 &) const;
// Method GetName is used, no direct access needed.
%ignore iEvent::Name;
%ignore iEvent::Add;
%ignore iEvent::Retrieve;
%extend iEvent 
{
	bool AddString(const char *name, const char *v)
	{
		return self->Add(name,v);
	}
	bool AddEvent(const char *name, iEvent *v)
	{
		return self->Add(name,v);
	}
	bool AddBase(const char *name, iBase *v)
	{
		return self->Add(name,v);
	}
	bool AddByteArray(const char *name, const void *v, size_t size)
	{
		return self->Add(name,v,size);
	}
	csEventError RetrieveEvent(const char *name, csRef<iEvent> & v)
	{
		return self->Retrieve(name,v);
	}
	csEventError RetrieveBase(const char *name, csRef<iBase> & v)
	{
		return self->Retrieve(name,v);
	}
	csEventError RetrieveByteArray(const char *name, const void *&v, size_t& size)
	{
		return self->Retrieve(name,v,size);
	}
}
%enddef
JAVA_IEVENT_WRAPPER
#undef JAVA_IEVENT_WRAPPER

#undef OUTLINE_EDGES_CONTAINER
%define OUTLINE_EDGES_CONTAINER
%typemap(javacode) csOutlineEdgesContainer
%{
	public final long [][] outline_edges = new long[1][0];
	public final boolean [][] outline_verts = new boolean[1][0];
%}
%inline 
%{
	struct csOutlineEdgesContainer {};
%}
%enddef
OUTLINE_EDGES_CONTAINER
#undef OUTLINE_EDGES_CONTAINER

#undef CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER
%define CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER
%ignore csTriangleMeshTools::CalculateEdges(iTriangleMesh*, size_t& num_edges);
%extend csTriangleMeshTools {
	static void CalculateEdges(iTriangleMesh* edges, size_t& num_edges,csTriangleMeshEdge* results,size_t & num_results) {
		results = csTriangleMeshTools::CalculateEdges(edges,num_edges);
		num_results = num_edges;
	}
}
%enddef
CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER
#undef CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER
