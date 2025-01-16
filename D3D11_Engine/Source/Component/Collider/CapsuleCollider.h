#pragma once
#include <Component/Base/Component.h>
#include <Component/Collider/Collider.h>


class CapsuleCollider : public Collider
{
	float body_height{ 1.0f }; // half_height �ƴ�!
	float radius{ 1.0f };
public:
	CapsuleCollider() {}
	~CapsuleCollider();

	void SetSize(float body_height, float radius);
	virtual void SetSizeFollowingMesh() override; // �ʱ�ȭ�� / �޽� ���� �ֵ��� �׳� �⺻ ������� UpdateShape ȣ���ؾ� ��.
	virtual void DrawBounds(Color color) override {};

	virtual void AttachShapeToActor() override;
	virtual void UpdateShape() override;
	virtual void DetachShapeToActor() override;

	virtual void SetIsTrigger(bool is_trigger) override;



public:
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}
};
