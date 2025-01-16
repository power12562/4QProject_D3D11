#pragma once
#include <vector>
#include <directxtk/SimpleMath.h>
#include "Physics/PhysXFramework.h"
#include "Physics/PhysicsLayerManager/PhysicsLayerManager.h"
#include "Physics/Struct/RaycastResult.h"
#include "Physics/PhysicsScene/PhysicsScene.h"
#include "Physics/PhysicsMaterial/PhysicsMaterial.h"
#include "Core/TSingleton.h"

using namespace DirectX::SimpleMath;

class PhysicsManager : public TSingleton<PhysicsManager>
{
	using uint = unsigned int;

	static px::PxFilterFlags DefaultPhysicsFilterShader(px::PxFilterObjectAttributes attributes0, px::PxFilterData filterData0, px::PxFilterObjectAttributes attributes1, px::PxFilterData filterData1, px::PxPairFlags& pairFlags, const void* constantBlock, px::PxU32 constantBlockSize);



	px::PxDefaultAllocator px_allocator_callback;
	px::PxDefaultErrorCallback px_error_callback;

	px::PxDefaultCpuDispatcher* px_dispatcher{ nullptr };
	px::PxFoundation* px_foundation{ nullptr };
	px::PxTolerancesScale px_tolerance_scale;
	px::PxPhysics* px_physics{ nullptr };

	// GPU 가속 사용시 필요
	px::PxCudaContextManager* px_cuda_manager{ nullptr };

	px::PxPvd* px_pvd{ nullptr };


	// 물리 충돌 필터링 레이어
	PhysicsLayerManager* layer_manager{ nullptr };

	PhysicsScene* physics_scene{nullptr};
	static px::PxSceneDesc* default_scene_desc;
	static PhysicsMaterial* defaultPhysicsMaterial;
	const bool use_gpu_acceleration{ false };

public:
	PhysicsManager(const PhysicsManager&) = delete;
	PhysicsManager& operator=(const PhysicsManager&) = delete;
	PhysicsManager();
	~PhysicsManager();

	PhysicsScene* GetPhysicsScene() { return physics_scene;}
	static void ClearPhysicsScene();

	px::PxDefaultCpuDispatcher* GetCpuDispatcher();
	px::PxFoundation* GetFoundation();
	px::PxTolerancesScale* GetToleranceScale();
	px::PxPhysics* GetPhysics();
	px::PxPvd* GetPVD();
	void ConnectSceneWithPVD(px::PxScene* scene);

	PhysicsLayerManager& GetPhysicsLayerManager() { return *layer_manager; }
	PhysicsMaterial* GetDefaultPhysicsMaterial() { return defaultPhysicsMaterial; }


	// 충돌 및 이벤트가 발생할 레이어 쌍 지정
public:
	void SetPhysicsLayer(class GameObject* object, uint physics_layer_slot);
	void SetPhysicsLayer(class Collider* collider, uint physics_layer_slot);


	static void OnAddRigidbody(class GameObject* object, class Rigidbody* rb);
	static void OnAddCollider(class GameObject* object, class Collider* collider);
	static void OnChangeScale(class Transform* transform);


	// Queries
public:
	RaycastResult Raycast(Vector3 origin, Vector3 direction, float length, bool return_on_first_contact, uint MAX_HIT = 256);




	// temp
public:
	static px::PxVec3 ConvertToPxVec(Vector3 vec);
	static px::PxQuat ConvertToPxQuat(Quaternion quat);
	static Vector3 ConvertToVector3(px::PxVec3 vec);
	static Quaternion ConvertToQuaternion(px::PxQuat quat);
};
