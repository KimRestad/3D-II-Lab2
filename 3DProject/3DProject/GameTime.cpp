#include "GameTime.h"

// Constructor - initialize the game time, set start values for teh time stamps and calculate the length of a tick
GameTime::GameTime() 
	:mPrevTimeStamp(0), mGameStartTime(0), mMilliSecondsPerTick(0.0)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mMilliSecondsPerTick = 1000.0 / (double)countsPerSec;

	QueryPerformanceCounter((LARGE_INTEGER*)&mGameStartTime);
	Update();
}

GameTime::~GameTime()
{
}

// Update the game time by calculating new values for elapsed time and getting new time stamp values
void GameTime::Update()
{
	__int64 currTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*) & currTimeStamp);

	mElapsedSinceLastTick.Milliseconds = (currTimeStamp - mPrevTimeStamp) * mMilliSecondsPerTick;
	mElapsedSinceLastTick.Seconds = mElapsedSinceLastTick.Milliseconds / 1000;

	mElapsedSinceStart.Milliseconds = (currTimeStamp - mGameStartTime) * mMilliSecondsPerTick;
	mElapsedSinceStart.Seconds = mElapsedSinceStart.Milliseconds / 1000;

	mPrevTimeStamp = currTimeStamp;
}

// Get the time elapsed since last update
Time GameTime::GetTimeSinceLastTick()
{
	return mElapsedSinceLastTick;
}

// Get the time elapsed since the game started
Time GameTime::GetTimeSinceGameStart()
{
	return mElapsedSinceStart;
}