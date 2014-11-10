//
// GraphicalUI.h
//
// The header file for the graphical UI
//

#ifndef __GraphicalUI_h__
#define __GraphicalUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

#include <FL/Fl_Choice.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "TraceUI.h"
#include "TraceGLWindow.h"
#include "debuggingWindow.h"

#ifdef MULTITHREADED
#include "../threads/ThreadPool.h"
#endif

class ModelerView;

class GraphicalUI : public TraceUI {
public:
	GraphicalUI();

	int run();

	void		alert( const string& msg );

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;
    Fl_Slider*          m_distAttenASlider;
    Fl_Slider*          m_distAttenBSlider;
    Fl_Slider*          m_distAttenCSlider;
    Fl_Check_Button*    m_reflectionEnabledCheckButton;
    Fl_Check_Button*    m_refractionEnabledCheckButton;
    Fl_Check_Button*    m_transparentColorFilteringEnabledCheckButton;
    Fl_Check_Button*    m_phongNormalInterpolationEnabledCheckButton;
    
#ifdef MULTITHREADED
	Fl_Slider*			m_threadSlider;
#endif


	Fl_Check_Button*	m_bsp_enabledCheckButton;
	Fl_Check_Button*	m_debuggingDisplayCheckButton;

	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;

	TraceGLWindow*		m_traceGlWindow;


	DebuggingWindow*	m_debuggingWindow;
	// member functions
	void		setRayTracer(RayTracer *tracer);

	static void stopTracing();
private:
	void updateRender();

// static class members
	static Fl_Menu_Item menuitems[];

	static GraphicalUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);

	static void cb_sizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);
    static void cb_distAttenASlides(Fl_Widget* o, void* v);
    static void cb_distAttenBSlides(Fl_Widget* o, void* v);
    static void cb_distAttenCSlides(Fl_Widget* o, void* v);
    static void cb_reflectionEnabledCheckButton(Fl_Widget* o, void* v);
    static void cb_refractionEnabledCheckButton(Fl_Widget* o, void* v);
    static void cb_transparentColorFilteringEnabledCheckButton(Fl_Widget* o, void* v);
    static void cb_phongNormalInterpolationEnabledCheckButton(Fl_Widget* o, void* v);
    
#ifdef MULTITHREADED
	static void cb_threadSlides(Fl_Widget* o, void* v);
	static void threadStart(ThreadPool* tp, void* arg);
#endif


	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);
	static void cb_bspEnabledCheckButton(Fl_Widget* o, void* v);
	static void cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v);

	static bool doneTrace;		// Flag that gets set when the trace is done
	static bool stopTrace;		// Flag that gets set when the trace should be stopped

	
	// File dialog stuff
	const char* fileDialog(Fl_Native_File_Chooser::Type dialogType, const char* filter, const char* title=NULL);
	Fl_Native_File_Chooser* m_nativeChooser;
};

#endif
