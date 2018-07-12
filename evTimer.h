#pragma once

#include "Common/objPool.h"
#include <ev.h>

#if __cplusplus >= 201103L
#include <functional>
#define CXX11	std
#else
#include <tr1/functional>
#define CXX11	std::tr1
#endif

//using TIMER_CALLBACK = std::function<void(void)>;
typedef CXX11::function<void(void)> TIMER_CALLBACK;

#define		EV_TIMER_POOL_SIZE	128
/* 基于libev的计时器,
	必须已调用 ev_loop(ev_default_loop(),0);
	或者调用. EvTimer::ImplTimer();
*/

class EvTimer
{
	OBJPOOL_DECLARATION(EvTimer, EV_TIMER_POOL_SIZE)
public:
	EvTimer();
	~EvTimer();

	static	void	ImplTimer(void);

	/*创建计时器.
		@after			多少秒数后触发.
		@repeat			重复触发时间.
		@proc			回调函数.std::function<void(void)>
 	*/
	void			CreateTimer(double after, double repeat, TIMER_CALLBACK& proc, bool started = true);
	/*停止*/
	void			Stop(void);
	/*重置并启动计时器*/
	void			Reset(void);

	int				GetRemain(void);
protected:
	static	void	OnTimer_cb(struct ev_loop* loop, struct ev_timer *watcher, int revents);
	void			Process(void);
private:
	ev_timer		_ev_timer;
	double			_after;
	double			_repeat;
	bool			_enable;
	TIMER_CALLBACK	_proc;
};