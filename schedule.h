#ifndef CODEYZ_SCHEDULE
#define CODEYZ_SCHEDULE

#include <queue>
#include <algorithm>
#include <pthread.h>
using namespace std;

void PthreadCall(const char* msg, int result);
long long GetCurrentMilli();

/**
 * Task to be scheduled
 */
struct SItem
{
	void* arg;
	void (*function)(void*);
	uint64_t s_time;
	// sort by s_time ascendingly
	bool operator < (const SItem& s) const
	{
		return s_time >= s.s_time;
	}
};

class Schedule
{
public:
	Schedule();
	~Schedule();

	/**
	 * add task into scheduler, do it after {millisec}
	 * return false if failed to add
	 */
	bool Sched(void (*function)(void*), void* arg, uint64_t millisec);

	/*
	 * pause to schedule
	 */
	void Pause();

	/**
	 * continue
	 */
	void Continue();

	/**
	 * stop
	 */
	void Stop();
	
private:
	// task queue
	priority_queue<SItem> m_tasks;
	// background thread that makes schedule
	pthread_t m_bgthread;
	// mark whether schedule starts or not
	bool m_started_bgthread;
	// paused
	bool m_paused;
	// stop
	bool m_stop;
	pthread_mutex_t m_mu;
	pthread_cond_t m_bgsignal;

	
	/**
	 * practically making schedule
	 */
	void BGThread();

	static void* BGThreadWrapper(void* arg);
};

#endif
