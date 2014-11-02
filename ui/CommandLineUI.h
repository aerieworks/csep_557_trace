//
// CommandLineUI.h
//
// The header file for the command line UI
//

#ifndef __CommandLineUI_h__
#define __CommandLineUI_h__

#include "TraceUI.h"

#ifdef MULTITHREADED
#include "../threads/ThreadPool.h"
#endif

class CommandLineUI 
	: public TraceUI
{
public:
	CommandLineUI( int argc, char** argv );
	int		run();

	void		alert( const string& msg );

private:
	void		usage();

	char*	rayName;
	char*	imgName;
	char*	progName;

#ifdef MULTITHREADED
	static void threadStart(ThreadPool* tp, void* arg);
#endif
};

#endif
