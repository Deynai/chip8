#pragma once
#include <SDL3/SDL.h>

class UpdateTimer
{
public:
	UpdateTimer(double frequency);
	
	// Poll the timer. Returns true if the update threshold has been hit and false if not
	bool Tick();

private:
	double _ticksPerUpdate;
	double _timer;
	double _last;
};
