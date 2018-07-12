#ifndef _MULTIPLE_THREAD_H_5189A213C6B54AF986B0A3C5C4E933AA
#define _MULTIPLE_THREAD_H_5189A213C6B54AF986B0A3C5C4E933AA

	#if __cplusplus >= 201103L

		#include <mutex>
		using mutex_ = std::recursive_mutex;
		using lockguard_ = std::lock_guard<mutex_>;

		#define MUTEX_INIT(x)
		#define	MUTEX_DELETE(x)
		#define MUTEX_LOCKED(x)
		#define MUTEX_UNLOCK(x)
	#else

		#if defined(WIN32) || defined(WIN64)
			#include <windows.h>
			typedef CRITICAL_SECTION mutex_;

			#define MUTEX_INIT(x)	InitializeCriticalSection(x)
			#define MUTEX_LOCKED(x)	EnterCriticalSection(x)
			#define MUTEX_UNLOCK(x) LeaveCriticalSection(x)
			#define MUTEX_DELETE(x)	DeleteCriticalSection(x);
		#else
			#include <pthread.h>
			typedef pthread_mutex_t mutex_;

			#define MUTEX_INIT(x)								\
			pthread_mutexattr_t attr;							\
			pthread_mutexattr_init(&attr);						\
			pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);\
			pthread_mutex_init(x,&attr)
			#define MUTEX_LOCKED(x)	pthread_mutex_lock(x)
			#define MUTEX_UNLOCK(x) pthread_mutex_unlock(x)
			#define MUTEX_DELETE(x)	pthread_mutex_destroy(x)
		#endif

		struct lockguard_
		{
			lockguard_(mutex_ mtx)
				:_mutex(mtx)
			{
				MUTEX_LOCKED(&_mutex);
			}
			~lockguard_()
			{
				MUTEX_UNLOCK(&_mutex);
			}
		private:
			mutex_	_mutex;
		};

	#endif

#endif