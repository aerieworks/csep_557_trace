// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>

using namespace std;

#define AIR_REFRACTION_INDEX (1.0003)

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
Vec3d RayTracer::trace( double x, double y )
{
	// Clear out the ray cache in the scene for debugging purposes,
	if (!traceUI->isMultithreading())
		scene->intersectCache.clear();

    ray r( Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY );

    scene->getCamera().rayThrough( x,y,r );
	Vec3d ret = traceRay( r, Vec3d(1.0,1.0,1.0), 0, true );
    ret.clamp();
	return ret;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay( const ray& r, 
	const Vec3d& thresh, int depth, bool isInAir )
{
	isect i;

    Vec3d directShade, reflectShade, refractShade;
    
    if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		const Material& m = i.getMaterial();
        
        // Calculate direct illumination.
        directShade = m.shade(scene, r, i, isInAir);
        
        if (depth < traceUI->getDepth())
        {
            Vec3d viewingDir(-r.getDirection());
            viewingDir.normalize();
            
            if (traceUI->getReflectionEnabled())
            {
                // Calculate reflected illumination.
                const Vec3d reflectionDir = 2.0 * (viewingDir * i.N) * i.N - viewingDir;
                ray reflectionRay(r.at(i.t) + (reflectionDir * RAY_EPSILON), reflectionDir, ray::REFLECTION);
                reflectShade = prod(m.kr(i), traceRay(reflectionRay, thresh, depth + 1, isInAir));
            }
            
            if (traceUI->getRefractionEnabled())
            {
                // The normal is always given relative to the external face.  If we are in an object, we're actually
                // impacting the internal face, so negate the normal.
                Vec3d normal = isInAir ? i.N : -i.N;
                
                // Calculate refracted illumination.
                const double n_i = isInAir ? AIR_REFRACTION_INDEX : m.index(i);
                const double n_t = isInAir ? m.index(i) : AIR_REFRACTION_INDEX;
                const double n_ratio = n_i / n_t;
                const double cos_theta_i = normal * viewingDir;
                const double cos_theta_t_root_term = 1 - pow(n_ratio, 2.0) * (1 - cos_theta_i * cos_theta_i);

                // A negative root term indicates total internal reflection.
                if (cos_theta_t_root_term >= 0)
                {
                    const double cos_theta_t = sqrt(cos_theta_t_root_term);
                    const Vec3d refractionDir = (n_ratio * cos_theta_i - cos_theta_t) * normal - n_ratio * viewingDir;
                    ray refractionRay(r.at(i.t) + (refractionDir * RAY_EPSILON), refractionDir, ray::REFRACTION);
                    const Vec3d refractionShade = traceRay(refractionRay, thresh, depth + 1, !isInAir);
                    refractShade = prod(m.kt(i), refractionShade);
                }
            }
        }
    }
    
    if (debugMode && r.type() == ray::VISIBILITY) cerr << "Direct: " << directShade << ", Reflect: " << reflectShade << ", Refract: " << refractShade << endl;
    return directShade + reflectShade + refractShade;
}

RayTracer::RayTracer()
	: scene( 0 ), buffer( 0 ), buffer_width( 0 ), buffer_height( 0 ), m_bBufferReady( false )
{
}


RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( const char* fn )
{
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos )
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}


	if( ! sceneLoaded() )
		return false;


	// Initialize the scene's BSP tree
	scene->initBSPTree();

	
	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];

	}
	memset( buffer, 0, w*h*3 );
	m_bBufferReady = true;
}

void RayTracer::tracePixel( int i, int j )
{
	Vec3d col;

	if( ! sceneLoaded() )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	col = trace( x,y );
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}

