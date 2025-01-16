#include "Component/Rigidbody/Rigidbody.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"

void Rigidbody::SetMass(float mass)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	this->mass = mass;
	actor->SetMass(mass);
}

void Rigidbody::SetDrag(float drag)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	this->drag = drag;
	actor->SetDrag(drag);
}

void Rigidbody::SetAngularDrag(float angular_drag)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	this->angular_drag = angular_drag;
	actor->SetAngularDrag(angular_drag);
}

//void Rigidbody::SetAutomaticCenterOfMass(bool automatic_center_of_mass)
//{
//	PhysicsActor* actor = GetOwner()->GetPhysicsActor();
//	if (actor == nullptr)
//	{
//		Debug::LogError("Physics actor not found.");
//		return;
//	}
//}
//
//void Rigidbody::SetAutomaticTensor(bool automatic_tensor)
//{
//	PhysicsActor* actor = GetOwner()->GetPhysicsActor();
//	if (actor == nullptr)
//	{
//		Debug::LogError("Physics actor not found.");
//		return;
//	}
//}

void Rigidbody::SetUseGravity(bool use_gravity)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	this->use_gravity = use_gravity;
	actor->SetUseGravity(use_gravity);
}

void Rigidbody::SetIsKinematic(bool is_kinematic)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	this->is_kinematic = is_kinematic;
	actor->SetIsKinematic(is_kinematic);
}

void Rigidbody::SetCollisionDetectionMode(CollisionDetectionMode collision_detection_mode)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	this->collision_detection_mode = collision_detection_mode;
	actor->SetCollisionDetectionMode(collision_detection_mode);
}

void Rigidbody::AddForce(Vector3 force, ForceMode force_mode)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	actor->AddForce(force, force_mode);
}

void Rigidbody::AddTorque(Vector3 force, ForceMode force_mode)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	actor->AddTorque(force, force_mode);
}

void Rigidbody::SetVelocity(Vector3 velocity)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	actor->SetVelocity(velocity);
}

void Rigidbody::SetAngularVelocity(Vector3 velocity)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	actor->SetAngularVelocity(velocity);
}

void Rigidbody::SetSleepThreshold(float threshold)
{
	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("Physics actor not found.");
		return;
	}

	actor->SetSleepThreshold(threshold);
}

void Rigidbody::Start()
{
	PhysicsManager::OnAddRigidbody(&GetGameObject(), this);
}