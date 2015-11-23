
#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <sstream>

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


///////////////////////////////////////////////////////
/// Empty Lsystem renderer
class LsystemRenderer
{
public:
    virtual void begin() {}
    virtual void end() {}
    
	virtual void F() {}
	virtual void f() {}
	virtual void plus() {}
	virtual void minus() {}
	virtual void push() {}
	virtual void pop() {}
};

///////////////////////////////////////////////////////
/// Production rule for a given predecessor alpha
/// Multiple successors can be specified with weights determining the likelyhood of being samples.
struct Production
{
	char alpha;

    std::vector<float> weights;
	std::vector<std::string> successors;

	std::string successor()
	{
		// return same in case
		if( successors.size() < 1 )
		{
			return std::string(1,alpha);
		}
        
        return weighted_sample(successors, weights);
	}
};


///////////////////////////////////////////////////////
/// L-system meat
class Lsystem
{
public:
    typedef std::vector<std::string> string_vector;
    
	// Override these Graphics system specific
	void F() { renderer->F(); }
	void f() { renderer->f(); }
	void plus() { renderer->plus(); }
	void minus() { renderer->minus(); }
	void push() { renderer->push(); }
	void pop() { renderer->pop(); }
    
    virtual LsystemRenderer * get_renderer() { return renderer; }
    virtual void set_renderer( LsystemRenderer * renderer_ )
    {
        renderer = renderer_;
    }
    
	Lsystem()
	{
		renderer = &emptyRenderer;

        // create our alphabet
		alphabet['F'] = std::bind(&Lsystem::F, this);
		alphabet['f'] = std::bind(&Lsystem::f, this);
		alphabet['+'] = std::bind(&Lsystem::plus, this);
		alphabet['-'] = std::bind(&Lsystem::minus, this);
		alphabet['['] = std::bind(&Lsystem::push, this);
		alphabet[']'] = std::bind(&Lsystem::pop, this);
        
        // additional symbols can be added by overriding this class
        // and adding entries with corresponding functions, and overriding
        // get_renderer/set_renderer with an apporpriate extended renderer.
	}
    
    bool parse_file( const std::string & path )
    {
        std::ifstream f(path);
        if(f.fail())
        {
            printf("Could not open file %s\n", path.c_str());
            return false;
        }
        
        std::stringstream buffer;
        buffer << f.rdbuf();
        return parse(buffer.str());
    }
    
    /// Reads optional parameters from the lines of a configuration.
    /// If parameters are found (between {} brackets),
    /// A new array is returned with the parameter specification line removed
    string_vector check_params( const string_vector& lines_ )
    {
        string_vector lines = lines_;
        
        default_params.clear();
        
        // optionally the first line can specify default l-system parameters
        std::string params = string_between(lines[0],'{','}');
        
        // if not present bail
        if(params=="")
            return lines;
        
        // otherwise parse and store in default_params dict.
        string_vector P = split(params, ",");
        for( int i = 0; i < P.size(); i++ )
        {
            string_vector p = split(P[i],":");
            if( p.size() != 2 )
                continue;
            default_params[p[0]] = atof(p[1].c_str());
        }
        
        // as nothing happened
        lines.erase(lines.begin());
        return lines;
    }
    
    /// Parse a configuration string
	bool parse( const std::string& str )
	{
        clear();
        
		std::vector<std::string> lines = split(str,"\n");
		if( lines.size() < 2 )
		{
			printf("Incomplete specification!\n");
			return false;
		}
        
        lines = check_params(lines);
        
        axiom = lines[0];

		P.clear();

		for( int i = 1; i < lines.size(); i++ )
		{
			if(!parse_production(lines[i]))
			{
				printf("Error in line %d\n",i);
			}
		}

		return true;
	}
    
    /// Parse a production string
	bool parse_production( const std::string& str )
	{
		std::vector<std::string> prod = split(str,":");
		if( prod.size() != 2 )
		{
			printf("Invalid production rule %s \n", str.c_str());
			return false;
		}

		std::string a_str = prod[0];
		std::string chi = prod[1];
		
		char alpha = a_str[0];
		
		float w = 1.0;
		
		// parse probability if present
		std::string weight_str = string_between(a_str,'(',')');
		if(weight_str!="")
			w = atof(weight_str.c_str());
		
		// add or update proudction
		if( in(alpha, P) )
		{
			Production & p = P[alpha];
			p.successors.push_back(chi);
			p.weights.push_back(w);	
		}
		else
		{
			Production p;
			p.successors.push_back(chi);
			p.weights.push_back(w);	
			P[alpha] = p;
		}
        
        return true;
	}
    
    /// Run a production iteration on string
	std::string produce( const std::string & str )
	{
        std::string res = "";
        
		for( int i = 0; i < str.length(); i++ )
		{
            char alpha = str[i];
            
            // skip whitespaces tabs etc..
            if( !isalnum(alpha) && !ispunct(alpha) )
                continue;
            
            if(in(alpha, P))
            {
                Production & e = P[str[i]];
                res += e.successor();
            }
            else
            {
                res += std::string(1, alpha);
            }
		}
        
		return res;
	}
    
    /// Run n iterations starting from the axiom
	void produce( int n )
	{
		std::string str = axiom;
        for( int i = 0; i < n; i++ )
        {
            //printf("%d: %s -->", i+1, str.c_str());
			str = produce(str);
            //printf("%s\n", str.c_str());
        }
        
		l_system.clear();

		for( int i = 0; i < str.length(); i++ )
		{
			if(in(str[i], alphabet))
			{
				l_system.push_back( alphabet[str[i]] );
			}
			else
			{
				//printf("%c not in alphabet!\n", str[i]);
			}
		}
	}

    /// Render parsed L-system
	void render()
	{
        renderer->begin();
		for( int i = 0; i < l_system.size(); i++ )
			l_system[i]();
        renderer->end();
	}
    
    void clear()
    {
        l_system.clear();
        P.clear();
    }
    
    bool has_default_param( const std::string & str )
    {
        return in(str, default_params);
    }
    
    float get_default_param( const std::string& str )
    {
        return default_params[str];
    }
    
	LsystemRenderer * renderer;
	LsystemRenderer emptyRenderer;

	std::vector< std::function<void()> > l_system;
	
	std::string axiom;
    
	std::map<char, Production> P;
	std::map<char, std::function<void()> > alphabet;
    
    std::map<std::string, float> default_params;
};

