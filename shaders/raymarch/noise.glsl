
vec3 mod289(vec3 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

float rand(float n){return fract(sin(n) * 43758.5453123);}

float noise(float p){
    float fl = floor(p);
  float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
  vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

// Classic Perlin noise
float cnoise(vec3 P)
{
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod289(Pi0);
  Pi1 = mod289(Pi1);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 * (1.0 / 7.0);
  vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 * (1.0 / 7.0);
  vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}

// Classic Perlin noise, periodic variant
float pnoise(vec3 P, vec3 rep)
{
  vec3 Pi0 = mod(floor(P), rep); // Integer part, modulo period
  vec3 Pi1 = mod(Pi0 + vec3(1.0), rep); // Integer part + 1, mod period
  Pi0 = mod289(Pi0);
  Pi1 = mod289(Pi1);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 * (1.0 / 7.0);
  vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 * (1.0 / 7.0);
  vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}

// noise stuff
float hash( float n ){

    return fract(sin(n)*43758.5453);
}

float hashNoise3D( in vec3 x ){

    vec3 p = floor(x);
    vec3 f = fract(x);//smoothstep(0., 1., fract(x));

    f = f*f*(3.0-2.0*f);//Smoothstep
    //f = f*f*f*(10.0+f*(6.0*f-15.0));//Smootherstep
   
    float n = p.x*7.0 + p.y*57.0 + 111.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  7.0),f.x),
                   mix( hash(n+ 57.0), hash(n+ 64.0),f.x),f.y),
               mix(mix( hash(n+111.0), hash(n+118.0),f.x),
                   mix( hash(n+168.0), hash(n+175.0),f.x),f.y),f.z);

    
    /*
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+ 157.0), hash(n+ 158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
    */ 
    
}

// Standard hash algorithm that you'll see all over the place.
vec3 hash33(vec3 p) { 

    // Faster, but doesn't disperse things quite as nicely as the block below it. However, when framerate
    // is an issue, and it often is, this is the one to use. Basically, it's a tweaked amalgamation I put
    // together, based on a couple of other random algorithms I've seen around... so use it with caution,
    // because I make a tonne of mistakes. :)
    //float n = sin(dot(p, vec3(7, 157, 113)));    
    //return fract(vec3(2097152, 262144, 32768)*n)*2.-1.; // return fract(vec3(64, 8, 1)*32768.0*n)*2.-1.; 

    // I'll assume this comes from IQ, of "ShaderToy.com."
    p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
		      dot(p,vec3(269.5,183.3,246.1)),
		      dot(p,vec3(113.5,271.9,124.6)));

    return fract(sin(p)*43758.5453)*2.0-1.0;

}

// I commented a while back that I knew of a way to drastically improve the speed of the standard, 3D-Voronoi shader 
// function that everyone uses. Eventually, someone wrote to me and said they had a difficult time believing my claim.
// "Faster, maybe, but not 'drastically' faster..." - I guess it was a little difficult to believe, since the ones getting 
// around are pretty quick... but not as quick as this one. :) And just to provoke the guy a little more: There's an even
// faster way. ;-)
//
// OK, the function doesn't produce aesthetically perfect Voronoi patterns, so Voronoi-esqe would probably be a better way 
// to describe it. Nevertheless, you can use the function for all sorts of things, and get away with it... or to put it another 
// way, at least you can use it for something interesting without bringing your frame rate to a screeching halt. You can't do 
// that on an average system with the 27-tap version. In fact, even the square-looking, 8-tap Voronoi function doesn't really 
// perform well enough. Anyway, that's enough rambing. Here's the routine, and a rough description:
//
// Instead of partitioning space into cubes, partition it into its simplex form, namely tetrahedrons. The closest vertice 
// will be one of the tetrahedral corners. The really cool thing is that the second closest point will also be one of the 
// corners. There's a bit more involved, but that's the basic idea. If you're familiar with a standard 3D Voronoi function, 
// and the standard 3D Simplex function, then the following shouldn't be too much of a stretch.
//
// Author: Gary "Shane" Warne
// Credits: Ken Perlin, the inventor of simplex noise, of course. Stefan Gustavson's paper - "Simplex Noise Demystified,"
// IQ, other "ShaderToy.com" people, etc.
float tetraVoronoi( in vec3 p )
{
    // Skewing the grid, then determining the first vertice.
    vec3 i  = floor(p + dot(p, vec3(0.333333)) );
    vec3 p0 = p - i + dot(i, vec3(0.166666)) ;

    // The following is a clever way to determine which tetrahedron we're in, which in turn can be used to calculate the 
    // two non-fixed vertices. Partitioning a volume into tetrahedrons, then calculating the corners is a fiddly, but doable 
    // math problem. You'll see it done time and again in 3D simplex noise algorithms, so I won't waste your time explaining 
    // it here. Just know that it works. :)
    
    // This takes away the necessary branching, when determining which side of two planes the point is on. This is my take
    // on it, but I got the idea from some anoymous internet person. I've seen something similar done in 3D voxel traversal,
    // so perhaps that's where it originated. 
    vec3 i1 = step(0., p0-p0.yzx); 
    vec3 i2 = max(i1, 1.0-i1.zxy);
    i1 = min(i1, 1.0-i1.zxy);    
    
    // The other three vertices. 
    vec3 p1 = p0 - i1 + 0.166666;
    vec3 p2 = p0 - i2 + 0.333333; 
    vec3 p3 = p0 - 0.5; 
    
    // Now we have all four vertices, which means we can do all sorts of cool things, but for now, I'm sticking to Voronoi patterns.   

    // This is probably the only place the function differs conceptually from the minimum-distance, cubic Voronoi function.
    // Here, we're using the standard 3D simplex falloff.
    vec4 v = max(0.5 - vec4(dot(p0,p0), dot(p1,p1), dot(p2,p2), dot(p3,p3)), 0.0)*2.;
    
    // Dotting the corners with a random vector generated for each corner, in order to determine the weighted 
    // contribution distribution... Kind of. Just for the record, you can do a non-gradient, value version that
    // works almost as well... Maybe, for another time.
    //
    // By the way, normalizing the hash gradients didn't seem to make an aesthetic difference, so I didn't waste the cyles.
    vec4 d = vec4(dot(p0, hash33(i)), dot(p1, hash33(i + i1)), dot(p2, hash33(i + i2)), dot(p3, hash33(i + 1.)));


    //return dot(d, v*v*v)*1.732 + 0.5; // Regular simplex noise... Not really, but close enough. There's your bonus. :)

    // For all intents and purposes, this vector holds the weighted values from our point in space to each of the vertices.
    // Now, all we have to do is find the two maximum values, or just the one, if that's all you're interested in. 
    d = (d+1.)*v*0.707; // It might need a bit of tweaking, but it'll do, for now.
    
    // Believe it or not, this mess simply determines the highest value in the 4D vector "d" (easy, and neat), and the 
    // second highest value (not so easy, and ugly). In fact, I'm sure there's a better way to do this. Some may be wondering
    // why I'm not trying to find the minimums. I could change the vector "v" above to arrange it that way, but by leaving it 
    // alone, I can get a simplex noise value as well... Something like that, anyway.
    float maxxy = max(d.x, d.y);
    float maxzw = max(d.z, d.w);
    float maxmins = max(min(d.x, d.y), min(d.z, d.w));   
    float minmaxs = min(maxxy, maxzw); 
   
    return  max(maxxy, maxzw) - max(maxmins, minmaxs); // Maximum value minus second highest value to give that cool beveled Voronoi look.
    //return  max(maxxy, maxzw); // Maximum, or regular value. 

}

float bumpFunction(in vec3 p){

     
     //return hashNoise3D(p*32.0)*0.57+hashNoise3D(p*64.0)*0.28+hashNoise3D(p*128.0)*0.15;
     //return hashNoise3D(p*64.)*0.667+hashNoise3D(p*128.)*0.333;
     
	  float vor = tetraVoronoi(p);
      float checkSize = 1.0; //1.0 is whole sphere
return vor;
     if ( mod(floor(checkSize * p.x) + floor(checkSize * p.y) + floor(checkSize *p.z), 1.0) < 1.0 ) return 1.-vor;
     else return vor;

   
}

/*
	terrain noise
*/

#define MOD2 vec2(3.07965, 7.4235)

vec2 add = vec2(1.0, 0.0);
float Hash12(vec2 p)
{
	p  = fract(p / MOD2);
    p += dot(p.xy, p.yx+19.19);
    return fract(p.x * p.y);
}

float Noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    
    float res = mix(mix( Hash12(p),          Hash12(p + add.xy),f.x),
                    mix( Hash12(p + add.yx), Hash12(p + add.xx),f.x),f.y);
    return res;
}

float tri( in vec2 p )
{

    vec2 q = 2.0*abs(fract(p)-0.5);
    q = q*q*(3.0-2.0*q);
    return -1.0 + q.x + q.y;
 
}