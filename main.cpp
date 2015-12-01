////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Text overlay
//

#pragma warning(disable : 4267)

#include "../../octet.h"
using namespace octet;
using namespace octet::scene;

#include "l_system_app.h"

int main(int argc, char **argv)
{
    // set up the platform.
    octet::app::init_all(argc, argv);
    
    // our application.
    LSystemApp app(argc, argv);
    app.init();
    
    // open windows
    octet::app::run_all_apps();
}


