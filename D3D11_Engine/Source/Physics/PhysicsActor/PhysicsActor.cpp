#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"
#include "GameObject/Base/GameObject.h"


PhysicsActor::PhysicsActor(GameObject* bind_object, px::PxScene* scene, Vector3 init_pos, Quaternion init_rot, PhysicsActorType actor_type)
{
	px::PxPhysics* physics = PhysicsManager::GetInstance().GetPhysics();

	px::PxTransform transform{ PhysicsManager::ConvertToPxVec(init_pos), PhysicsManager::ConvertToPxQuat(init_rot) };

	this->actor_type = actor_type;
	if (actor_type == PhysicsActorType::StaticBody)
	{
		actor = physics->createRigidStatic(transform);
	}
	else if (actor_type == PhysicsActorType::DynamicBody)
	{
		actor = physics->createRigidDynamic(transform);
	}
	else if (actor_type == PhysicsActorType::KinematicBody)
	{
		actor = physics->createRigidDynamic(transform);
		SetIsKinematic(true);
	}

	scene->addActor(*actor);
	SetSleepThreshold(sleep_threshold);

	actor->userData = static_cast<void*>(bind_object);
}

PhysicsActor::~PhysicsActor()
{
	// 별도로 PxScene::releaseActor()를 호출하지 않아도 알아서 제거됨
	// PxActor와 PxShape는 얘기가 다르니 혼동하지 말자.
	if (actor != nullptr) actor->release();
}




void PhysicsActor::SetFreezeMoveX(bool freeze)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	freeze_move_x = freeze;
	static_cast<px::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(px::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, freeze);
}

void PhysicsActor::SetFreezeMoveY(bool freeze)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	freeze_move_y = freeze;
	static_cast<px::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(px::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, freeze);
}

void PhysicsActor::SetFreezeMoveZ(bool freeze)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	freeze_move_z = freeze;
	static_cast<px::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(px::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, freeze);
}

void PhysicsActor::SetFreezeRotateX(bool freeze)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	freeze_rotate_x = freeze;
	static_cast<px::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(px::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, freeze);
}

void PhysicsActor::SetFreezeRotateY(bool freeze)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	freeze_rotate_y = freeze;
	static_cast<px::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(px::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, freeze);
}

void PhysicsActor::SetFreezeRotateZ(bool freeze)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	freeze_rotate_z = freeze;
	static_cast<px::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(px::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, freeze);
}

void PhysicsActor::SetUseGravity(bool use_gravity)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	this->use_gravity = use_gravity;
	actor->setActorFlag(px::PxActorFlag::eDISABLE_GRAVITY, !use_gravity);
}

void PhysicsActor::SetSendEventOnSleepAndWake(bool send_event)
{
	send_event_on_sleep_and_wake = send_event;
	actor->setActorFlag(px::PxActorFlag::eSEND_SLEEP_NOTIFIES, send_event);
}

void PhysicsActor::SetEnableSimulation(bool enable_simulation)
{
	this->enable_simulation = enable_simulation;
	actor->setActorFlag(px::PxActorFlag::eDISABLE_SIMULATION, !enable_simulation);
}
void PhysicsActor::SetIsKinematic(bool is_kinematic)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	this->is_kinematic = is_kinematic;
	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
	rb->setRigidBodyFlag(px::PxRigidBodyFlag::eKINEMATIC, is_kinematic);
}
void PhysicsActor::SetMass(float mass)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
	rb->setMass(mass);
}
void PhysicsActor::SetDrag(float drag)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
	rb->setLinearDamping(drag);
}
void PhysicsActor::SetAngularDrag(float angular_drag)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
	rb->setAngularDamping(drag);
}
//void PhysicsActor::SetAutomaticCenterOfMass(bool automatic_center_of_mass)
//{
//	if (actor_type == PhysicsActorType::StaticBody) return;
//
//	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
//	rb->setRigidBodyFlag(px::PxRigidBodyFlag::eKINEMATIC, is_kinematic);
//}
//void PhysicsActor::SetAutomaticTensor(bool automatic_tensor)
//{
//	if (actor_type == PhysicsActorType::StaticBody) return;
//
//	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
//	rb->setRigidBodyFlag(px::PxRigidBodyFlag::eKINEMATIC, is_kinematic);
//}




void PhysicsActor::SetCollisionDetectionMode(CollisionDetectionMode detection_mode)
{
	if (actor_type == PhysicsActorType::StaticBody) return;
	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);

	collision_detection_mode = detection_mode;
	switch (detection_mode)
	{
	case CollisionDetectionMode::Discrete:
		rb->setRigidBodyFlag(px::PxRigidBodyFlag::eENABLE_CCD, false);
		break;
	case CollisionDetectionMode::Continuous:
		rb->setRigidBodyFlag(px::PxRigidBodyFlag::eENABLE_CCD, true);
		break;
	case CollisionDetectionMode::ContinuousDynamic:
		rb->setRigidBodyFlag(px::PxRigidBodyFlag::eENABLE_CCD, true);
		rb->setRigidBodyFlag(px::PxRigidBodyFlag::eENABLE_CCD_FRICTION, true);
		break;
	}
}

void PhysicsActor::AddForce(Vector3 force, ForceMode force_mode)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);

	px::PxVec3 _force = PhysicsManager::ConvertToPxVec(force);
	switch (force_mode)
	{
	case ForceMode::Force:
		rb->addForce(_force, px::PxForceMode::eFORCE);
		break;
	case ForceMode::Impulse:
		rb->addForce(_force, px::PxForceMode::eIMPULSE);
		break;
	case ForceMode::Acceleration:
		rb->addForce(_force, px::PxForceMode::eACCELERATION);
		break;
	case ForceMode::VelocityChange:
		rb->addForce(_force, px::PxForceMode::eVELOCITY_CHANGE);
		break;
	}
}

void PhysicsActor::AddTorque(Vector3 force, ForceMode force_mode)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);

	px::PxVec3 _force = PhysicsManager::ConvertToPxVec(force);
	switch (force_mode)
	{
	case ForceMode::Force:
		rb->addTorque(_force, px::PxForceMode::eFORCE);
		break;
	case ForceMode::Impulse:
		rb->addTorque(_force, px::PxForceMode::eIMPULSE);
		break;
	case ForceMode::Acceleration:
		rb->addTorque(_force, px::PxForceMode::eACCELERATION);
		break;
	case ForceMode::VelocityChange:
		rb->addTorque(_force, px::PxForceMode::eVELOCITY_CHANGE);
		break;
	}
}

void PhysicsActor::SetVelocity(Vector3 velocity)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
	rb->setLinearVelocity(PhysicsManager::ConvertToPxVec(velocity));
}

void PhysicsActor::SetAngularVelocity(Vector3 velocity)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
	rb->setAngularVelocity(PhysicsManager::ConvertToPxVec(velocity));
}

void PhysicsActor::SetSleepThreshold(float threshold)
{
	if (actor_type == PhysicsActorType::StaticBody) return;

	sleep_threshold = threshold;
	px::PxRigidDynamic* rb = static_cast<px::PxRigidDynamic*>(actor);
	rb->setSleepThreshold(threshold);
}

void PhysicsActor::PropagateTransform(Transform* transform)
{
	px::PxVec3 pos = PhysicsManager::ConvertToPxVec(transform->GetPosition());
	px::PxQuat quat = PhysicsManager::ConvertToPxQuat(transform->GetRotation());

	px::PxTransform px_transform{ pos, quat };
	actor->setGlobalPose(px_transform);
}

void PhysicsActor::UpdateTransform(Transform* transform)
{
	const px::PxTransform& px_transform = actor->getGlobalPose();

	Vector3 position{ PhysicsManager::ConvertToVector3(px_transform.p) };
	Quaternion rotation{ PhysicsManager::ConvertToQuaternion(px_transform.q) };

	Vector3 parent_position = { 0, 0, 0 };
	if (Transform* pt = transform->GetParent(); pt != nullptr)
	{
		parent_position = pt->GetPosition();
	}
	Vector3 relative_position = position - parent_position;

	Quaternion parent_rotation = { 0, 0, 0, 1 };
	if (Transform* pt = transform->GetParent(); pt != nullptr)
	{
		parent_rotation = pt->GetRotation();
	}
	Quaternion parent_rotation_inverse;
	parent_rotation.Inverse(parent_rotation_inverse);
	Quaternion relative_rotation = parent_rotation_inverse * rotation;

	transform->SetPosition(relative_position);
	transform->SetRotation(relative_rotation);
}