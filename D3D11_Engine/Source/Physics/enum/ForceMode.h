#pragma once
enum class ForceMode
{
	// 질량 고려
	Force, // divide by deltaTime
	Impulse,
	// 질량 무시
	Acceleration, // divide by deltaTime
	VelocityChange
};