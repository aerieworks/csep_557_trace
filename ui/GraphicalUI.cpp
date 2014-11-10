//
// GraphicalUI.cpp
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifndef COMMAND_LINE_ONLY

#include <FL/fl_ask.H>
#include "debuggingView.h"

#include "GraphicalUI.h"
#include "../RayTracer.h"
#include "dialog.h"

#pragma warning (disable: 4996)


bool GraphicalUI::stopTrace = false;
bool GraphicalUI::doneTrace = true;

//------------------------------------- Help Functions --------------------------------------------
GraphicalUI* GraphicalUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ( (GraphicalUI*)(o->parent()->user_data()) );
}

//------------------------------------------------------------------------
// This displays a modal file chooser with a native look-and-feel.
// The available dialog types are:
//   BROWSE_DIRECTORY       - Open a single directory
//   BROWSE_FILE            - Open a single file
//   BROWSE_MULTI_DIRECTORY - Open directories, allowing the user to
//                            select more than one at a time
//   BROWSE_MULTI_FILE      - Open files, allowing the user to select
//                            more than one at a time
//   BROWSE_SAVE_DIRECTORY  - Save a directory
//   BROWSE_SAVE_FILE       - Save a file
//
// The filter limits the displayed files. See cb_load_image for an example.
// title is optional, use NULL for the OS default title.
// The return value is the filepath.
//------------------------------------------------------------------------
const char* GraphicalUI::fileDialog(Fl_Native_File_Chooser::Type dialogType, const char* filter, const char* title) {
	if(!m_nativeChooser)
		m_nativeChooser = new Fl_Native_File_Chooser(dialogType);
	else
		m_nativeChooser->type(dialogType);
	m_nativeChooser->filter(filter);
	m_nativeChooser->title(title);
	int ret = m_nativeChooser->show();
	if(ret == -1 ||	ret == 1) {
		//error or cancel respectively
		return NULL;
	}
	return m_nativeChooser->filename();
}

//--------------------------------- Callback Functions --------------------------------------------
void GraphicalUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	GraphicalUI* pUI=whoami(o);
	
	static char* lastFile = 0;
	const char* newfile = pUI->fileDialog(Fl_Native_File_Chooser::BROWSE_FILE,
											"Scene Description File (*.ray)\t*.ray",
											"Open Scene");
	if(!newfile)
		return;

	char buf[300];

	if (pUI->raytracer->loadScene(newfile)) {
		sprintf(buf, "Ray <%s>", newfile);
		stopTracing();	// terminate the previous rendering
	} else{
		sprintf(buf, "Ray <Not Loaded>");
	}

    char buftitle[300];
    int ic;
    for (ic = 299;ic >= 0; ic--) {
        buftitle[ic] = buf[ic];
        if (buf[ic] == '/') break;
    }

    ic++;

    for (int uc = ic;ic <  300; ic++) {
        buftitle[ic-uc] = buf[ic];
    }
    sprintf(buf, "Ray <%s", buftitle);
    
	pUI->m_mainWindow->label(buf);
	pUI->m_debuggingWindow->m_debuggingView->setDirty();

	if( lastFile != 0 && strcmp(newfile, lastFile) != 0 )
		pUI->m_debuggingWindow->m_debuggingView->resetCamera();

	pUI->m_debuggingWindow->redraw();
}

void GraphicalUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	GraphicalUI* pUI=whoami(o);

	const char* szFileName = pUI->fileDialog(Fl_Native_File_Chooser::BROWSE_SAVE_FILE,
											"PNG Image File (*.png)\t*.png\nJPEG Image File (*.jpg)\t*.jpg",
											"Save Image");
	if(!szFileName)
		return;

	int filetype = pUI->m_nativeChooser->filter_value();	//0: PNG, 1: JPG
	int quality = 95;
	if (filetype == 1) {
		Dialog2 x(0,0,0,0,"ok");
		quality = x.getValue();
	}
	pUI->m_traceGlWindow->saveImage(szFileName, filetype == 0 ? ".png" : ".jpg", quality);
}

void GraphicalUI::cb_exit(Fl_Menu_* o, void* v)
{
	GraphicalUI* pUI=whoami(o);

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
}

void GraphicalUI::cb_exit2(Fl_Widget* o, void* v) 
{
	GraphicalUI* pUI=(GraphicalUI *)(o->user_data());
	
	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
}

void GraphicalUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project, FLTK version for CSEP557 Spring 2009.");
}

void GraphicalUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	GraphicalUI* pUI=(GraphicalUI*)(o->user_data());

	// terminate the rendering so we don't get crashes
	stopTracing();

	pUI->m_nSize=int( ((Fl_Slider *)o)->value() ) ;
	int	height = (int)(pUI->m_nSize / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow( pUI->m_nSize, height );
	// Need to call traceSetup before trying to render
	pUI->raytracer->setReady(false);
}

void GraphicalUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_reflectionEnabledCheckButton(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_reflectionEnabled = (((Fl_Check_Button *)o)->value() == 1);
}

void GraphicalUI::cb_refractionEnabledCheckButton(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_refractionEnabled = (((Fl_Check_Button *)o)->value() == 1);
}

void GraphicalUI::cb_transparentColorFilteringEnabledCheckButton(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_transparentColorFilteringEnabled = (((Fl_Check_Button *)o)->value() == 1);
}

void GraphicalUI::cb_phongNormalInterpolationEnabledCheckButton(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_phongNormalInterpolationEnabled = (((Fl_Check_Button *)o)->value() == 1);
}

#ifdef MULTITHREADED
void GraphicalUI::cb_threadSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->num_threads=int( ((Fl_Slider *)o)->value() ) ;
}
#endif

void GraphicalUI::cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v)
{
	GraphicalUI* pUI=(GraphicalUI*)(o->user_data());
	pUI->m_displayDebuggingInfo = (((Fl_Check_Button*)o)->value() == 1);
	if( pUI->m_displayDebuggingInfo )
		pUI->m_debuggingWindow->show();
	else
		pUI->m_debuggingWindow->hide();
}


void GraphicalUI::cb_bspEnabledCheckButton(Fl_Widget* o, void* v)
{
	GraphicalUI* pUI=(GraphicalUI*)(o->user_data());
	pUI->m_bsp_enabled_value = (((Fl_Check_Button*)o)->value() == 1);
}

void GraphicalUI::cb_render(Fl_Widget* o, void* v)
{
	char buffer[256];

	GraphicalUI* pUI=((GraphicalUI*)(o->user_data()));
    clock_t start = clock();
	
	if (pUI->raytracer->sceneLoaded()) {
		pUI->width=pUI->getSize();
		pUI->height = (int)(pUI->width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow( pUI->width, pUI->height );

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(pUI->width, pUI->height);
		
		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		doneTrace = false;
		stopTrace = false;
#ifdef MULTITHREADED
		pUI->nextX = 0;
		pUI->nextY = 0;
		
		pUI->setMultithreading(true);
		ThreadPool* tp = new ThreadPool();
		for (int i = 0; i < pUI->num_threads; i++)
			tp->startThread(threadStart, pUI);
		
		while (!tp->waitForThreads(500)) {
			pUI->updateRender();
			sprintf(buffer, "(%d%%) %s", (int)((double)pUI->nextY / (double)pUI->height * 100.0), old_label);
			pUI->m_traceGlWindow->label(buffer);
		}

		pUI->setMultithreading(false);
		delete tp;
#else
		// start to render here	
		clock_t prev, now;
		prev=clock();

		pUI->setMultithreading(false);
		for (int y=0; y<pUI->height; y++) {
			for (int x=0; x<pUI->width; x++) {
				if (stopTrace) break;
				
				// current time
				now = clock();

				// check event every 1/2 second
				if (((double)(now-prev)/CLOCKS_PER_SEC)>0.5) {
					prev=now;
					pUI->updateRender();
				}

				pUI->raytracer->tracePixel( x, y );
				pUI->m_debuggingWindow->m_debuggingView->setDirty();
			}
			if (stopTrace) break;

			// flush when finish a row
			pUI->updateRender();
			// update the window label
			sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)pUI->height * 100.0), old_label);
			pUI->m_traceGlWindow->label(buffer);
			
		}
#endif
		doneTrace=true;
		stopTrace=false;

		pUI->m_traceGlWindow->refresh();

		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);
        
        std::cerr << "Render time: " << (double)(clock() - start) / (double)CLOCKS_PER_SEC << " seconds" << std::endl;
	}
}

void GraphicalUI::cb_stop(Fl_Widget* o, void* v)
{
	stopTrace = true;
}

int GraphicalUI::run()
{
	Fl::visual(FL_DOUBLE|FL_INDEX);

	m_mainWindow->show();

	return Fl::run();
}

void GraphicalUI::alert( const string& msg )
{
	fl_alert( "%s", msg.c_str() );
}

void GraphicalUI::setRayTracer(RayTracer *tracer)
{
	TraceUI::setRayTracer( tracer );
	m_traceGlWindow->setRayTracer(tracer);
	m_debuggingWindow->m_debuggingView->setRayTracer(tracer);
}

// menu definition
Fl_Menu_Item GraphicalUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)GraphicalUI::cb_load_scene },
		{ "&Save Image...",	FL_ALT + 's', (Fl_Callback *)GraphicalUI::cb_save_image },
		{ "&Exit",			FL_ALT + 'e', (Fl_Callback *)GraphicalUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback *)GraphicalUI::cb_about },
		{ 0 },

	{ 0 }
};


void GraphicalUI::stopTracing()
{
	if( stopTrace ) return;			// Only one person can be waiting at a time

	stopTrace = true;

	// Wait for the trace to finish (simple synchronization)
	while( !doneTrace )	Fl::wait();
}

GraphicalUI::GraphicalUI() : m_nativeChooser(NULL) {
	// init.

	m_mainWindow = new Fl_Window(100, 40, 350, 350, "Ray <Not Loaded>");
		m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
		// install menu bar
		m_menubar = new Fl_Menu_Bar(0, 0, 320, 25);
		m_menubar->menu(menuitems);

		// install depth slider
		m_depthSlider = new Fl_Value_Slider(10, 30, 180, 20, "Depth");
		m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_depthSlider->type(FL_HOR_NICE_SLIDER);
        m_depthSlider->labelfont(FL_COURIER);
        m_depthSlider->labelsize(12);
		m_depthSlider->minimum(0);
		m_depthSlider->maximum(10);
		m_depthSlider->step(1);
		m_depthSlider->value(m_nDepth);
		m_depthSlider->align(FL_ALIGN_RIGHT);
		m_depthSlider->callback(cb_depthSlides);

		// install size slider
		m_sizeSlider = new Fl_Value_Slider(10, 55, 180, 20, "Size");
		m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_sizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sizeSlider->labelfont(FL_COURIER);
        m_sizeSlider->labelsize(12);
		m_sizeSlider->minimum(64);
		m_sizeSlider->maximum(512);
		m_sizeSlider->step(1);
		m_sizeSlider->value(m_nSize);
		m_sizeSlider->align(FL_ALIGN_RIGHT);
		m_sizeSlider->callback(cb_sizeSlides);

#ifdef MULTITHREADED
		m_threadSlider = new Fl_Value_Slider(10, 80, 180, 20, "Number of threads");
		m_threadSlider->user_data((void*)(this)); // record self to be used by static callback functions
		m_threadSlider->type(FL_HOR_NICE_SLIDER);
		m_threadSlider->labelfont(FL_COURIER);
		m_threadSlider->labelsize(12);
		m_threadSlider->minimum(1);
		m_threadSlider->maximum(40);
		m_threadSlider->step(1);
		m_threadSlider->value(4);
		m_threadSlider->align(FL_ALIGN_RIGHT);
		m_threadSlider->callback(cb_threadSlides);
		num_threads = 4;
#endif

        m_phongNormalInterpolationEnabledCheckButton = new Fl_Check_Button(0, 180, 335, 20, "Phong Normal Interpolation Enabled (When Avail.)");
        m_phongNormalInterpolationEnabledCheckButton->user_data((void*)(this));
        m_phongNormalInterpolationEnabledCheckButton->callback(cb_phongNormalInterpolationEnabledCheckButton);
        m_phongNormalInterpolationEnabledCheckButton->value(m_phongNormalInterpolationEnabled);
    
        m_transparentColorFilteringEnabledCheckButton = new Fl_Check_Button(0, 205, 250, 20, "Transparent Color Filtering Enabled");
        m_transparentColorFilteringEnabledCheckButton->user_data((void*)(this));
        m_transparentColorFilteringEnabledCheckButton->callback(cb_transparentColorFilteringEnabledCheckButton);
        m_transparentColorFilteringEnabledCheckButton->value(m_transparentColorFilteringEnabled);
    
        m_reflectionEnabledCheckButton = new Fl_Check_Button(0, 230, 180, 20, "Reflection Enabled");
        m_reflectionEnabledCheckButton->user_data((void*)(this));
        m_reflectionEnabledCheckButton->callback(cb_reflectionEnabledCheckButton);
        m_reflectionEnabledCheckButton->value(m_reflectionEnabled);

        m_refractionEnabledCheckButton = new Fl_Check_Button(0, 255, 180, 20, "Refraction Enabled");
        m_refractionEnabledCheckButton->user_data((void*)(this));
        m_refractionEnabledCheckButton->callback(cb_refractionEnabledCheckButton);
        m_refractionEnabledCheckButton->value(m_refractionEnabled);

        // set up debugging display checkbox
        m_debuggingDisplayCheckButton = new Fl_Check_Button(0, 305, 180, 20, "Debugging display");
		m_debuggingDisplayCheckButton->user_data((void*)(this));
		m_debuggingDisplayCheckButton->callback(cb_debuggingDisplayCheckButton);
		m_debuggingDisplayCheckButton->value(m_displayDebuggingInfo);

		// set up BSP acceleration checkbox
        m_bsp_enabledCheckButton= new Fl_Check_Button(0, 280, 180, 20, "BSP Acceleration Enabled");
		m_bsp_enabledCheckButton->user_data((void*)(this));
		m_bsp_enabledCheckButton->callback(cb_bspEnabledCheckButton);
		m_bsp_enabledCheckButton->value(m_bsp_enabled_value);

		// set up "render" button
		m_renderButton = new Fl_Button(240, 27, 70, 25, "&Render");
		m_renderButton->user_data((void*)(this));
		m_renderButton->callback(cb_render);

		// set up "stop" button
		m_stopButton = new Fl_Button(240, 55, 70, 25, "&Stop");
		m_stopButton->user_data((void*)(this));
		m_stopButton->callback(cb_stop);

		m_mainWindow->callback(cb_exit2);
		m_mainWindow->when(FL_HIDE);
    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);

	// debugging view
	m_debuggingWindow = new DebuggingWindow();
}


void GraphicalUI::updateRender() {
	if (Fl::ready()) {
		// refresh
		m_traceGlWindow->refresh();
		// check event
		Fl::check();

		if (Fl::damage()) {
			Fl::flush();
		}
	}
}

#ifdef MULTITHREADED
void GraphicalUI::threadStart(ThreadPool* tp, void* arg) {
	GraphicalUI* pUI = (GraphicalUI*)arg;
	tp->holdMutex();
	int x = pUI->nextX;
	int y = pUI->nextY;
	while (!stopTrace && y < pUI->height) {
		int maxX = x + 32;
		int maxY = y + maxX / pUI->width;
		maxX %= pUI->width;
		pUI->nextX = maxX;
		pUI->nextY = maxY;
		tp->releaseMutex();
		while (!stopTrace && y < pUI->height && (y < maxY || y == maxY && x < maxX)) {
			pUI->raytracer->tracePixel(x, y);
			x++;
			if (x >= pUI->width) {
				x = 0;
				y++;
			}
		}
		tp->holdMutex();
		pUI->m_debuggingWindow->m_debuggingView->setDirty();
		x = pUI->nextX;
		y = pUI->nextY;
	}
	tp->releaseMutex();
}
#endif
#endif

