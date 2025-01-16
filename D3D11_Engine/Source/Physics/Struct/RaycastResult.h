#pragma once
#include <vector>
#include <directxtk/SimpleMath.h>
using namespace DirectX::SimpleMath;

struct RaycastHit
{
	// �浹�� �ݶ��̴��� ������ ������Ʈ
	class GameObject* object{ nullptr };
	// ���а� �浹�� �ݶ��̴�
	class Collider* collider{ nullptr };
	// �浹 ���� ��ǥ
	Vector3 position;
	// ray origin���κ����� �Ÿ�
	float distance;
};

struct RaycastResult
{
	std::vector<RaycastHit> hits;
};