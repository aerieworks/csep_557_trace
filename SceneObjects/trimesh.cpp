#include <cmath>
#include <float.h>
#include "trimesh.h"
#include "TraceUI.h"

using namespace std;
extern TraceUI* traceUI;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
    vertices.push_back( v );
    boundingBox.min = minimum(boundingBox.min, v);
    boundingBox.max = maximum(boundingBox.max, v);
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt )
        return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    faces.push_back( newFace );
    scene->add(newFace);
    return true;
}

char *
Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

// Calculates and returns the normal of the triangle too.
bool TrimeshFace::intersectLocal( const ray& r, isect& i ) const
{
    // YOUR CODE HERE:
    // Add triangle intersection code here.
    // it currently ignores all triangles and just return false.
    //
    // Note that you are only intersecting a single triangle, and the vertices
    // of the triangle are supplied to you by the trimesh class.
    //
    // You should retrieve the vertices using code like this:
    //
    // const Vec3d& a = parent->vertices[ids[0]];
    // const Vec3d& b = parent->vertices[ids[1]];
    // const Vec3d& c = parent->vertices[ids[2]];
	// You are also required to implement Phong normal interpolation here to compute 
	// a more "rendering accurate" normal for the intersection point on triangle's face

    // Get triangle vertices.
    const Vec3d& a = parent->vertices[ids[0]];
    const Vec3d& b = parent->vertices[ids[1]];
    const Vec3d& c = parent->vertices[ids[2]];
    
    // Time when ray intersects plane is found by:
    // t = (d - n * ray.position) / (n * ray.direction)
    const double normalDotDirection = normal * r.getDirection();
    if (normalDotDirection != 0)
    {
        // A non-zero means the ray intersects the plane, need to see if it does so in the triangle.
        const double t = (facePlaneD - (normal * r.getPosition())) / normalDotDirection;
        if (t > RAY_EPSILON)
        {
            const Vec3d q = r.at(t);
            const double abqDotN = ((b - a) ^ (q - a)) * normal;
            if (abqDotN >= 0)
            {
                const double qbcDotN = ((c - b) ^ (q - b)) * normal;
                if (qbcDotN >= 0)
                {
                    const double aqcDotN = ((a - c) ^ (q - c)) * normal;
                    if (aqcDotN >= 0)
                    {
                        if (parent->normals.size() > 0 && traceUI->getPhongNormalInterpolationEnabled())
                        {
                            const double alpha = qbcDotN / barycentricDenominator;
                            const double beta = aqcDotN / barycentricDenominator;
                            const double gamma = abqDotN  / barycentricDenominator;
                    
                            i.setN(Vec3d(parent->normals[ids[0]] * alpha +
                                         parent->normals[ids[1]] * beta +
                                         parent->normals[ids[2]] * gamma));
                            i.N.normalize();
                        }
                        else
                        {
                            i.setN(normal);
                        }
                        
                        i.setMaterial(*material);
                        i.setObject(this);
                        i.setT(t);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}


void
Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    int faceIndex = 0;
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
        Vec3d a = vertices[(**fi)[0]];
        Vec3d b = vertices[(**fi)[1]];
        Vec3d c = vertices[(**fi)[2]];
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += (**fi).getNormal();
            ++numFaces[(**fi)[i]];
        }
        
        faceIndex++;
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
}

