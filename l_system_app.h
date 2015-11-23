#include "quick_mesh.h"
#include "l_system.h"
#include <dirent.h> // wont work on Windows

using namespace octet;

/// Lists files in a directory
std::vector<std::string> files_in_directory( const std::string& path )
{
    struct dirent *de=NULL;
    DIR *d=NULL;
    
    std::string p = path;
    p+="/";
    
    d=opendir(path.c_str());
    
    std::vector<std::string> files;
    if(d == NULL)
    {
        printf("Couldn't open directory\n");
        return files;
    }
    
    // Loop while not NULL
    while(de = readdir(d))
    {
        if(de->d_type==DT_REG)
        {
            std::string name = de->d_name;// de->d_type
            
            if(name!=".DS_Store")
            {
                files.push_back( p+name );
            }
            
        }
    }
    
    closedir(d);
    return files;
}

class LSystemLineRenderer : public LsystemRenderer
{
public:
    LSystemLineRenderer()
    {
        stack.push_back(mat4t());
        mesh = new QuickMesh(GL_LINES);
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
        mesh->vertex(mat().w().xyz());
        f();
        mesh->vertex(mat().w().xyz());
    }
    
    void f() 
    {
        // octet vectors are rows
        mat() = mat4t().translate(0, d, 0) * mat();
    }
    
    void plus() 
    {
        mat() = mat4t().rotateZ(-delta) * mat();
    }
    
    void minus() 
    {
        mat() = mat4t().rotateZ(+delta) * mat();
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
 
                     
    ref<QuickMesh> mesh;
    std::vector< mat4t > stack;
    float delta=20.0; // <- degrees
    float d=1;
};


class LSystemApp : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;
    ref<material> default_mtl;
    ref<QuickMesh> mesh;
    
    LSystemLineRenderer l_renderer;
    Lsystem G;
    
    int n_iter = 7;
    int cur_file = 0;
    
    std::vector<std::string> files;
    
    float scale = 1.0;
    
public:
    LSystemApp(int argc, char **argv) : app(argc, argv) {
    }
    
    ~LSystemApp() {
    }
    
    void reload()
    {
        printf("Loading %s\n",files[cur_file].c_str());
        G.parse_file(files[cur_file]);
        
        if( G.has_default_param("n") )
        {
            n_iter = G.default_params["n"];
        }
        
        if( G.has_default_param("delta") )
        {
            l_renderer.delta = G.default_params["delta"];
        }
        
        G.produce(n_iter);
        G.set_renderer(&l_renderer);
        G.render();
        l_renderer.compute_aabb();
    }
    
    /// this is called once OpenGL is initialized
    void app_init()
    {
        // List all files in data dir.
        files = files_in_directory("./data");
        // load first one
        reload();
        
        app_scene =  new visual_scene();
        app_scene->create_default_camera_and_lights();
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 4, 0));
        
        default_mtl = new material(vec4(0, 0, 0, 0.8));
        
        mat4t mat;
        mat.loadIdentity();
        
        app_scene->add_shape(mat, l_renderer.mesh, default_mtl, false);
    }
    
    void keyboard()
    {
        if(is_key_down('Z'))
        {
            scale -= 0.1;
            printf("scale=%g\n", scale);
        }

        if(is_key_down('X'))
        {
            scale += 0.1;
            printf("scale=%g\n", scale);
        }

        if(is_key_down('A'))
        {
            l_renderer.delta -= 0.03;
            printf("delta=%g\n", l_renderer.delta);
        }
        
        if(is_key_down('S'))
        {
            l_renderer.delta += 0.03;
            printf("delta=%g\n", l_renderer.delta);
        }
        
        if(is_key_going_up(key_up))
        {
            cur_file = (cur_file+1)%files.size();
            reload();
        }
        
        if(is_key_going_up(key_down))
        {
            cur_file = cur_file-1;
            if( cur_file < 0 )
                cur_file += files.size();
            reload();
        }
        
        if(is_key_going_up(' '))
        {
            reload();
        }
        
        for( int i = 0; i < 9; i++ )
        {
            if(is_key_going_up(i+48))
            {
                printf("Producing with %d iterations\n", i);
                n_iter = i;
                G.produce(n_iter);
                G.render();
                l_renderer.compute_aabb();
                break;
            }
        }
    }
    
    void transform( float w, float h )
    {
        camera_instance * cam = app_scene->get_camera_instance(0);
        
        // orthographic projection
        cam->set_ortho(w, h, 1, -1, 1);
        
        // scaling to fit
        aabb bb = l_renderer.mesh->get_aabb();
        float boxw = bb.get_max().x() - bb.get_min().x();
        float boxh = bb.get_max().y() - bb.get_min().y(); //:S
        float ratio = std::max(boxw/w, boxh/h);
    
        // center
        cam->get_node()->loadIdentity();
        cam->get_node()->translate(bb.get_center());
        cam->get_node()->scale(vec3(ratio*scale, ratio*scale, 1));

    }
    
    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h)
    {
        int vx = 0, vy = 0;
        get_viewport_size(vx, vy);
        
        app_scene->begin_render(w, h, vec4(0.99, 0.998, 1, 1));
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        transform(vx, vy);
        keyboard();
        
        G.render();
        // update matrices. assume 30 fps.
        app_scene->update(1.0f/30);
        
        // draw the scene
        app_scene->render((float)vx / vy);
    }
};
