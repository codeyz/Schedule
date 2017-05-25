#include "schedule.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

void PthreadCall(const char* msg, int result)
{
	if(result)
	{
		cout << msg << ": " << strerror(result) << endl;
		abort();
	}
}

long long GetCurrentMilli()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

Schedule::Schedule()
{
	m_started_bgthread = false;
	m_paused = false;
	m_stop = false;
	PthreadCall("mutex init", pthread_mutex_init(&m_mu, NULL));
	PthreadCall("cond init", pthread_cond_init(&m_bgsignal, NULL));
}

Schedule::~Schedule()
{
	PthreadCall("mutex destroy", pthread_mutex_destroy(&m_mu));
	PthreadCall("cond destroy", pthread_cond_destroy(&m_bgsignal));
}

void* Schedule::BGThreadWrapper(void* arg)
{
	reinterpret_cast<Schedule*>(arg)->BGThread();
	return NULL;
}

bool Schedule::Sched(void (*function)(void*), void* arg, uint64_t millisec)
{
	PthreadCall("mutex lock to sched", pthread_mutex_lock(&m_mu));
	if(m_stop)
	{
		PthreadCall("mutex unlock to sched", pthread_mutex_unlock(&m_mu));
		return false;
	}
	if(!m_started_bgthread)
	{
		m_started_bgthread = true;
		PthreadCall("bgthread create", pthread_create(&m_bgthread, NULL, &Schedule::BGThreadWrapper, this));
	}
	if(m_tasks.empty())
	{
		PthreadCall("bgthread signal", pthread_cond_signal(&m_bgsignal));
	}
	SItem item;
	item.arg = arg;
	item.function = function;
	item.s_time = GetCurrentMilli() + millisec;
	m_tasks.push(item);
	PthreadCall("mutex unlock to sched", pthread_mutex_unlock(&m_mu));
	return true;
}

void Schedule::Pause()
{
	PthreadCall("mutex lock to pause", pthread_mutex_lock(&m_mu));
	m_paused = true;
	PthreadCall("mutex unlock to pause", pthread_mutex_unlock(&m_mu));
}

void Schedule::Continue()
{
	PthreadCall("mutex lock to continue", pthread_mutex_lock(&m_mu));
	if(m_paused)
	{
		m_paused = false;
		PthreadCall("bgthread signal", pthread_cond_signal(&m_bgsignal));
	}
	PthreadCall("mutex unlock to continue", pthread_mutex_unlock(&m_mu));
}

void Schedule::Stop()
{
	m_stop = true;
	PthreadCall("bgthread signal", pthread_cond_signal(&m_bgsignal));
	PthreadCall("join", pthread_join(m_bgthread, NULL));
}

void Schedule::BGThread()
{
	while(true)
	{
		PthreadCall("mutex lock to run task", pthread_mutex_lock(&m_mu));
		if(m_stop) 
			break;
		if(m_tasks.empty() || m_paused)
			PthreadCall("wait to run task", pthread_cond_wait(&m_bgsignal, &m_mu));
		if(m_stop) // it may be {Stop} wake up bgthread
			break;
		void (*function)(void*) = m_tasks.top().function;
		void* arg = m_tasks.top().arg;
		uint64_t s_time = m_tasks.top().s_time;
		if(GetCurrentMilli() >= s_time) // this task is expired
		{
			m_tasks.pop();
			PthreadCall("mutex unlock release task", pthread_mutex_unlock(&m_mu));
			(*function)(arg);
		}
		else
		{
			PthreadCall("mutex unlock release task", pthread_mutex_unlock(&m_mu));
			sleep(1); // have a rest
		}
	}
	PthreadCall("mutex unlock to exit", pthread_mutex_unlock(&m_mu));
	pthread_exit(NULL);
}
