#pragma once
#include "Physics/PhysXFramework.h"
#include "PhysicsMaterialCombineMode.h"

class PhysicsMaterial
{
	float static_friction{ 0.6f };
	float dynamic_friction{ 0.6f };
	float bounciness{ 1.0f };

	PhysicsMaterialCombineMode friction_combine_mode{ PhysicsMaterialCombineMode::Average };
	PhysicsMaterialCombineMode bounciness_combine_mode{ PhysicsMaterialCombineMode::Average };

	px::PxMaterial* material{ nullptr };



public:
	PhysicsMaterial(px::PxPhysics* physics);
	~PhysicsMaterial();
	void SetStaticFriction(float friction);
	void SetDynamicFriction(float friction);
	void SetBounciness(float bounciness);
	void SetFrictionCombineMode(PhysicsMaterialCombineMode mode);
	void SetBounceCombineMode(PhysicsMaterialCombineMode mode);

	px::PxMaterial* GetPxMaterial() { return material; }
};