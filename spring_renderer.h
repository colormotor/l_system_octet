#pragma once

typedef std::vector<vec3> polyline;

vec3 interpolate_polyline( const polyline & P, float t )
{
    if( t < 0.0 )
        return P[0];
    if( t > 1.0 )
        return P.back();
    
    int nsegs = P.size()-1;
    float param_len = 1.0 / nsegs;
    int iseg = (int)(t*nsegs);
    t = (t - iseg*param_len) / param_len;
    float t1 = 1.0 - t;
    return P[iseg]*t1 + P[iseg+1]*t;
}

polyline mass_spring_path( const polyline & P, float m=0.1, float kp=70, float kv=30 )
{
    vec3 v = vec3(0,0,0);
    vec3 a = vec3(0,0,0);
    vec3 f = vec3(0,0,0);
    
    vec3 p = P[0];
    
    polyline res;
    int n = 200;
    float dt = 1.0/n;
    for( int i = 0; i < n; i++ )
    {
        vec3 ph = interpolate_polyline(P, (float)i / 200);
        f = m*a - v*kv + (ph-p)*kp;
        a = f;
        v += f*dt;
        p += v*dt;
        res.push_back(p);
    }
    
    return res;
}

polyline spring_path( const polyline & P, float kp=70, float kv=30 )
{
    vec3 v = vec3(0,0,0);
    vec3 a = vec3(0,0,0);
    vec3 f = vec3(0,0,0);
    
    vec3 p = P[0];
    
    polyline res;
    int n = 200;
    float dt = 1.0/n;
    for( int i = 0; i < n; i++ )
    {
        vec3 ph = interpolate_polyline(P, (float)i / 200);
        f = (ph-p)*kp - v*kv;
        v += f*dt;
        p += v*dt;
        res.push_back(p);
    }
    
    return res;
}


class SpringRenderer : public LsystemRenderer
{
public:
    
    SpringRenderer( QuickMesh * mesh )
    :
    mesh(mesh)
    {
        mesh = new QuickMesh(GL_LINES);
    }
    
    ~SpringRenderer()
    {
        if(tree)
            delete tree;
    }
    
    struct Node
    {
        Node( const vec3 & pos=vec3(0,0,0) )
        :
        pos(pos)
        {
        }
        
        ~Node()
        {
            for( int i = 0; i < children.size(); i++ )
            {
                delete children[i];
            }
            children.clear();
        };
        
        void add_child( Node * n )
        {
            children.push_back(n);
            n->parent = this;
        }
        
        vec3 pos;
        std::vector<Node*> children;
        Node * parent = 0;
    };
    
    Node * add_node()
    {
        Node * n = new Node(pos());
        nodes.push_back(n);
        node_stack.back()->add_child(n);
        node_stack.back() = n;
        return n;
    }
    
    void begin()
    {
        nodes.clear();
        
        if(tree)
            delete tree;
        
        tree = new Node();
        
        node_stack.clear();
        node_stack.push_back(tree);
        
        stack.clear();
        stack.push_back(mat4t());
    }
    
    void add_polyline( const polyline & P )
    {
        // reversed since we start from tree root
        for( int i = 0; i < P.size()-1; i++ )
        {
            mesh->vertex(P[i]);
            mesh->vertex(P[i+1]);
        }
    }
    
    void end()
    {
        // find leaf nodes
        std::vector<Node*> leafs;
        for( int i = 0; i < nodes.size(); i++ )
        {
            if( nodes[i]->children.size() == 0 )
                leafs.push_back(nodes[i]);
        }
        
        mesh->clear();
        for( int i = 0; i < leafs.size(); i++ )
        {
            Node * n = leafs[i];
            polyline P;
            while(n->parent != 0)
            {
                
                P.push_back(n->pos);
                P.push_back(n->parent->pos);
                n = n->parent;
            }
            
            // reverse because we begin from root
            std::reverse(P.begin(),P.end());
            
            add_polyline(mass_spring_path(P, m, kp, kv));
        }
        mesh->update();
    }
    
    void compute_aabb()
    {
        mesh->calc_aabb();
    }
    
    void F()
    {
        // Todo handle pen up pen down
        f();
    }
    
    void f()
    {
        mat() = mat4t().translate(0, d, 0) * mat();
        add_node();
    }
    
    void plus()
    {
        mat() = mat4t().rotateZ(+delta) * mat();
    }
    
    void minus()
    {
        mat() = mat4t().rotateZ(-delta) * mat();
    }
    
    void push()
    {
        stack.push_back(stack.back());
        node_stack.push_back(node_stack.back());
    }
    
    void pop()
    {
        if(stack.size() < 2)
        {
            printf("Error, stack underflow!\n");
            return;
        }
        
        stack.pop_back();
        node_stack.pop_back();
    }
    
    mat4t & mat() { return stack.back(); }
    const mat4t & mat() const { return stack.back(); }
    
    vec3 pos() const { return mat().w().xyz(); }
    
    void render()
    {
        
    }
    
    QuickMesh* mesh;
    
    std::vector< mat4t > stack;
    std::vector< Node* > node_stack;
    
    float kp=70.0;
    float kv=30.0;
    float m=0.1;
    
    std::vector<Node*> nodes;
    Node *tree=0;
    Node * curNode;
};
