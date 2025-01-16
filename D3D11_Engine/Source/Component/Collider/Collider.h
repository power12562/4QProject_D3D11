#pragma once
#include "Component/Base/Component.h"
#include "Physics/PhysXFramework.h"

enum class ColliderType
{
	Box,
	Sphere,
	Capsule
};

class Collider : public Component
{
	friend class GameObject;
	friend class PhysicsManager;

protected:
	ColliderType type;
	Vector3 anchor{ 0, 0, 0 };

	bool is_trigger{ false };
	bool need_to_update_shape{ false };

	class PhysicsMaterial* physics_material{ nullptr };
protected:
	px::PxShape* shape{ nullptr };
	unsigned int physics_layer_slot;
public:
	Collider();
	~Collider();

	bool GetIsTrigger() { return is_trigger; }

	Vector3 GetAnchor() { return anchor; }
	void SetAnchor(Vector3 anchor) { this->anchor = anchor; }

	ColliderType GetType() { return type; }
	class PhysicsMaterial* GetPhysicsMaterial() { return physics_material; }

	void SetPhysicsMateiral(class PhysicsMaterial* material);

	virtual void DrawBounds(Color color = Color{ 1,0,0,1 }) {};
	virtual void SetIsTrigger(bool is_trigger) abstract;
	virtual void AttachShapeToActor() abstract;
	virtual void UpdateShape() abstract;
	virtual void DetachShapeToActor() abstract;
	virtual void SetSizeFollowingMesh() abstract;




public:
	virtual void Start() override {}
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}
};
