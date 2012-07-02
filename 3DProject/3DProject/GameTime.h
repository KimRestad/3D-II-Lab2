#ifndef GAMETIME_H
#define GAMETIME_H

#include <Windows.h>

struct Time
{
	double Milliseconds;
	double Seconds;
};

class GameTime
{
public:
	GameTime();
	~GameTime();
	void Update();
	Time GetTimeSinceLastTick();
	Time GetTimeSinceGameStart();

private:
	__int64 mPrevTimeStamp;
	__int64 mGameStartTime;

	double mMilliSecondsPerTick;

	Time mElapsedSinceStart;
	Time mElapsedSinceLastTick;
};

#endif