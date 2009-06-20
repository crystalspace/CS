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

#undef CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER
%define CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER
%ignore csTriangleMeshTools::CalculateOutline(csTriangleMeshEdge* edges, size_t num_edges,csPlane3* planes, size_t num_vertices,const csVector3& pos,size_t* outline_edges, size_t& num_outline_edges,bool* outline_verts,float& valid_radius);
%ignore csTriangleMeshTools::SortTrianglesX (iTriangleMesh* trimesh,csTriangleMinMax*& tris, size_t& tri_count,csPlane3*& planes);
%ignore csTriangleMeshTools::CalculateEdges(iTriangleMesh*, size_t& num_edges);
%extend csTriangleMeshTools {
	static void CalculateEdges(iTriangleMesh* edges, size_t& num_edges,csTriangleMeshEdge* results,size_t & num_results) {
		results = csTriangleMeshTools::CalculateEdges(edges,num_edges);
		num_results = num_edges;
	}
	static void CalculateOutline(csTriangleMeshEdge* edges, size_t num_edges,csPlane3* planes, size_t num_vertices,const csVector3& pos,size_t* outline_edges, size_t& num_outline_edges,bool* outline_verts,size_t & num_outline_verts,float& valid_radius) {
		csTriangleMeshTools::CalculateOutline(edges,num_edges,planes,num_vertices,pos,outline_edges,num_outline_edges,outline_verts,valid_radius);
		num_outline_verts = num_outline_edges;
	}
	static void SortTrianglesX (iTriangleMesh* trimesh,csTriangleMinMax** tris, size_t& tri_count,csPlane3** planes,size_t & plane_count) {
		csTriangleMeshTools::SortTrianglesX(trimesh,*tris,tri_count,*planes);
		plane_count = tri_count;
	}
}
%enddef
CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER
#undef CS_TRIANGLE_MESH_EDGE_TOOLS_WRAPPER

%ignore csSimpleRenderMesh::vertices;