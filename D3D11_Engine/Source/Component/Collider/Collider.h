#pragma once
#include <Component/Base/Component.h>

class Collider : public Component
{


public:
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
};

