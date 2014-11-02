#include <cmath>

#include "Cone.h"

using namespace std;

bool Cone::intersectLocal( const ray& r, isect& i ) const
{
	bool ret = false;
	const int x = 0, y = 1, z = 2;	// For the dumb array indexes for the vectors

	Vec3d normal;
	
	Vec3d R0 = r.getPosition();
	Vec3d Rd = r.getDirection();
	double pz = R0[2];
	double dz = Rd[2];
	
	double a = Rd[x]*Rd[x] + Rd[y]*Rd[y] - beta_squared * Rd[z]*Rd[z];

	if( a == 0.0) return false;		// We're in the x-y plane, no intersection

	double b = 2 * (R0[x]*Rd[x] + R0[y]*Rd[y] - beta_squared * ((R0[z] + gamma) * Rd[z]));
	double c = -beta_squared*(gamma + R0[z])*(gamma + R0[z]) + R0[x] * R0[x] + R0[y] * R0[y];

	double discriminant = b * b - 4 * a * c;
	
	double BIG_NUMBER = 1e100;
	double farRoot, nearRoot, theRoot = -1;
	bool farGood, nearGood;
	
	if(discriminant < 0) return false;		// No intersection

	discriminant = sqrt(discriminant);

	// We have two roots, so calculate them
	nearRoot = (-b + discriminant) / ( 2 * a );
	farRoot = (-b - discriminant) / ( 2 * a );
	
	nearGood = isGoodRoot(r.at(nearRoot));
	if(nearGood && (nearRoot > RAY_EPSILON))
	{
		theRoot = nearRoot;
		normal = Vec3d(2*(r.at(theRoot))[x], 2*(r.at(theRoot))[y], -2.0 * beta_squared * (r.at(theRoot)[z] + gamma));
	}

	farGood = isGoodRoot(r.at(farRoot));
	if(farGood && (farRoot < theRoot || theRoot < 0) && (farRoot > RAY_EPSILON) ) 
	{
		theRoot = farRoot;
		normal = Vec3d(2*(r.at(theRoot))[x], (2*r.at(theRoot))[y], -2.0 * beta_squared * (r.at(theRoot)[z] + gamma));
	}

	// In case we are _inside_ the _uncapped_ cone, we need to flip the normal.
	// Essentially, the cone in this case is a double-sided surface
	// and has _2_ normals
	if( !capped && (normal * r.getDirection()) > 0 )
		normal = -normal;

	// These are to help with finding caps
	double t1 = (-pz)/dz;
	double t2 = (height-pz)/dz;
	
	Vec3d p( r.at( t1 ) );
	
	if(capped) {
		if( p[0]*p[0] + p[1]*p[1] <=  b_radius*b_radius)
		{
			if((t1 < theRoot || theRoot < 0) && t1 > RAY_EPSILON)
			{
				theRoot = t1;
				normal = Vec3d( 0.0, 0.0, -1.0 );
			}
		}
		Vec3d q( r.at( t2 ) );
		if( q[0]*q[0] + q[1]*q[1] <=  t_radius*t_radius)
		{
			if((t2 < theRoot || theRoot < 0) && t2 > RAY_EPSILON)
			{
				theRoot = t2;
				normal = Vec3d( 0.0, 0.0, 1.0 );
			}
		}
	}
	
	if(theRoot <= RAY_EPSILON) return false;
	
	i.setT(theRoot);
	normal.normalize();
	i.setN(normal);
	i.obj = this;
	return true;
	
	return ret;
}

bool Cone::isGoodRoot(Vec3d root) const
{

	if(root[2] < 0 || root[2] > height)
		return false;
	return true;
}

