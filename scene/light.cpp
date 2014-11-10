#include <cmath>

#include "TraceUI.h"
#include "light.h"

using namespace std;

extern TraceUI* traceUI;
extern bool debugMode;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
    const Vec3d direction = getDirection(P);
    ray r(P + direction * RAY_EPSILON, direction, ray::SHADOW);
    isect i;
    Vec3d attenuation = Vec3d(1, 1, 1);
    while (scene->intersect(r, i))
    {
        // No need to check distance to object, since directional lights are infinitely far away.
        // Object is guaranteed to be closer.
        if (traceUI->getTransparentColorFilteringEnabled())
        {
            attenuation = prod(attenuation, i.getMaterial().kt(i));
            r = ray(r.at(i.t) + direction * RAY_EPSILON, direction, ray::SHADOW);
        }
        else
        {
            attenuation *= 0;
            break;
        }
    }
    
    return attenuation;
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
    // Model distance attenuation as:
    //      1 / (A + Bd + Cd^2)
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
    ray r(P + direction * RAY_EPSILON, direction, ray::SHADOW);
    isect i;
    Vec3d attenuation(1, 1, 1);
    while (scene->intersect(r, i))
    {
        const Vec3d isectPoint = r.at(i.t);
        if ((isectPoint - P).length() < (position - P).length())
        {
            if (traceUI->getTransparentColorFilteringEnabled())
            {
                attenuation = prod(attenuation, i.getMaterial().kt(i));
                r = ray(isectPoint + direction * RAY_EPSILON, direction, ray::SHADOW);
            }
            else
            {
                attenuation *= 0;
                break;
            }
        }
        else
        {
            break;
        }
    }
    
    return attenuation;
}
