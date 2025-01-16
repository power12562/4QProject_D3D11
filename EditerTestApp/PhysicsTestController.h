#pragma once
#include <vector>
#include "Component/Base/Component.h"
#include "framework.h"

class PhysicsTestController : public Component
{
	bool press_p{ false };
	AudioClip* sm{ nullptr };
	int music_idx = 0;
	std::vector<Sound*> sounds;

public:
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}
};

