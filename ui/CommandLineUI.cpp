#include <iostream>
#include <time.h>
#include <stdarg.h>

#include <assert.h>

#include "CommandLineUI.h"
#include "../fileio/imageio.h"

#include "../RayTracer.h"
#include "../getopt.h"

using namespace std;

// ***********************************************************


// The command line UI simply parses out all the arguments off
// the command line and stores them locally.
CommandLineUI::CommandLineUI( int argc, char** argv )
	: TraceUI()
{
	int i;
	num_threads = 10;

	progName=argv[0];

	while( (i = getopt( argc, argv, "r:w:t:bBaAh" )) != EOF )
	{
		switch( i )
		{
			case 'r':
				m_nDepth = atoi( optarg );
				break;

			case 'w':
				m_nSize = atoi( optarg );
				break;
			case 'b':
				m_bsp_enabled_value = true;
				break;
			case 'B':
				m_bsp_enabled_value = false;
				break;
#ifdef MULTITHREADED
			case 't':
				num_threads = atoi( optarg );
				break;
#endif
			case 'h':
				usage();
				exit(1);
				break;

			default:
			// Oops; unknown argument
			std::cerr << "Invalid argument: '" << i << "'." << std::endl;
			usage();
			exit(1);
		}
	}

	if( optind >= argc-1 )
	{
		std::cerr << "no input and/or output name." << std::endl;
		exit(1);
	}

//#ifdef __APPLE__
//	rayName = argv[++argInd];
//	imgName = argv[++argInd];

//#else
	rayName = argv[optind];
	imgName = argv[optind+1];
//#endif
}

int CommandLineUI::run()
{
	assert( raytracer != 0 );
	raytracer->loadScene( rayName );

	if( raytracer->sceneLoaded() )
	{
		width = m_nSize;
		height = (int)(width / raytracer->aspectRatio() + 0.5);

		raytracer->traceSetup( width, height );

		clock_t start, end;
		start = clock();

#ifdef MULTITHREADED
		nextX = 0;
		nextY = 0;
		
		setMultithreading(true);
		ThreadPool* tp = new ThreadPool();
		for (int i = 0; i < num_threads; i++)
			tp->startThread(threadStart, this);
		tp->waitForThreads(ThreadPool::NO_TIMEOUT);
		setMultithreading(false);
		delete tp;
#else
		for( int j = 0; j < height; ++j )
			for( int i = 0; i < width; ++i )
				raytracer->tracePixel(i,j);
#endif

		end=clock();

		// save image
		unsigned char* buf;

		raytracer->getBuffer(buf, width, height);

		if (buf)
			save(imgName, buf, width, height, ".png", 95);

		double t=(double)(end-start)/CLOCKS_PER_SEC;
		std::cout << "total time = " << t << " seconds" << std::endl;
        return 0;
	}
	else
	{
		std::cerr << "Unable to load ray file '" << rayName << "'" << std::endl;
		return( 1 );
	}
}

void CommandLineUI::alert( const string& msg )
{
	std::cerr << msg << std::endl;
}

void CommandLineUI::usage()
{
	std::cerr << "usage: " << progName << " [options] [input.ray output.bmp]" << std::endl;
	std::cerr << "  -r <#>      set recursion level (default " << m_nDepth << ")" << std::endl; 
	std::cerr << "  -w <#>      set output image width (default " << m_nSize << ")" << std::endl;
	std::cerr << "  -b          enable BSP tree acceleration (default)" << std::endl;
	std::cerr << "  -B          disable BSP tree acceleration" << std::endl;
	std::cerr << "  -a          (TODO) enable antialiasing" << std::endl;
	std::cerr << "  -A          (TODO) disable antialiasing (default)" << std::endl;
#ifdef MULTITHREADED
	std::cerr << "  -t			number of threads (default 10)" << std::endl;
#endif
	std::cerr << "  -h          display this help message" << std::endl;
}

#ifdef MULTITHREADED
void CommandLineUI::threadStart(ThreadPool* tp, void* arg) {
	CommandLineUI* pUI = (CommandLineUI*)arg;
	tp->holdMutex();
	int x = pUI->nextX;
	int y = pUI->nextY;
	while (y < pUI->height) {
		int maxX = x + 8;
		int maxY = y + maxX / pUI->width;
		maxX %= pUI->width;
		pUI->nextX = maxX;
		pUI->nextY = maxY;
		tp->releaseMutex();
		while (y < pUI->height && (y < maxY || y == maxY && x < maxX)) {
			pUI->raytracer->tracePixel(x, y);
			x++;
			if (x >= pUI->width) {
				x = 0;
				y++;
			}
		}
		tp->holdMutex();
		x = pUI->nextX;
		y = pUI->nextY;
	}
	tp->releaseMutex();
}
#endif
