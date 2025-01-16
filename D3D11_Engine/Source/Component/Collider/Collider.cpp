#include "Component/Collider/Collider.h"


Collider::Collider()
{

}

Collider::~Collider()
{

}

void Collider::SetPhysicsMateiral(PhysicsMaterial* material)
{
	this->physics_material = material;

	UpdateShape();
	AttachShapeToActor();
}