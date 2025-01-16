#pragma once
#include <random>

class Random
{
	static std::random_device rd;
	static std::mt19937 gen;
	static std::uniform_int_distribution<> dis;
public:
	// [a, b] 구간의 무작위 정수를 반환합니다.
	static int Range(int a, int b);
	// [a, b] 구간의 무작위 실수를 반환합니다. 소수점은 최대 4자리.
	static float Range(float a, float b);
};