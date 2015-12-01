#pragma once


class LineRenderer : public LsystemRenderer
{
public:
    LineRenderer( QuickMesh * mesh )
    :
    mesh(mesh)
    {
        stack.push_back(mat4t());
    }
    
    void begin()
    {
        mesh->clear();
        stack.clear();
        stack.push_back(mat4t());
    }
    
    void end()
    {
        mesh->update();
        //compute_aabb();
    }
    
    void compute_aabb()
    {
        mesh->calc_aabb();
    }
    
    void F()
    {
        mesh->vertex(vec4(0,0,0,1)*mat()); // or optionally mat().w().xyz());
        f();
        mesh->vertex(vec4(0,0,0,1)*mat());// mat().w().xyz());
    }
    
    void f()
    {
        // octet vectors are rows
        mat() = mat4t().translate(0, d, 0) * mat();
    }
    
    void plus()
    {
        mat() = mat4t().rotateZ(+(delta+delta_offset)) * mat();
    }
    
    void minus()
    {
        mat() = mat4t().rotateZ(-(delta+delta_offset)) * mat();
    }
    
    void push()
    {
        stack.push_back(stack.back());
    }
    
    void pop()
    {
        if(stack.size() < 2)
        {
            printf("Error, stack underflow!\n");
            return;
        }
        
        stack.pop_back();
    }
    
    mat4t & mat() { return stack.back(); }
    const mat4t & mat() const { return stack.back(); }
    
    QuickMesh* mesh;
    
    std::vector< mat4t > stack;
};

