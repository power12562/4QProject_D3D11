#pragma once
enum class CollisionDetectionMode
{
	Discrete,
	Continuous, // �׳� CCD �˰��� ����
	ContinuousDynamic // CCD + �����±��� ����ϴ� ���� �������̰� ��� ����
};