/********************************************************************
 --------------------------------------------------------------------
 --           _,(_)._
 --      ___,(_______).      ____
 --    ,'__.           \    /\___\-.
 --   /,' /             \  /  /     \
 --  | | |              |,'  /       \
 --   \`.|                  /    ___|________
 --    `. :           :    /     COLORMOTOR
 --      `.            :.,'        Graphics and Multimedia Framework
 --        `-.________,-'          © Daniel Berio
 --								   http://www.enist.org
 --								   drand48@gmail.com
 --
 --------------------------------------------------------------------
 ********************************************************************/

#pragma once
#include "../../octet.h"

using namespace octet;

class EpsFile
{
public:

	EpsFile();
	~EpsFile();
	
	bool	open( const std::string & fname);
	void	close();
	
	// usually:
    // open(fname);
	// header();
	// newpath();
	// drawstuff...
	// stroke(); or fill();
	// showpage();
    // close();
    
	void		shape( const polyline & s, bool closed=false );
	void 		fillStroke();
		
	/////////////////////////////////////////////////////
	void		header(const char *autor = "ENIST", int x0 = 0, int y0 = 0, int x1 = 4000, int y1 = 3000 );
	
	/////////////////////////////////////////////////////
	void		newpath();

	/////////////////////////////////////////////////////
	void		closepath();

	/////////////////////////////////////////////////////
	void		setlinewidth( float w );

	/////////////////////////////////////////////////////
	void		setrgb( float r, float g, float b );

	/////////////////////////////////////////////////////
	void		setrgb( const vec4 & clr );

	/////////////////////////////////////////////////////
	void		setcmyk(float c, float m, float y, float k );
	/////////////////////////////////////////////////////
	void		circle( const vec3 & pos, float radius );
	/////////////////////////////////////////////////////
	void		rect( float x, float y, float w, float h );
	/////////////////////////////////////////////////////
	void 		text( const vec3 & pos, float size, const std::string & str );

	/////////////////////////////////////////////////////
	void		gray( float g );

	/////////////////////////////////////////////////////
	void		moveto( const vec3 & v );
	void		moveto( float x, float y );

	/////////////////////////////////////////////////////
	void		lineto( float x, float y );
	void		lineto( const vec3 & v );
	/////////////////////////////////////////////////////
	void		arrowto( float x, float y );
	void		arrowto( const vec3 & v );
	/////////////////////////////////////////////////////
	void		rlineto( float x, float y );
	void		rlineto( const vec3 & v );

	/////////////////////////////////////////////////////
	void		stroke();
	/////////////////////////////////////////////////////
	void		fill();
	/////////////////////////////////////////////////////
	void		showpage();
	/////////////////////////////////////////////////////
	void		gsave();
	/////////////////////////////////////////////////////
	void		grestore();
	/////////////////////////////////////////////////////
	void		setdash(float d0, float d1, float offset);
	/////////////////////////////////////////////////////
	void		setdash(float d0, float d1, float d2, float d3, float offset);

	/////////////////////////////////////////////////////
	void		translate( float x, float y );
	/////////////////////////////////////////////////////
	void		scale( float x, float y );
	/////////////////////////////////////////////////////
	void 		rotate( float ang );

	void 		setStrokeColor();
	void 		setFillColor();

	void 		strokeGray( float v );
	void 		strokeRgb( float r, float g, float b );
	void 		strokeRgb( const vec4 & clr );
	void 		strokeCmyk( float c, float m, float y, float k );
	void 		fillGray( float v );
	void 		fillRgb( float r, float g, float b );
	void 		fillRgb( const vec4 & clr );
	void 		fillCmyk( float c, float m, float y, float k );
	void 		fillNone();
	void 		strokeNone();


	//// New higher level funcs
	void 		strokeShape( const polyline & s, const vec4 & clr=vec4(0,0,0,1) );
	void 		fillShape( const polyline & s, const vec4 & clr=vec4(0,0,0,1) );
	
    void		strokeCircle( const vec3 & center, float radius, const vec4 & clr );
    void		fillCircle( const vec3 & center, float radius, const vec4 & clr );
    
    void		strokeRect( float x, float y, float w, float h, const vec4 & clr );
    void		fillRect( float x, float y, float w, float h, const vec4 & clr );
    
    void		drawLine( const vec3 & a, const vec3 & b, const vec4 & clr );
    void		drawArrow( const vec3 & a, const vec3 & b, float size, const vec4 & clr );
    
    
	/////////////////////////////////////////////////////
	// TODO: translate rotate reset etc. 
	//       function and var definition.

	/////////////////////////////////////////////////////
	// print to file.
	void  print(const char *pFormat,... );

	bool isOpen() const { return _file != 0; }
	FILE * getFile() { return _file; }
protected:
	enum
	{
		NONE = -1,
		RGB=0,
		CMYK=1,
		GRAY
	};

	int fillType;
	float fillColor[4];
	int strokeType;
	float strokeColor[4];

	FILE * _file;

	float makey( float y) const;
	float makex( float x) const;

};


/// Impl

#define P	fprintf
#define SCALE 1.0

static const char * arrowCode = STRINGIFY(
                                          
                                          /arrowto { \n
                                              << \n
                                              /tipy 3 -1 roll \n
                                              /tipx 5 -1 roll \n
                                              /tailx currentpoint \n
                                              /taily exch \n
                                              /tip 22.5 cos 22.5 sin div \n
                                              /headwidth 7 \n
                                              >> \n
                                              begin \n
                                              /dx tipx tailx sub def \n
                                              /dy tipy taily sub def \n
                                              /angle dy dx atan def \n
                                              /arrowlength \n
                                              dx dx mul dy dy mul add sqrt \n
                                              def \n
                                              /tiplength \n
                                              tip currentlinewidth 2 div mul \n
                                              def /base \n
                                              arrowlength tiplength sub \n
                                              def \n
                                              /headlength \n
                                              tip headwidth mul neg \n
                                              def \n
                                              gsave \n
                                              currentpoint translate \n
                                              angle rotate \n
                                              base 0 translate \n
                                              0 0 lineto stroke \n
                                              tiplength 0 translate \n
                                              0 0 moveto \n
                                              currentlinewidth 2 div dup scale \n
                                              headlength headwidth lineto \n
                                              headlength tip add 1 \n
                                              headlength tip add -1 \n
                                              headlength headwidth neg curveto \n
                                              closepath \n
                                              fill \n
                                              grestore \n
                                              tipx tipy moveto \n
                                              end \n
                                          } def \n
                                          
                                          );


EpsFile::EpsFile()
{
    _file = NULL;
    
    strokeGray(0);
    fillGray(0);
}

EpsFile::~EpsFile()
{
    if(_file)
        fclose(_file);
}


bool	EpsFile::open(const std::string & fname)
{
    close();
    
    _file = fopen(fname.c_str(),"w");
    if(_file == NULL)
        return false;
    return true;
}

void EpsFile::close()
{
    
    if(_file)
    {
        showpage();
        fclose(_file);
    }
    _file = 0;
}
// usually:
// header();
// newpath();
// drawstuff...
// stroke(); or fill();
// showpage();
void	EpsFile::shape( const polyline & s, bool closed )
{
    if(!_file)
        return;
    
    newpath();
    
    for( int j = 0; j < s.size(); j++ )
    {
        const vec3 & p = s[j];
        if( j == 0 )
            moveto(p.x(),p.y());
        else
            lineto(p.x(),p.y());
    }
    if(closed || fillType!= NONE)
        closepath();
    
    fillStroke();
}

void EpsFile::fillStroke()
{
    if( fillType != NONE )
    {
        gsave();
        setFillColor();
        fill();
        grestore();
    }
    
    if( strokeType != NONE )
    {
        gsave();
        setStrokeColor();
        stroke();
        grestore();
    }
}

/////////////////////////////////////////////////////
void	EpsFile::header(const char *autor , int x0 , int y0, int x1, int y1 )
{
    if(!_file)
        return;
    
    //header.
    P(_file,"%%!PS-Adobe-3.0 EPSF-3.0\n");
    P(_file,"%%%%BoundingBox: %d %d %d %d\n",x0,y0,x1,y1);//0 0 2000 2000\n");
    P(_file,"%%%%generated by %s\n",autor);
    // add arrow code
    P(_file,"%s\n",arrowCode);
}

/////////////////////////////////////////////////////
void		EpsFile::newpath()
{
    if(!_file)
        return;
    
    P(_file,"newpath\n");
}

/////////////////////////////////////////////////////
void		EpsFile::closepath()
{
    if(!_file)
        return;
    
    P(_file,"closepath\n");
}

/////////////////////////////////////////////////////
void		EpsFile::setlinewidth( float w )
{
    if(!_file)
        return;
    
    P(_file,"%.5f setlinewidth\n",w);
}

/////////////////////////////////////////////////////
void		EpsFile::setrgb( float r, float g, float b )
{
    if(!_file)
        return;
    
    P(_file,"%.5f %.5f %.5f setrgbcolor\n",r,g,b);
}

/////////////////////////////////////////////////////
void		EpsFile::setrgb( const vec4 & clr )
{
    if(!_file)
        return;
    
    P(_file,"%.5f %.5f %.5f setrgbcolor\n",clr.x(),clr.y(),clr.z());
}

/////////////////////////////////////////////////////
void		EpsFile::setcmyk(float c, float m, float y, float k )
{
    if(!_file)
        return;
    
    P(_file,"%.5f %.5f %.5f %.5f setcmykcolor\n",c,m,y,k);
}

/////////////////////////////////////////////////////
void EpsFile::circle( const vec3 & pos, float radius )
{
    //gsave();
    //translate(pos.x(),pos.y());
    newpath();
    P(_file,"%.5f %.5f %.5f %.5f %.5f arc\n",makex( pos.x()*SCALE ),
      makey( pos.y()*SCALE ),
      radius*SCALE,
      0.0,
      360.0);
    closepath();
    fillStroke();
    //grestore();
}

void EpsFile::rect( float x, float y, float w, float h )
{
    newpath();
    moveto(x,y);
    lineto(x+w,y);
    lineto(x+w,y+h);
    lineto(x,y+h);
    closepath();
    fillStroke();
}

/////////////////////////////////////////////////////
void EpsFile::text( const vec3 & pos, float size, const std::string & str )
{
    gsave();
    newpath();
    P(_file,"/Times-Roman findfont\n");
    P(_file,"%.5f scalefont setfont\n",size*SCALE);
    moveto(pos);
    setFillColor();
    P(_file,"(%s) show\n",str.c_str());
    grestore();
}


/////////////////////////////////////////////////////
void		EpsFile::gray( float g )
{
    if(!_file)
        return;
    
    P(_file,"%.5f setgray\n",g);
}

/////////////////////////////////////////////////////
void		EpsFile::moveto( const vec3 & v )
{
    moveto(v.x(),v.y());
}

void		EpsFile::moveto( float x, float y )
{
    if(!_file)
        return;
    
    P(_file,"%.5f %.5f moveto\n", makex(x*SCALE) ,makey(y*SCALE) );
}

/////////////////////////////////////////////////////
void		EpsFile::lineto( float x, float y )
{
    if(!_file)
        return;
    
    P(_file,"%.5f %.5f lineto\n", makex(x*SCALE) , makey(y*SCALE) );
}
/////////////////////////////////////////////////////
void		EpsFile::lineto( const vec3 & v )
{
    lineto(v.x(),v.y());
}
/////////////////////////////////////////////////////
void		EpsFile::arrowto( float x, float y )
{
    P(_file,"%.5f %.5f arrowto\n", makex(x*SCALE), makey(y*SCALE) );
}
/////////////////////////////////////////////////////
void		EpsFile::arrowto( const vec3 & v )
{
    arrowto(v.x(),v.y());
}
/////////////////////////////////////////////////////
void		EpsFile::rlineto( float x, float y )
{
    if(!_file)
        return;
    
    P(_file,"%.5f %.5f rlineto\n", makex(x*SCALE), makey(y*SCALE) );
}

void		EpsFile::rlineto( const vec3 & v )
{
    rlineto(v.x(),v.y());
}


/////////////////////////////////////////////////////
void		EpsFile::stroke()
{
    if(!_file)
        return;
    
    P(_file,"stroke\n");
}

/////////////////////////////////////////////////////
void		EpsFile::fill()
{
    if(!_file)
        return;
    
    P(_file,"fill\n");
}

/////////////////////////////////////////////////////
void		EpsFile::showpage()
{
    if(!_file)
        return;
    
    P(_file,"showpage\n");
}

/////////////////////////////////////////////////////
void		EpsFile::gsave()
{
    if(!_file)
        return;
    
    P(_file,"gsave\n");
}

/////////////////////////////////////////////////////
void		EpsFile::grestore()
{
    if(!_file)
        return;
    
    P(_file,"grestore\n");
}

/////////////////////////////////////////////////////
void		EpsFile::setdash(float d0, float d1, float offset)
{
    if(!_file)
        return;
    
    P(_file,"[%.5f %.5f] %.5f setdash \n",d0,d1,offset);
}

/////////////////////////////////////////////////////
void		EpsFile::setdash(float d0, float d1, float d2, float d3, float offset)
{
    if(!_file)
        return;
    
    P(_file,"[%.5f %.5f %.5f %.5f] %.5f setdash \n",d0,d1,d2,d3,offset);
}


/////////////////////////////////////////////////////
void		EpsFile::translate( float x, float y )
{
    if(!_file)
        return;
    
    P(_file,"%g %g translate\n", makex(x*SCALE), makey(y*SCALE) );
}

/////////////////////////////////////////////////////
void		EpsFile::scale( float x, float y )
{
    if(!_file)
        return;
    
    P(_file,"%g %g scale\n",x,y);
}


/////////////////////////////////////////////////////
void		EpsFile::rotate( float ang )
{
    if(!_file)
        return;
    
    P(_file,"%g rotate\n",ang);
}


void EpsFile::setStrokeColor()
{
    switch(strokeType)
    {
        case GRAY:
            gray(strokeColor[0]);
            break;
        case RGB:
            setrgb(strokeColor[0],strokeColor[1],strokeColor[2]);
            break;
        case CMYK:
            setcmyk(strokeColor[0],strokeColor[1],strokeColor[2],strokeColor[3]);
            break;
    }
}

void EpsFile::setFillColor()
{
    switch(fillType)
    {
        case GRAY:
            gray(fillColor[0]);
            break;
        case RGB:
            setrgb(fillColor[0],fillColor[1],fillColor[2]);
            break;
        case CMYK:
            setcmyk(fillColor[0],fillColor[1],fillColor[2],fillColor[3]);
            break;
    }
}


void EpsFile::strokeGray( float v )
{
    strokeType = GRAY;
    strokeColor[0] = v;
}

void EpsFile::strokeRgb( float r, float g, float b )
{
    strokeType = RGB;
    strokeColor[0] = r;
    strokeColor[1] = g;
    strokeColor[2] = b;
}

void EpsFile::strokeRgb( const vec4 & clr )
{
    strokeRgb(clr.x(), clr.y(), clr.z());
}

void EpsFile::strokeCmyk( float c, float m, float y, float k )
{
    strokeType = CMYK;
    strokeColor[0] = c;
    strokeColor[1] = m;
    strokeColor[2] = y;
    strokeColor[3] = k;
}

void EpsFile::fillGray( float v )
{
    fillType = GRAY;
    fillColor[0] = v;
}

void EpsFile::fillRgb( float r, float g, float b )
{
    fillType = RGB;
    fillColor[0] = r;
    fillColor[1] = g;
    fillColor[2] = b;
}

void EpsFile::fillRgb( const vec4 & clr )
{
    fillRgb(clr.x(), clr.y(), clr.z());
}

void EpsFile::fillCmyk( float c, float m, float y, float k )
{
    fillType = CMYK;
    fillColor[0] = c;
    fillColor[1] = m;
    fillColor[2] = y;
    fillColor[3] = k;
}

void EpsFile::fillNone()
{
    fillType = NONE;
}

void EpsFile::strokeNone()
{
    strokeType = NONE;
}

/////////////////////////////////////////////////////
// TODO: translate rotate reset etc. 
//       function and var definition.

/////////////////////////////////////////////////////
// print to file.
void  EpsFile::print(const char *pFormat,... )
{
    if(!_file)
        return;
    
    char str[1024];
    va_list	parameter;
    va_start(parameter,pFormat);
    vsprintf(str,pFormat,parameter);
    va_end(parameter);
    
    P(_file,"%s",str);
}


void EpsFile::strokeShape( const polyline & s, const vec4 & clr )
{
    gsave();
    fillNone();
    strokeRgb(clr);
    shape(s);
    grestore();
}

void EpsFile::fillShape( const polyline & s, const vec4 & clr )
{
    gsave();
    strokeNone();
    fillRgb(clr);
    shape(s);
    grestore();
}

void EpsFile::strokeCircle( const vec3 & center, float radius, const vec4 & clr )
{
    gsave();
    fillNone();
    strokeRgb(clr);
    circle(center, radius);
    grestore();
}

void EpsFile::fillCircle( const vec3 & center, float radius, const vec4 & clr )
{
    gsave();
    strokeNone();
    fillRgb(clr);
    circle(center, radius);
    grestore();
}

void EpsFile::strokeRect( float x, float y, float w, float h, const vec4 & clr )
{
    gsave();
    fillNone();
    strokeRgb(clr);
    rect(x,y,w,h);
    grestore();
}

void EpsFile::fillRect( float x, float y, float w, float h, const vec4 & clr )
{
    gsave();
    strokeNone();
    fillRgb(clr);
    rect(x,y,w,h);
    grestore();
}

void EpsFile::drawLine( const vec3 & a, const vec3 & b, const vec4 & clr )
{
    gsave();
    newpath();
    fillNone();
    strokeRgb(clr);
    setStrokeColor();
    moveto(a);
    lineto(b);
    stroke();
    grestore();
}

void EpsFile::drawArrow( const vec3 & a, const vec3 & b, float size, const vec4 & clr )
{
    gsave();
    fillRgb(clr);
    strokeRgb(clr);
    setStrokeColor();
    setFillColor();
    moveto(a);
    arrowto(b);
    grestore();
}

float EpsFile::makex( float x ) const 
{
    return x;
}

float EpsFile::makey( float y ) const 
{
    return y; 
}


