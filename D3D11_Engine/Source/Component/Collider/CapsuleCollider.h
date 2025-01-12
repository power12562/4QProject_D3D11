#pragma once
#include <Component/Base/Component.h>
#include <Component/Collider/Collider.h>

class CapsuleCollider : public Collider
{


public:
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
};

