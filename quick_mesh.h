#pragma once
namespace octet
{
namespace scene
{
    
/// A mesh class for making prcedural geometry of various kinds.
/// Note: breaks with bullet.
    
class QuickMesh : public mesh {
    dynarray<vec3p> verts;
    
    void init( GLenum primitive ) {
        set_default_attributes();
        set_params(32, 0, 0, primitive, 0);
        //update();
    }
    
public:
//    RESOURCE_META(QuickMesh)
    
    /// make a new, empty mesh.
    QuickMesh(  GLenum primitive )
    {
        init(primitive);
    }
    
    void clear()
    {
        verts.reset();
    }

    /// add a point to the mesh.
    void vertex(vec3_in pos)
    {
        verts.push_back(pos);
    }
    
    /// Build the OpenGL geometry.
    void update()
    {
        allocate(sizeof(mesh::vertex)*verts.size(), 0);
        
        gl_resource::wolock vtx_lock(get_vertices());
        mesh::vertex *vtx = (mesh::vertex *)vtx_lock.u8();
        
        for (unsigned i = 0; i != verts.size(); ++i) {
            vtx->pos = verts[i];
            vtx->normal = vec3p(0, 0, 1);
            vtx->uv = vec2(0, 0);
            vtx++;
        }
        
        set_num_indices(0);
        set_num_vertices(verts.size());
    }
    
    /// Serialize.
    void visit(visitor &v) {
        mesh::visit(v);
        v.visit(verts, atom_point);
    }
};

}
}
