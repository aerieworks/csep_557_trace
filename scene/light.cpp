#include <cmath>

#include "TraceUI.h"
#include "light.h"

using namespace std;

extern TraceUI* traceUI;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
    return Vec3d(1,1,1);
}

Vec3d DirectionalLight::getColor( const Vec3d& P ) const
{
	// Color doesn't depend on P 
	return color;
}

Vec3d DirectionalLight::getDirection( const Vec3d& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const Vec3d& P ) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0
    const double distance = (position - P).length();
    return min(1.0, 1.0 / (traceUI->getDistanceAttenuationA()
                           + traceUI->getDistanceAttenuationB() * distance
                           + traceUI->getDistanceAttenuationC() * pow(distance, 2)));
}

Vec3d PointLight::getColor( const Vec3d& P ) const
{
	// Color doesn't depend on P 
	return color;
}

Vec3d PointLight::getDirection( const Vec3d& P ) const
{
	Vec3d ret = position - P;
	ret.normalize();
	return ret;
}


Vec3d PointLight::shadowAttenuation(const Vec3d& P) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
    const Vec3d direction = getDirection(P);
    const ray r(P, direction);
    isect i;
    if (scene->intersect(r, i))
    {
        const Vec3d isectPoint = r.at(i.t);
        if ((isectPoint - P).length() < (position - P).length())
        {
            return Vec3d(0, 0, 0);
        }
    }
    
    return Vec3d(1,1,1);
}
