#pragma once
#include "Component/Base/Component.h"

class PhysicsTestController : public Component
{
public:
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}
};

