#include "UpdateTimer.h"

UpdateTimer::UpdateTimer(double freq) : _timer{ 0 }, _ticksPerUpdate{ 1000 / freq } 
{
	_last = SDL_GetTicks();
}

bool UpdateTimer::Tick()
{
	uint64_t ticks = SDL_GetTicks();
	uint64_t delta = ticks - _last;
	_last = ticks;

	_timer += delta;

	if (_timer >= _ticksPerUpdate) {
		_timer -= _ticksPerUpdate;
		return true;
	}

	return false;
}
