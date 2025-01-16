#include <Utility/Random.h>

std::random_device Random::rd;
std::mt19937 Random::gen;
std::uniform_int_distribution<> Random::dis;

int Random::Range(int a, int b)
{
	std::mt19937 genTmp(rd());
	gen = genTmp;
	std::uniform_int_distribution<> disTmp(a, b);
	dis = disTmp;

	return dis(gen);
}

float Random::Range(float a, float b)
{
	int integer = Range((int)(a * 10000), (int)(b * 10000));
	return (float)(integer / 10000.0f);
}
