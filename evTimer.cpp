#include "evTimer.h"

#include "Common/logfile.h"
#include "include/libevwork/EVWork.h"

OBJPOOL_IMPLEMENTATION(EvTimer, EV_TIMER_POOL_SIZE)

using evwork::CEnv;

//struct ev_loop* _s_timer_loop = ev_default_loop();
struct ev_loop* _s_timer_loop = NULL;// CEnv::getEVLoop()->getEvLoop();

void EvTimer::ImplTimer()
{
	if (_s_timer_loop == NULL) {
		_s_timer_loop = CEnv::getEVLoop()->getEvLoop();
	}
	ev_loop(_s_timer_loop, 0);
}

EvTimer::EvTimer()
	:_enable(false)
{

}

EvTimer::~EvTimer()
{
	Stop();
}

void EvTimer::CreateTimer(double after, double repeat, TIMER_CALLBACK& proc, bool started)
{
	if (_s_timer_loop == NULL) {
		_s_timer_loop = CEnv::getEVLoop()->getEvLoop();
	}
	_after = after;
	_repeat = repeat;
	_proc.swap(proc);
	if(started) Reset();
}

void EvTimer::Stop(void)
{
	if (_enable) {
		ev_timer_stop(_s_timer_loop, &_ev_timer);
		_enable = false;
	}
}

void EvTimer::Reset(void)
{
	if (!_enable) {
		_ev_timer.data = this;
		ev_timer_init(&_ev_timer, OnTimer_cb, _after, _repeat);
		ev_timer_start(_s_timer_loop, &_ev_timer);
		_enable = true;
	}
}

int EvTimer::GetRemain(void)
{
	if (_enable) {
		return (int)ev_timer_remaining(_s_timer_loop, &_ev_timer);
	}
	return 0;
}

void EvTimer::OnTimer_cb(struct ev_loop* loop, struct ev_timer *watcher, int revents)
{
	if (EV_ERROR & revents) {
		logDebug("EvTimer::OnTimer_cb invalid event");
		return;
	}
	EvTimer* ths = (EvTimer*)watcher->data;
	ths->Process();
}

void EvTimer::Process(void)
{
	if (static_cast<int>(_repeat) == 0) {
		_enable = false;
	}

	if (_proc) {
		_proc();
	}else{
		logDebug("invalid timer");
	}
}
