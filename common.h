#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <sstream>

#include <dirent.h> // wont work on Windows

#define STRINGIFY( expr ) #expr

typedef std::vector<std::string> string_vector;
typedef std::vector<vec3> polyline;

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

// Utils:
/// Weighted random sample between a number of elements in an array
template <class T>
T weighted_sample( const std::vector<T>& X, const std::vector<float> &W )
{
    float cum = 0.0;
    for( int i = 0; i < W.size(); i++ )
        cum += W[i];
    
    float sample = drand48()*cum;
    
    for( int i = 0; i < X.size(); i++ )
    {
        cum -= W[i];
        if( cum < sample )
            return X[i];
    }
    
    // we should not be here :/
    assert(0);
    return X[0];
}

/// Python-like check if key is in dictionary
template <class A, class B>
bool in( A key, const std::map<A,B>& dict )
{
    return dict.find(key) != dict.end();
}

/// (Wrapper arounf strtok) Splits a string into tokens delimited by characters in delimiters.
std::vector<std::string> split( const std::string& str, const std::string& delimiters )
{
    char * buf = new char[str.length()+1]; // need to consider EOS
    memcpy(buf, str.c_str(), str.length()+1);
    
    std::vector<std::string> tokens;
    const char * tok = strtok(buf, delimiters.c_str());
    
    while(tok != NULL)
    {
        tokens.push_back(std::string(tok) + "");
        tok = strtok(NULL, delimiters.c_str());
    }
    
    delete [] buf;
    
    return tokens;
}

/// Returns a string between two delimiter characters a and b
std::string string_between( const std::string& str, char a, char b )
{
    size_t ia = str.find_first_of(a);
    if(ia==std::string::npos)
        return "";
    size_t ib = str.find_first_of(b);
    if(ib==std::string::npos)
        return "";
    size_t n = ib-ia;
    if(n < 2)
    {
        return "";
    }
    return str.substr(ia+1, n-1);
}

/// Extracts a string from a file
std::string string_from_file( const std::string & path )
{
    std::ifstream f(path);
    if(f.fail())
    {
        printf("Could not open file %s\n", path.c_str());
        return "";
    }

    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}


