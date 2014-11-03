//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

// who the hell cares if my identifiers are longer than 255 characters:
#pragma warning(disable : 4786)

#include <math.h>
#include "../vecmath/vec.h"
#include "../vecmath/mat.h"

#include <string>

#define MULTITHREADED

using std::string;

class RayTracer;

class TraceUI {
public:
	TraceUI()
		: m_nDepth(0), m_nSize(512),
        m_distAttenA(0.1), m_distAttenB(0.3), m_distAttenC(0.03),
		m_displayDebuggingInfo( false ),
		m_bsp_enabled_value( true ), 
		raytracer( 0 )
	{ }

	virtual int		run() = 0;

	// Send an alert to the user in some manner
	virtual void		alert(const string& msg) = 0;

	// setters
	virtual void		setRayTracer( RayTracer* r )
		{ raytracer = r; }

	// accessors:
	int		getSize() const { return m_nSize; }
	int		getDepth() const { return m_nDepth; }
    double  getDistanceAttenuationA() const { return m_distAttenA; }
    double  getDistanceAttenuationB() const { return m_distAttenB; }
    double  getDistanceAttenuationC() const { return m_distAttenC; }

	void setMultithreading(bool multithread) { this->multithread = multithread; }
	bool isMultithreading() const { return multithread; }

private:
	bool multithread;

protected:
	RayTracer*	raytracer;

	int			m_nSize;				// Size of the traced image
	int			m_nDepth;				// Max depth of recursion
    double      m_distAttenA;
    double      m_distAttenB;
    double      m_distAttenC;
    
	int num_threads;

	int width;
	int height;
	int nextX;
	int nextY;

	// Determines whether or not to show debugging information
	// for individual rays.  Disabled by default for efficiency
	// reasons.
	bool		m_displayDebuggingInfo;


public:
	bool		getBSPEnabled() const { return m_bsp_enabled_value; }

protected:
	bool		m_bsp_enabled_value;

};

#endif
