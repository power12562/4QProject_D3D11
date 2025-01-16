#include "Component/Collider/SphereCollider.h"
#include "Math/Mathf.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"


SphereCollider::~SphereCollider()
{
	DetachShapeToActor();
}

void SphereCollider::SetSize(float radius)
{
	this->radius = radius;
	UpdateShape();
	AttachShapeToActor();
}

void SphereCollider::SetSizeFollowingMesh()
{
	BoundingBox bb = GetGameObject().GetBBToWorld();

	radius = std::max<float>({ bb.Extents.x, bb.Extents.y, bb.Extents.z });
	UpdateShape();
}

void SphereCollider::DrawBounds(Color color)
{
	/*BoundingSphere bs;
	bs.Center = { 0, 0, 0 };
	bs.Radius = radius;
	bs.Transform(bs, GetOwner()->transform->GetWorldTransformMatrix());

	Gizmo::DrawBounds(bs, color);*/
}

void SphereCollider::AttachShapeToActor()
{
	if (shape == nullptr)
	{
		//Debug::LogError("PxShape not found.");
		return;
	}

	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("PxActor not found.");
		return;
	}

	actor->GetActor()->attachShape(*shape);
}

void SphereCollider::UpdateShape()
{
	if (shape != nullptr)
	{
		DetachShapeToActor();
		shape->release();
	}

	px::PxPhysics* physics = PhysicsManager::GetInstance().GetPhysics();

	Vector3 scale = GetGameObject().transform.GetScale();
	px::PxSphereGeometry geometry{ radius * std::max<float>({scale.x, scale.y, scale.z}) };

	PhysicsMaterial* pm = GetPhysicsMaterial();
	if (pm == nullptr)
	{
		pm = PhysicsManager::GetInstance().GetDefaultPhysicsMaterial();
	}
	shape = physics->createShape(geometry, *pm->GetPxMaterial(), true);
	shape->userData = static_cast<void*>(this);
}

void SphereCollider::DetachShapeToActor()
{
	if (shape == nullptr)
	{
		//Debug::LogError("PxShape not found.");
		return;
	}

	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("PxActor not found.");
		return;
	}

	actor->GetActor()->detachShape(*shape);
}

void SphereCollider::SetIsTrigger(bool is_trigger)
{
	this->is_trigger = is_trigger;
	if (shape == nullptr)
	{
		//Debug::LogError("PxShape not found.");
		return;
	}

	shape->setFlag(px::PxShapeFlag::eSIMULATION_SHAPE, !is_trigger);
	shape->setFlag(px::PxShapeFlag::eTRIGGER_SHAPE, is_trigger);
}

void SphereCollider::Start()
{
	PhysicsManager::OnAddCollider(&GetGameObject(), this);
}
