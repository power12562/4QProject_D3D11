#pragma once
enum class ForceMode
{
	// ���� ���
	Force, // divide by deltaTime
	Impulse,
	// ���� ����
	Acceleration, // divide by deltaTime
	VelocityChange
};