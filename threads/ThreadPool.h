// ThreadPool.h
// Written by Joshua Snyder
// 10/26/2008
// A class that wraps Win32 threads and pthreads
// to do basic parallel computations

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <vector>
#endif

class ThreadPool
{
public:
	ThreadPool();
	virtual ~ThreadPool();
	void holdMutex();
	void releaseMutex();
	typedef void(*threadFunc)(ThreadPool*, void*);
	bool startThread(threadFunc func, void* arg);
	bool waitForThreads(unsigned int millis);
	static const unsigned int NO_TIMEOUT = (unsigned)(-1);
private:
#ifdef WIN32
	HANDLE winMutex;
	DWORD winNumThreads;
	DWORD winThreadBufSize;
	HANDLE *winThreads;
#else // !WIN32
	std::vector<pthread_t> posThreads;
	pthread_cond_t posWaitCond;
	pthread_mutex_t posInternalMutex;
	pthread_mutex_t posUserMutex;
	int posRunningThreads;
	static void* posix_func_wrapper(void* arg);
#endif
};

#endif
