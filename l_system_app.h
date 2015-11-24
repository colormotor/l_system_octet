
#include "quick_mesh.h"
#include "common.h"
#include "l_system.h"
#include "line_renderer.h"
#include "spring_renderer.h"

using namespace octet;

class LSystemApp : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;
    ref<material> default_mtl;
    ref<QuickMesh> mesh;
    
    LineRenderer *line_renderer;
    SpringRenderer *spring_renderer;
    LsystemRenderer *renderer;
    
    Lsystem G;
    
    int n_iter = 7;
    int cur_file = 0;
    
    std::vector<std::string> files;
    
    float scale = 1.0;

    bool dirty=true; // flag indicates we need to update the scene
    
    std::map <std::string, float*> config;
    
public:
    LSystemApp(int argc, char **argv) : app(argc, argv) {
    }
    
    ~LSystemApp() {
        delete line_renderer;
        delete spring_renderer;
    }
    
    void parse_config()
    {
        std::string str = string_from_file("config.txt");
        string_vector lines = split(str,"\n");

        for( int i = 0; i < lines.size(); i++ )
        {
            std::string l = lines[i];
            string_vector p = split(l,":");
            if( in(p[0], config) )
            {
                *config[p[0]] = atof(p[1].c_str());
            }
            else
            if( p[0]=="renderer" )
            {
                if(p[1]=="line")
                    renderer = line_renderer;
                if(p[1]=="spring")
                    renderer = spring_renderer;
                else
                    printf("Unknown renderer in config: %s\n",p[1].c_str());
            }
            else
            {
                printf("Could not parse %s:%s\n",p[0].c_str(),p[1].c_str());
            }
            G.render(renderer);
        }
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
            renderer->delta = G.default_params["delta"];
        }
        
        G.produce(n_iter);
        G.render(renderer);
        mesh->calc_aabb();

    }
    
    /// this is called once OpenGL is initialized
    void app_init()
    {
        // mesh shared by renderers
        mesh = new QuickMesh(GL_LINES);
        // renderers
        line_renderer = new LineRenderer(mesh);
        spring_renderer = new SpringRenderer(mesh);
        renderer = line_renderer;
        
        // add config entries
        config["kp"] = &spring_renderer->kp;
        config["kv"] = &spring_renderer->kv;
        config["m"] = &spring_renderer->m;
        
        // read configuration
        //parse_config();
        
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
        
        app_scene->add_shape(mat, mesh, default_mtl, false);
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
            dirty = true;
            renderer->delta -= 0.03;
            printf("delta=%g\n", renderer->delta);
        }
        
        if(is_key_down('S'))
        {
            dirty = true;
            renderer->delta += 0.03;
            printf("delta=%g\n", renderer->delta);
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
                G.render(renderer);
                mesh->calc_aabb();
                break;
            }
        }
        
        if(is_key_going_up(key_tab))
        {
            parse_config();
        }
    }
    
    void transform( float w, float h )
    {
        camera_instance * cam = app_scene->get_camera_instance(0);
        
        // orthographic projection
        cam->set_ortho(w, h, 1, -1, 1);
        
        // scaling to fit
        aabb bb = mesh->get_aabb();
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
        
        if(dirty)
            G.render(renderer);
        dirty = false;
        
        //l_renderer.mesh->render();
        
        // update matrices. assume 30 fps.
        app_scene->update(1.0f/30);
        
        // draw the scene
        app_scene->render((float)vx / vy);
    }
};
