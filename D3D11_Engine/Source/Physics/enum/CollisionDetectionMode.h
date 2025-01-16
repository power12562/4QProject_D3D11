#pragma once
enum class CollisionDetectionMode
{
	Discrete,
	Continuous, // 그냥 CCD 알고리즘만 적용
	ContinuousDynamic // CCD + 마찰력까지 계산하는 가장 현실적이고 비싼 연산
};