#pragma once
#include "Physics/PhysXFramework.h"
#include "Physics/PhysicsScene/PhysicsSceneEventCallback.h"


class PhysicsScene
{
	friend class PhysicsManager;

	static PhysicsSceneEventCallback physics_scene_event_callback;

	px::PxScene* scene{ nullptr };
public:
	PhysicsScene(px::PxSceneDesc* scene_desc);
	PhysicsScene(const PhysicsScene&) = delete;
	PhysicsScene(PhysicsScene&&) = delete;
	~PhysicsScene();

	void ResetScene(px::PxSceneDesc* scene_desc);
	px::PxScene* GetScene() { return scene; }

	void Update(float delta_time);
};