/*
 * DefaultThreadManager.h
 * Desktop
 *
 *  Created on: Sep 21, 2012
 *      Author: Mitchell Wills
 */

#ifndef DEFAULTTHREADMANAGER_H_
#define DEFAULTTHREADMANAGER_H_


class DefaultThreadManager;
class PeriodicNTThread;

#include "networktables2/thread/PeriodicRunnable.h"
#include "networktables2/thread/NTThreadManager.h"
#include "networktables2/thread/NTThread.h"

#if (defined __vxworks || defined WIN32)

#include "OSAL/Task.h"

class DefaultThreadManager : public NTThreadManager{
public:
	virtual NTThread* newBlockingPeriodicThread(PeriodicRunnable* r, const char* name);
};

class PeriodicNTThread : public NTThread {
private:
	const char* name;
	NTTask* thread;
	PeriodicRunnable* r;
	bool run;
	bool detached;
	int _taskMain();
	static int taskMain(PeriodicNTThread* o);
public:
	PeriodicNTThread(PeriodicRunnable* r, const char* name);
	virtual ~PeriodicNTThread();
	virtual void stop();
	virtual void detach();
	virtual bool isRunning();
};

#else

#include <pthread.h>

class DefaultThreadManager : public NTThreadManager{
	virtual NTThread* newBlockingPeriodicThread(PeriodicRunnable* r, const char* name);
};

class PeriodicNTThread : public NTThread {
private:
	pthread_t thread;
	PeriodicRunnable* r;
	bool run;
	bool detached;
	void _taskMain();
	static void* taskMain(PeriodicNTThread* o);
public:
	PeriodicNTThread(PeriodicRunnable* r, const char* name);
	virtual ~PeriodicNTThread();
	virtual void stop();
	virtual void detach();
	virtual bool isRunning();
};
#endif // __vxworks


#endif /* DEFAULTTHREADMANAGER_H_ */
