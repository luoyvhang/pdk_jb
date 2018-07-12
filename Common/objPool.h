#ifndef _OBJ_POOL_H_67358BE8013941CCA2AB15FA2740EECD
#define _OBJ_POOL_H_67358BE8013941CCA2AB15FA2740EECD

#include <list>
#include <malloc.h>

#ifdef MULTIPLE_THREAD
	#include "../Common/multpleThread.h"
	#define SAFE_MUTEX	mutex_	_mutex;
	#define SAFE_THREAD	lockguard_ _lck(_mutex);
	#define SAFE_INIT	MUTEX_INIT(&_mutex);
	#define SAFE_DESTROY	MUTEX_DELETE(&_mutex);
#else
	#define SAFE_MUTEX
	#define SAFE_THREAD
	#define SAFE_INIT
	#define SAFE_DESTROY
#endif


/* 简单对象池的实现.使用缓存对象实现.
		指定maxSize为最大缓存数量.
	在类中使用 OBJPOOL_xxx 俩个宏,实现重载new/delete
	示例:
		*.H
		class myClass
		{
				OBJPOOL_DECLARATION(myClass,1024)
		}

		*.CPP
		OBJPOOL_IMPLEMENTATION(myClass,1024)


		myClass* cls=new myClass;
		delete myClass;	
*/
template<class T, int maxSize = 1024>
class ObjectPool
{
public:
	ObjectPool()
	{
		SAFE_INIT;
		_size = 0;
	}
	~ObjectPool()
	{
		SAFE_DESTROY;
		FreeAllObject();
	}

	/*获取对象*/
	T*		GetObject(void)
	{
		SAFE_THREAD;
		T* pObj = NULL;
		if (_pool.size()) {
			--_size;
			pObj = _pool.front();
			_pool.pop_front();
		}else{
			pObj = (T*)malloc(sizeof(T));
		}
		return pObj;
	}

	/*释放对象*/
	void	FreeObject(T* p)
	{
		SAFE_THREAD;
		//超过最大缓存数量.直接释放.
		if (_size >= maxSize) {
			free((void*)p);
			return;
		}
		++_size;
		_pool.push_back(p);
	}

	//debug
	int		GetSize(void)
	{
		return _size;
	}
private:
	typedef typename std::list<T*>::iterator ITER_OBJECT;
	void	FreeAllObject(void)
	{
		for (ITER_OBJECT it = _pool.begin(); it != _pool.end(); ++it) {
			T* pObj = *it;
			if (pObj) free(pObj);
		}
		_size = 0;
		_pool.clear();
	}

	std::list<T*>		_pool;
	int					_size;
	SAFE_MUTEX
};

#define OBJPOOL_DECLARATION(type,max)				\
	public:											\
		void*	operator new(size_t);				\
		void	operator delete(void*);				\
	private:										\
		static ObjectPool<type,max> __s_##type##_pool;

#define OBJPOOL_IMPLEMENTATION(type,max)			\
	ObjectPool<type,max> type::__s_##type##_pool;	\
	void* type::operator new(size_t){				\
		return (void*)__s_##type##_pool.GetObject();}\
	void type::operator delete(void* p){			\
		__s_##type##_pool.FreeObject((type*)p);}

#define OBJPOOL_GETSIZE(type)	__s_##type##_pool.GetSize()

#endif