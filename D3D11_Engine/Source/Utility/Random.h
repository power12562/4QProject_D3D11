#pragma once
#include <random>

class Random
{
	static std::random_device rd;
	static std::mt19937 gen;
	static std::uniform_int_distribution<> dis;
public:
	// [a, b] ������ ������ ������ ��ȯ�մϴ�.
	static int Range(int a, int b);
	// [a, b] ������ ������ �Ǽ��� ��ȯ�մϴ�. �Ҽ����� �ִ� 4�ڸ�.
	static float Range(float a, float b);
};