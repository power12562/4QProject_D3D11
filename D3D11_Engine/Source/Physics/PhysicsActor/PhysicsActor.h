#pragma once
#include <directxtk/SimpleMath.h>
#include "Physics/PhysXFramework.h"
#include "Physics/enum/CollisionDetectionMode.h"
#include "Physics/enum/ForceMode.h"

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Quaternion;

enum class PhysicsActorType
{
	DynamicBody,
	KinematicBody,
	StaticBody
};

class PhysicsActor
{
	px::PxRigidActor* actor{ nullptr };

	PhysicsActorType actor_type{};

	// 플래그의 관리 등은 RigidBody, Collider 컴포넌트를 통해 수행.
	// 여기 있는 건 디버그용.
	bool freeze_move_x{ false };
	bool freeze_move_y{ false };
	bool freeze_move_z{ false };
	bool freeze_rotate_x{ false };
	bool freeze_rotate_y{ false };
	bool freeze_rotate_z{ false };

	bool use_gravity{ true };
	bool send_event_on_sleep_and_wake{ false };
	bool enable_simulation{ true };

	bool is_kinematic{ false };

	bool automatic_center_of_mass{ true };
	bool automatic_tensor{ true };

	float mass{ 1.0f };
	float drag{ 0.0f };
	float angular_drag{ 0.05f };

	float sleep_threshold{ 1.0f };

	CollisionDetectionMode collision_detection_mode{ CollisionDetectionMode::Discrete };

public:
	PhysicsActor(class GameObject* bind_object, px::PxScene* scene, Vector3 init_pos, Quaternion init_rot, PhysicsActorType actor_type);
	~PhysicsActor();

	px::PxRigidActor* GetActor() { return actor; }
	PhysicsActorType GetActorType() { return actor_type; }

	void SetFreezeMoveX(bool freeze);
	void SetFreezeMoveY(bool freeze);
	void SetFreezeMoveZ(bool freeze);
	void SetFreezeRotateX(bool freeze);
	void SetFreezeRotateY(bool freeze);
	void SetFreezeRotateZ(bool freeze);

	void SetUseGravity(bool use_gravity);
	void SetSendEventOnSleepAndWake(bool send_event);
	void SetEnableSimulation(bool enable_simulation);

	void SetIsKinematic(bool is_kinematic);
	void SetMass(float mass);
	void SetDrag(float drag);
	void SetAngularDrag(float angular_drag);

	// 이 둘은 PhysX가 알아서 해주는 거 같은데. 굳이 건들지 말자.
	//void SetAutomaticCenterOfMass(bool automatic_center_of_mass);
	//void SetAutomaticTensor(bool automatic_tensor);

	void SetCollisionDetectionMode(CollisionDetectionMode detection_mode);



public:
	void AddForce(Vector3 force, ForceMode force_mode);
	void AddTorque(Vector3 force, ForceMode force_mode);

	void SetVelocity(Vector3 velocity);
	void SetAngularVelocity(Vector3 velocity);



public:
	void SetSleepThreshold(float threshold);



public:
	void PropagateTransform(class Transform* transform);
	void UpdateTransform(class Transform* transform);
};