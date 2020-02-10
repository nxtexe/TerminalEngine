#ifndef GAMETIMER_H
#define GAMETIMER_H
#include<windows.h>
class GameTimer {
public:
	GameTimer();

	float TotalTime() const;//in seconds
	float DeltaTime() const;//in seconds

	void Reset();//call before message loop
	void Start();//call when unpaused
	void Stop();//call when paused
	void Tick();//call every frame
private:
	double mSecondsPerCount;
	double mDeltaTime;
	bool mStopped;
	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;
};
#endif