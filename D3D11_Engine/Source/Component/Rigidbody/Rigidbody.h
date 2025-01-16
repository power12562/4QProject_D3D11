#pragma once
#include "Component/Base/Component.h"
#include "Physics/enum/CollisionDetectionMode.h"
#include "Physics/enum/ForceMode.h"


class Rigidbody : public Component
{
	float mass{ 1.0f };
	float drag{ 0.0f };
	float angular_drag{ 0.05f };

	bool automatic_center_of_mass{ true };
	bool automatic_tensor{ true };

	bool use_gravity{ true };
	bool is_kinematic{ false };

	CollisionDetectionMode collision_detection_mode{ CollisionDetectionMode::Discrete };

public:
	void SetMass(float mass);
	void SetDrag(float drag);
	void SetAngularDrag(float angular_drag);

	// 이 둘은 PhysX가 알아서 해주는 거 같은데. 굳이 건들지 말자.
	/*void SetAutomaticCenterOfMass(bool automatic_center_of_mass);
	void SetAutomaticTensor(bool automatic_tensor);*/

	void SetUseGravity(bool use_gravity);
	void SetIsKinematic(bool is_kinematic);
	void SetCollisionDetectionMode(CollisionDetectionMode collision_detection_mode);


public:
	void AddForce(Vector3 force, ForceMode force_mode);
	void AddTorque(Vector3 force, ForceMode force_mode);

	void SetVelocity(Vector3 velocity);
	void SetAngularVelocity(Vector3 velocity);

	void SetSleepThreshold(float threshold);







public:
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}
};