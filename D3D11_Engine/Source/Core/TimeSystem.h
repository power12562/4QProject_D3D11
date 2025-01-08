#pragma once
#include <Core/TSingleton.h>
#include <windows.h>
#include <cmath>

class QPCTime : public TSingleton<QPCTime>
{
	friend class TSingleton;
private:
	QPCTime();
	~QPCTime() = default; 

public:
	void UpdateTime();
	const int GetFrameRate() const
	{
		if (deltaTime_ms == 0) return 0;

		return static_cast<int>(std::ceil(((1000.0f / deltaTime_ms) * 1000.f) / 1000.f));
	}

	float GetDeltaTime_ms(bool isScale = true) const;
	float GetDeltaTime(bool isScale = true) const { return (GetDeltaTime_ms(isScale) / 1000.f);}

	/* DeltaTime (단위 : sec). timeScale 영향 받습니다.*/
	float GetPropertyDeltaTime() const { return deltaTime_ms * timeScale / 1000.f; }
	/* DeltaTime (단위 : sec). timeScale 영향 받습니다.*/
	__declspec(property(get = GetPropertyDeltaTime)) float DeltaTime;

	/* DeltaTime (단위 : ms). timeScale 영향 받습니다.*/
	float GetPropertyDeltaTime_ms() const { return deltaTime_ms * timeScale; }
	/* DeltaTime (단위 : ms). timeScale 영향 받습니다.*/
	__declspec(property(get = GetPropertyDeltaTime_ms)) float DeltaTime_ms;

	int FixedUpdatePerSec = 50;
	float GetFixedDelta() const { return 1.f / (float)FixedUpdatePerSec; }

	float timeScale;

private:
	LARGE_INTEGER previousTime;
	LARGE_INTEGER currentTime;
	LARGE_INTEGER frequency;

	float deltaTime_ms = 0;
};

namespace TimeSystem
{
	extern QPCTime& Time;
	extern float FixedTimeStep; //fixed Update 호출 주기.
}
