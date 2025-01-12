#pragma once
#include <Component/Base/Component.h>

class Rigidbody : public Component
{

public:
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
};

