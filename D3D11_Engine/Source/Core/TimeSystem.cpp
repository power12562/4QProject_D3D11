#include "TimeSystem.h"

namespace TimeSystem
{
	QPCTime& Time = QPCTime::GetInstance();
	float FixedTimeStep = 0.02f; //fixed Update 호출 주기.
}

QPCTime::QPCTime() :
	timeScale(1.0f)
{
	previousTime = { 0 };
	currentTime = { 0 };
	frequency = { 0 };
	deltaTime_ms = 0.f;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&previousTime);
}

void QPCTime::UpdateTime()
{
	previousTime = currentTime;
	QueryPerformanceCounter(&currentTime);

	deltaTime_ms = float(currentTime.QuadPart - previousTime.QuadPart) / float(frequency.QuadPart / 1000.0); //ms
}

float QPCTime::GetDeltaTime_ms(bool isScale) const
{
	if (isScale)
	{
		return deltaTime_ms * timeScale;
	}
	else
	{
		return deltaTime_ms;
	}
	
}
