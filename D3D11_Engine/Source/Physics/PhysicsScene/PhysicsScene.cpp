#include "Physics/PhysicsScene/PhysicsScene.h"
#include "Scene/Base/Scene.h"
#include "Physics/PhysicsManager.h"

PhysicsSceneEventCallback PhysicsScene::physics_scene_event_callback;

PhysicsScene::PhysicsScene(px::PxSceneDesc* scene_desc)
{
	auto* pm = PhysicsManager::GetInstance().GetPhysics();

	scene = pm->createScene(*scene_desc);
	scene->setSimulationEventCallback(&physics_scene_event_callback);

#ifdef _DEBUG
	PhysicsManager::GetInstance().ConnectSceneWithPVD(scene);
#endif
}

PhysicsScene::~PhysicsScene()
{
	if (scene != nullptr) scene->release();
}

void PhysicsScene::ResetScene(px::PxSceneDesc* scene_desc)
{
	if (scene != nullptr) scene->release();

	auto * pm = PhysicsManager::GetInstance().GetPhysics();

	scene = pm->createScene(*scene_desc);
	scene->setSimulationEventCallback(&physics_scene_event_callback);

#ifdef _DEBUG
	PhysicsManager::GetInstance().ConnectSceneWithPVD(scene);
#endif
}

void PhysicsScene::Update(float delta_time)
{
	scene->simulate(delta_time);
	scene->fetchResults(true);
}