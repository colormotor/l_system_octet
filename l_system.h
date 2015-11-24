#pragma once

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
    
    float delta=20.0; // <- degrees
    float d=1;
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
	// Override these Graphics system specific
	void F( LsystemRenderer * renderer ) { renderer->F(); }
	void f( LsystemRenderer * renderer ) { renderer->f(); }
	void plus( LsystemRenderer * renderer ) { renderer->plus(); }
	void minus( LsystemRenderer * renderer ) { renderer->minus(); }
	void push( LsystemRenderer * renderer ) { renderer->push(); }
	void pop( LsystemRenderer * renderer ) { renderer->pop(); }
    
	Lsystem()
	{
        // create our alphabet
        alphabet['F'] = std::bind(&Lsystem::F, this, std::placeholders::_1 );
		alphabet['f'] = std::bind(&Lsystem::f, this, std::placeholders::_1 );
		alphabet['+'] = std::bind(&Lsystem::plus, this, std::placeholders::_1 );
		alphabet['-'] = std::bind(&Lsystem::minus, this, std::placeholders::_1 );
		alphabet['['] = std::bind(&Lsystem::push, this, std::placeholders::_1 );
		alphabet[']'] = std::bind(&Lsystem::pop, this, std::placeholders::_1 );
        
        // additional symbols can be added by overriding this class
        // and adding entries with corresponding functions, and overriding
        // get_renderer/set_renderer with an apporpriate extended renderer.
	}
    
    bool parse_file( const std::string & path )
    {
        std::string str = string_from_file(path);
        if(str=="")
            return false;
        return parse(str);
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
	void render( LsystemRenderer * renderer )
	{
        renderer->begin();
		for( int i = 0; i < l_system.size(); i++ )
			l_system[i](renderer);
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
    
	std::vector< std::function<void(LsystemRenderer*)> > l_system;
	
	std::string axiom;
    
	std::map<char, Production> P;
	std::map<char, std::function<void(LsystemRenderer*)> > alphabet;
    
    std::map<std::string, float> default_params;
};

