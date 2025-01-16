#pragma once
#include <Component/Base/Component.h>
#include <Component/Collider/Collider.h>

class BoxCollider : public Collider
{
	float width{ 1.0f };
	float height{ 1.0f };
	float depth{ 1.0f };

public:
	BoxCollider() {}
	~BoxCollider();

	void SetSize(float width, float height, float depth);
	virtual void SetSizeFollowingMesh() override; // �ʱ�ȭ�� / �޽� ���� �ֵ��� �׳� �⺻ ������� UpdateShape ȣ���ؾ� ��.
	virtual void DrawBounds(Color color = { 1, 0, 0, 1 }) override;

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