#pragma once
#include <vector>
#include <directxtk/SimpleMath.h>
using namespace DirectX::SimpleMath;

struct RaycastHit
{
	// 충돌한 콜라이더가 부착된 오브젝트
	class GameObject* object{ nullptr };
	// 선분과 충돌한 콜라이더
	class Collider* collider{ nullptr };
	// 충돌 월드 좌표
	Vector3 position;
	// ray origin으로부터의 거리
	float distance;
};

struct RaycastResult
{
	std::vector<RaycastHit> hits;
};