#include "Component/Collider/BoxCollider.h"
#include "Math/Mathf.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"

BoxCollider::~BoxCollider()
{
	DetachShapeToActor();
}

void BoxCollider::SetSize(float width, float height, float depth)
{
	this->width = width;
	this->height = height;
	this->depth = depth;
	UpdateShape();
	AttachShapeToActor();
}

void BoxCollider::SetSizeFollowingMesh()
{
	BoundingBox bb = GetGameObject().GetBBToWorld();

	this->width = bb.Extents.x * 2.0f;
	this->height = bb.Extents.y * 2.0f;
	this->depth = bb.Extents.z * 2.0f;

	UpdateShape();
}

void BoxCollider::DrawBounds(Color color)
{
	/*BoundingOrientedBox bob;
	bob.Center = { 0, 0, 0 };
	bob.Extents.x = width * 0.5f;
	bob.Extents.y = height * 0.5f;
	bob.Extents.z = depth * 0.5f;
	bob.Transform(bob, GetOwner()->transform->GetWorldTransformMatrix());

	Gizmo::DrawBounds(bob, color);*/
}

void BoxCollider::AttachShapeToActor()
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

void BoxCollider::UpdateShape()
{
	if (shape != nullptr)
	{
		DetachShapeToActor();
		shape->release();
	}

	px::PxPhysics* physics = PhysicsManager::GetInstance().GetPhysics();

	Vector3 real_half_size{ width * 0.5f, height * 0.5f, depth * 0.5f };
	Vector3 scale = GetGameObject().transform.GetScale();
	real_half_size *= GetGameObject().transform.GetScale();

	px::PxBoxGeometry geometry{ PhysicsManager::ConvertToPxVec(real_half_size) };
	PhysicsMaterial* pm = GetPhysicsMaterial();
	if (pm == nullptr)
	{
		pm = PhysicsManager::GetInstance().GetDefaultPhysicsMaterial();
	}
	shape = physics->createShape(geometry, *pm->GetPxMaterial(), true);

	// geometry에 0 담기면 shape 생성이 아예 무시되던데?
	if (shape == nullptr)
	{
		//Debug::LogError("Shape not found.");
		return;
	}
	shape->userData = static_cast<void*>(this);
}

void BoxCollider::DetachShapeToActor()
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

void BoxCollider::SetIsTrigger(bool is_trigger)
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

void BoxCollider::Start()
{
	PhysicsManager::OnAddCollider(&GetGameObject(), this);
}
