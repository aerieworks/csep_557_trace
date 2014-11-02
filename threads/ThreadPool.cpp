#include <iostream>
#include "ThreadPool.h"


#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#ifdef WIN32
#include <process.h>
#include <string.h>



typedef struct {
	ThreadPool* tp;
	ThreadPool::threadFunc func;
	void* arg;
} WinThreadClosure;

static unsigned int __stdcall win_func_wrapper(void* arg) {
	WinThreadClosure* c = (WinThreadClosure*) arg;
	c->func(c->tp, c->arg);
	delete c;
	_endthread();
	return 0;
}

ThreadPool::ThreadPool() {
	winMutex = CreateMutex(NULL, FALSE, NULL);
	winThreadBufSize = 10;
	winThreads = new HANDLE[winThreadBufSize];
	winNumThreads = 0;
}

ThreadPool::~ThreadPool() {
	CloseHandle(winMutex);
	while (winNumThreads)
		CloseHandle(winThreads[--winNumThreads]);
	delete [] winThreads;
}

void ThreadPool::holdMutex() {
	WaitForSingleObject(winMutex, INFINITE);
}

void ThreadPool::releaseMutex() {
	ReleaseMutex(winMutex);
}

bool ThreadPool::startThread(threadFunc func, void* arg) {
	WinThreadClosure* c = new WinThreadClosure;
	c->tp = this;
	c->func = func;
	c->arg = arg;
	HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, win_func_wrapper, c, 0, NULL);
	if (!thread) {
		delete c;
		return false;
	}
	if (winNumThreads + 1 >= winThreadBufSize) {
		winThreadBufSize *= 2;
		HANDLE *old = winThreads;
		winThreads = new HANDLE[winThreadBufSize];
		memcpy(winThreads, old, winNumThreads * sizeof(HANDLE));
		delete [] old;
	}
	winThreads[winNumThreads] = thread;
	winNumThreads++;
	return true;
}

bool ThreadPool::waitForThreads(unsigned int millis) {
	if (millis == NO_TIMEOUT)
		millis = INFINITE;
	bool done = WaitForMultipleObjects(winNumThreads, winThreads, TRUE, millis) != WAIT_TIMEOUT;
	while (done && winNumThreads)
		CloseHandle(winThreads[--winNumThreads]);
	return done;
}

#else // !WIN32

typedef struct {
	ThreadPool* tp;
	ThreadPool::threadFunc func;
	void* arg;
} PosixThreadClosure;

void* ThreadPool::posix_func_wrapper(void* arg) {
	PosixThreadClosure* c = (PosixThreadClosure*) arg;
	ThreadPool* tp = c->tp;
	c->func(tp, c->arg);
	delete c;
	pthread_mutex_lock(&tp->posInternalMutex);
	tp->posRunningThreads--;
	if (!tp->posRunningThreads)
		pthread_cond_signal(&tp->posWaitCond);
	pthread_mutex_unlock(&tp->posInternalMutex);
	return NULL;
}

ThreadPool::ThreadPool() {
	pthread_cond_init(&posWaitCond, NULL);
	pthread_mutex_init(&posInternalMutex, NULL);
	pthread_mutex_init(&posUserMutex, NULL);
	posRunningThreads = 0;
}

ThreadPool::~ThreadPool() {
	pthread_cond_destroy(&posWaitCond);
	pthread_mutex_destroy(&posInternalMutex);
	pthread_mutex_destroy(&posUserMutex);
}

void ThreadPool::holdMutex() {
	pthread_mutex_lock(&posUserMutex);
}

void ThreadPool::releaseMutex() {
	pthread_mutex_unlock(&posUserMutex);
}

bool ThreadPool::startThread(threadFunc func, void* arg) {
	pthread_t thread;
	PosixThreadClosure* c = new PosixThreadClosure;
	c->tp = this;
	c->func = func;
	c->arg = arg;
	if (pthread_create(&thread, NULL, posix_func_wrapper, c)) {
		delete c;
		return false;
	}
	posRunningThreads++;
	posThreads.push_back(thread);
	return true;
}

bool ThreadPool::waitForThreads(unsigned int millis) {
	bool threadsDone = false;
	if (millis != NO_TIMEOUT) {
		pthread_mutex_lock(&posInternalMutex);
		if (posRunningThreads) {
			timespec timeout;
#ifdef __APPLE__ // OS X does not have clock_gettime, use clock_get_time
      clock_serv_t cclock;
      mach_timespec_t mts;
      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);
      timeout.tv_sec = mts.tv_sec;
      timeout.tv_nsec = mts.tv_nsec;
#else
			clock_gettime(CLOCK_REALTIME, &timeout);
#endif
      timeout.tv_nsec += 1000000 * millis;
			timeout.tv_sec += timeout.tv_nsec / 1000000000;
			timeout.tv_nsec %= 1000000000;
			pthread_cond_timedwait(&posWaitCond, &posInternalMutex, &timeout);
			threadsDone = !posRunningThreads;
		}
		pthread_mutex_unlock(&posInternalMutex);
	}
	if (threadsDone || millis == NO_TIMEOUT) {
		void* ptr;
		for (std::vector<pthread_t>::iterator i = posThreads.begin(); i != posThreads.end(); i++) {
			pthread_t thread = *i;
			pthread_join(thread, &ptr);
		}
		posThreads.clear();
		return true;
	}
	return false;
}

#endif
