// Code adapted from LibCinder
// local defs for simplify

typedef struct{
    vec3 P0;
    vec3 P1;
}Segment;

#define norm2(v)   dot(v,v)        // norm2 = squared length of vector
#define norm(v)    sqrt(norm2(v))  // norm = length of vector
#define d2(u,v)    norm2(u-v)      // distance squared = norm2 of difference
#define d(u,v)     norm(u-v)       // distance = norm of difference


void  dp(float tol, vec3* v, int j, int k, int* mk )
{
    if (k <= j+1) // there is nothing to simplify
        return;
    vec3 caz;
    
    // check for adequate approximation by segment S from v[j] to v[k]
    int     maxi	= j;          // index of vertex farthest from S
    float   maxd2	= 0;         // distance squared of farthest vertex
    float   tol2	= tol * tol;  // tolerance squared
    Segment S		= {v[j], v[k]};  // segment from v[j] to v[k]
    vec3 u;
    u				= S.P1 - S.P0;   // segment direction vector
    double  cu		= dot(u,u);     // segment length squared
    
    // test each vertex v[i] for max distance from S
    // compute using the Feb 2001 Algorithm's dist_ofPoint_to_Segment()
    // Note: this works in any dimension (2D, 3D, ...)
    vec3  w;
    vec3   Pb;                // base of perpendicular from v[i] to S
    float  b, cw, dv2;        // dv2 = distance v[i] to S squared
    
    for (int i=j+1; i<k; i++){
        // compute distance squared
        w = v[i] - S.P0;
        cw = dot(w,u);
        if ( cw <= 0 ) dv2 = d2(v[i], S.P0);
        else if ( cu <= cw ) dv2 = d2(v[i], S.P1);
        else {
            b = (float)(cw / cu);
            Pb = S.P0 + u*b;
            dv2 = d2(v[i], Pb);
        }
        // test with current max distance squared
        if (dv2 <= maxd2) continue;
        
        // v[i] is a new max vertex
        maxi = i;
        maxd2 = dv2;
    }
    if (maxd2 > tol2)        // error is worse than the tolerance
    {
        // split the polyline at the farthest vertex from S
        mk[maxi] = 1;      // mark v[maxi] for the simplified polyline
        // recursively simplify the two subpolylines at v[maxi]
        dp( tol, v, j, maxi, mk );  // polyline v[j] to v[maxi]
        dp( tol, v, maxi, k, mk );  // polyline v[maxi] to v[k]
    }
    // else the approximation is OK, so ignore intermediate vertices
    return;
}


//-------------------------------------------------------------------
// needs simplifyDP which is above
std::vector<vec3> dp_simplify(const std::vector<vec3>& P, float tol)
{
    int n = P.size();
    std::vector <vec3> V = P;
    
    int    i, k, m, pv;            // misc counters
    float  tol2 = tol * tol;       // tolerance squared
    
    vec3 * vt = new vec3[n];
    int * mk = new int[n];
    
    memset(mk, 0, sizeof(int) * n );
    
    // STAGE 1.  Vertex Reduction within tolerance of prior vertex cluster
    vt[0] = V[0];              // start at the beginning
    for (i=k=1, pv=0; i<n; i++)
    {
        if (d2(V[i], V[pv]) < tol2) continue;
        
        vt[k++] = V[i];
        pv = i;
    }
    
    // add flag?
    if (pv < n-1) vt[k++] = V[n-1];      // finish at the end
    
    // STAGE 2.  Douglas-Peucker polyline simplification
    mk[0] = mk[k-1] = 1;       // mark the first and last vertices
    dp( tol, vt, 0, k-1, mk );
    
    std::vector<vec3> res;
    
    for (i=m=0; i<k; i++)
    {
        const vec3 & p = vt[i];
        if(mk[i])
            res.push_back(p);
    }
    
    delete [] vt;
    delete [] mk;
    
    return res;
}

