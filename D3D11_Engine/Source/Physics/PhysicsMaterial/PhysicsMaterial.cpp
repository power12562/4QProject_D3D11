#include "PhysicsMaterial.h"

PhysicsMaterial::PhysicsMaterial(px::PxPhysics* physics)
{
	material = physics->createMaterial(static_friction, dynamic_friction, bounciness);
	SetFrictionCombineMode(friction_combine_mode);
	SetBounceCombineMode(bounciness_combine_mode);
}

PhysicsMaterial::~PhysicsMaterial()
{
	if (material == nullptr) material->release();
}

void PhysicsMaterial::SetStaticFriction(float friction)
{
	this->static_friction = friction;
	material->setStaticFriction(friction);
}

void PhysicsMaterial::SetDynamicFriction(float friction)
{
	this->dynamic_friction = friction;
	material->setDynamicFriction(friction);
}

void PhysicsMaterial::SetBounciness(float bounciness)
{
	this->bounciness = bounciness;
	material->setRestitution(bounciness);
}

void PhysicsMaterial::SetFrictionCombineMode(PhysicsMaterialCombineMode mode)
{
	friction_combine_mode = mode;

	switch (mode)
	{
	case PhysicsMaterialCombineMode::Average:
		material->setFrictionCombineMode(px::PxCombineMode::eAVERAGE);
		break;
	case PhysicsMaterialCombineMode::Min:
		material->setFrictionCombineMode(px::PxCombineMode::eMIN);
		break;
	case PhysicsMaterialCombineMode::Multiply:
		material->setFrictionCombineMode(px::PxCombineMode::eMULTIPLY);
		break;
	case PhysicsMaterialCombineMode::Max:
		material->setFrictionCombineMode(px::PxCombineMode::eMAX);
		break;
	}
}

void PhysicsMaterial::SetBounceCombineMode(PhysicsMaterialCombineMode mode)
{
	friction_combine_mode = mode;

	switch (mode)
	{
	case PhysicsMaterialCombineMode::Average:
		material->setRestitutionCombineMode(px::PxCombineMode::eAVERAGE);
		break;
	case PhysicsMaterialCombineMode::Min:
		material->setRestitutionCombineMode(px::PxCombineMode::eMIN);
		break;
	case PhysicsMaterialCombineMode::Multiply:
		material->setRestitutionCombineMode(px::PxCombineMode::eMULTIPLY);
		break;
	case PhysicsMaterialCombineMode::Max:
		material->setRestitutionCombineMode(px::PxCombineMode::eMAX);
		break;
	}
}