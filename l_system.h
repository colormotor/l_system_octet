#pragma once

/// Float value wrapper. allows to randomly select from a set of values.
class FloatParam
{
public:
    FloatParam( float v = 0.0 )
    {
        values.push_back(v);
    }
    
    FloatParam(const std::string &value)
    {
        string_vector S = split(value,",");
        if(S.size() == 0)
            S.push_back(value);
        for( int i=0; i < S.size(); i++ )
            values.push_back(atof(S[i].c_str()));
    }
    
    operator float() const
    {
        if(!values.size())
            return 0.0;
        
        return values[ rand()%values.size() ];
    }
    
    const FloatParam & operator = (float v)
    {
        values.clear();
        values.push_back(v);
    }
    
    std::vector<float> values;
};

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
    
    FloatParam delta=20.0; // Angle, the FloatParam allows for randomization
    float delta_offset=0.0;
    int d=1;
};


///////////////////////////////////////////////////////
/// Production rule for a given predecessor a
/// Multiple successors can be specified with weights determining the likelyhood of being samples.
struct Production
{
	char a;

    std::vector<float> weights;
	std::vector<std::string> successors;

	std::string successor()
	{
		// return same in case
		if( successors.size() < 1 )
		{
			return std::string(1,a);
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
    
    /// Parses an l-system description file
    bool parse_file( const std::string & path )
    {
        std::string str = string_from_file(path);
        if(str=="")
            return false;
        return parse(str);
    }
    
    /// Reads optional parameters from the lines of an l-system description.
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
        string_vector P = split(params, ";");
        for( int i = 0; i < P.size(); i++ )
        {
            string_vector p = split(P[i],":");
            if( p.size() != 2 )
                continue;
            default_params[p[0]] = FloatParam(p[1].c_str());
        }
        
        // as nothing happened
        lines.erase(lines.begin());
        return lines;
    }
    
    /// Parse an L-System specification string
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
        
		if( prod.size() < 1 )
		{
			printf("Invalid production rule %s \n", str.c_str());
			return false;
		}

		std::string a_str = prod[0];
        std::string chi = "";
        if( prod.size() > 1 )
            chi = prod[1];
		
		char a = a_str[0];
		
		float w = 1.0;
		
		// parse probability if present
		std::string weight_str = string_between(a_str,'(',')');
		if(weight_str!="")
			w = atof(weight_str.c_str());
		
		// add or update proudction
		if( in(a, P) )
		{
			Production & p = P[a];
			p.successors.push_back(chi);
			p.weights.push_back(w);	
		}
		else
		{
			Production p;
			p.successors.push_back(chi);
			p.weights.push_back(w);	
			P[a] = p;
		}
        
        return true;
	}
    
    /// Run a production iteration on string
	std::string produce( const std::string & str )
	{
        std::string res = "";
        
		for( int i = 0; i < str.length(); i++ )
		{
            char a = str[i];
            
            // skip whitespaces tabs etc..
            if( !isalnum(a) && !ispunct(a) )
                continue;
            
            if(in(a, P))
            {
                Production & e = P[str[i]];
                res += e.successor();
            }
            else
            {
                res += std::string(1, a);
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

    /// Render parsed L-system with a given renderer
	void render( LsystemRenderer * renderer )
	{
        renderer->begin();
		for( int i = 0; i < l_system.size(); i++ )
			l_system[i](renderer);
        renderer->end();
	}
    
    // Clear the L-System
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
    
    std::map<std::string, FloatParam> default_params;
};

