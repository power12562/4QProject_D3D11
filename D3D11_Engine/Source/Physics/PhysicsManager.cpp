#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsScene/PhysicsScene.h"
#include "GameObject/Base/GameObject.h"
#include "Component/Collider/Collider.h"
#include "Component/Collider/BoxCollider.h"
#include "Component/Collider/SphereCollider.h"
#include "Component/Collider/CapsuleCollider.h"


PhysicsMaterial* PhysicsManager::defaultPhysicsMaterial{ nullptr };
px::PxSceneDesc* PhysicsManager::default_scene_desc{ nullptr };

PhysicsManager::PhysicsManager()
{
	// create foundation
	px_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, px_allocator_callback, px_error_callback);
	if (!px_foundation) throw("PxCreateFoundation failed!");

	// (optional) connect with pvd
	// pvd ��ü�� PxPhysics�� �����Ǳ� ���� ���� �����Ǿ�� �Ѵ�.
#ifdef _DEBUG
	px_pvd = px::PxCreatePvd(*px_foundation);
	px::PxPvdTransport* transport = px::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10); // PVD�� 5425 ��Ʈ�� �����.
	px_pvd->connect(*transport, px::PxPvdInstrumentationFlag::eALL);
#endif

		// set tolerance scale
	px_tolerance_scale.length = 100;
	px_tolerance_scale.speed = 981;

	// create physics
	px_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *px_foundation, px_tolerance_scale, true, px_pvd);

	// initialize PhysX Extensions
	PxInitExtensions(*px_physics, px_pvd);

	// create cpu dispatcher ~ CPU ����� ��Ƽ������ ����
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	unsigned int logicalCoreNum = sysinfo.dwNumberOfProcessors;
	px_dispatcher = px::PxDefaultCpuDispatcherCreate(logicalCoreNum / 2);

	if (use_gpu_acceleration)
	{
		// GPU ������ ����ϸ� CPU�� ���� ������ ����� ���� �ٸ��� �Ҿ����ϴ�.
		// ������ �߿����� ���� ������ �� ���� ��������, ������ �߿��� ���ӿ��� �� �� ��.
		px::PxCudaContextManagerDesc px_cuda_desc;
		px_cuda_manager = PxCreateCudaContextManager(*px_foundation, px_cuda_desc, PxGetProfilerCallback());

		default_scene_desc = new px::PxSceneDesc(px_physics->getTolerancesScale());
		default_scene_desc->gravity = px::PxVec3(0.0f, -9.81f, 0.0f);
		default_scene_desc->cpuDispatcher = px_dispatcher;
		// default_scene_desc->filterShader = px::PxDefaultSimulationFilterShader; // �̰� �ݹ� ȣ�� �� ��
		default_scene_desc->filterShader = DefaultPhysicsFilterShader;
		default_scene_desc->cudaContextManager = px_cuda_manager;
		default_scene_desc->flags |= px::PxSceneFlag::eENABLE_GPU_DYNAMICS;
		default_scene_desc->broadPhaseType = px::PxBroadPhaseType::eGPU;
	}
	else
	{
		default_scene_desc = new px::PxSceneDesc(px_physics->getTolerancesScale());
		default_scene_desc->gravity = px::PxVec3(0.0f, -9.81f, 0.0f);
		default_scene_desc->cpuDispatcher = px_dispatcher;
		// default_scene_desc->filterShader = px::PxDefaultSimulationFilterShader; // �̰� �ݹ� ȣ�� �� ��
		default_scene_desc->filterShader = DefaultPhysicsFilterShader;
	}

	layer_manager = new PhysicsLayerManager();
	defaultPhysicsMaterial = new PhysicsMaterial(px_physics);
}

px::PxFilterFlags PhysicsManager::DefaultPhysicsFilterShader(px::PxFilterObjectAttributes attributes0, px::PxFilterData filterData0, px::PxFilterObjectAttributes attributes1, px::PxFilterData filterData1, px::PxPairFlags& pairFlags, const void* constantBlock, px::PxU32 constantBlockSize)
{
	pairFlags = px::PxPairFlag::eCONTACT_DEFAULT;

	pairFlags |= px::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	//pairFlags |= px::PxPairFlag::eNOTIFY_TOUCH_PERSISTS; // �浹 ���� ���´� ������ ���� �����ϰ� �� ������ ȣ���ϴ� �������
	pairFlags |= px::PxPairFlag::eNOTIFY_TOUCH_LOST;



	return px::PxFilterFlag::eDEFAULT;
}

PhysicsManager::~PhysicsManager()
{
	SafeDelete(&physics_scene);
	SafeDelete(&layer_manager);
	SafeDelete(&default_scene_desc);
	SafeDelete(&defaultPhysicsMaterial);

	// �۵��ϴ��� Ȯ�� �� ��
	if (px_physics) px_physics->release();
	if (px_dispatcher) px_dispatcher->release();
	if (px_pvd) px_pvd->release();
	if (px_foundation) px_foundation->release();
}

void PhysicsManager::ClearPhysicsScene()
{
	auto& pm = PhysicsManager::GetInstance();
	if (pm.physics_scene == nullptr)
	{
		pm.physics_scene = new PhysicsScene(default_scene_desc);
		return;
	}
	else
	{
		pm.physics_scene->ResetScene(default_scene_desc);
	}
}

px::PxDefaultCpuDispatcher* PhysicsManager::GetCpuDispatcher()
{
	return px_dispatcher;
}

px::PxFoundation* PhysicsManager::GetFoundation()
{
	return px_foundation;
}

px::PxTolerancesScale* PhysicsManager::GetToleranceScale()
{
	return &px_tolerance_scale;
}

px::PxPhysics* PhysicsManager::GetPhysics()
{
	return px_physics;
}

px::PxPvd* PhysicsManager::GetPVD()
{
	return px_pvd;
}

void PhysicsManager::ConnectSceneWithPVD(px::PxScene* scene)
{
	px::PxPvdSceneClient* px_pvd_client = scene->getScenePvdClient();
	if (px_pvd_client)
	{
		px_pvd_client->setScenePvdFlag(px::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		px_pvd_client->setScenePvdFlag(px::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		px_pvd_client->setScenePvdFlag(px::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
}








void PhysicsManager::SetPhysicsLayer(GameObject* object, uint physics_layer_slot)
{
	bool is_valid = layer_manager->IsValidSlot(physics_layer_slot);
	if (is_valid == false)
	{
		//Debug::LogError("Invalid physics slot.");
		return;
	}

	px::PxU32 my_layer = (1 << physics_layer_slot);
	px::PxU32 collision = layer_manager->GetCollisionBitFlag(physics_layer_slot);
	px::PxFilterData filter_data;
	filter_data.word0 = my_layer;
	filter_data.word1 = collision;

	std::vector<Collider*> colliders = object->GetEveryCollider();
	for (Collider* collider : colliders)
	{
		collider->shape->setSimulationFilterData(filter_data);
		collider->physics_layer_slot = physics_layer_slot;
	}

	object->phyiscsLayer = physics_layer_slot;
}
void PhysicsManager::SetPhysicsLayer(Collider* collider, uint physics_layer_slot)
{
	bool is_valid = layer_manager->IsValidSlot(physics_layer_slot);
	if (is_valid == false)
	{
		//Debug::LogError("Invalid physics slot.");
		return;
	}

	px::PxU32 my_layer = (1 << physics_layer_slot);
	px::PxU32 collision = layer_manager->GetCollisionBitFlag(physics_layer_slot);
	px::PxFilterData filter_data;
	filter_data.word0 = my_layer;
	filter_data.word1 = collision;

	collider->shape->setSimulationFilterData(filter_data);
	collider->physics_layer_slot = physics_layer_slot;
}

void PhysicsManager::OnAddRigidbody(GameObject* object, Rigidbody* rb)
{
	bool actor_created = false;
	auto& colliders = object->GetEveryCollider();

	if (object->physicsActor == nullptr)
	{
		object->physicsActor = new PhysicsActor(object, PhysicsManager::GetInstance().physics_scene->scene, object->transform.GetPosition(), object->transform.GetRotation(), PhysicsActorType::DynamicBody);
		actor_created = true;
	}
	else
	{
		if (object->physicsActor->GetActorType() == PhysicsActorType::StaticBody)
		{
			for (auto& collider : colliders)
			{
				collider->DetachShapeToActor();
			}

			delete object->physicsActor;
			object->physicsActor = new PhysicsActor(object, PhysicsManager::GetInstance().physics_scene->scene, object->transform.GetPosition(), object->transform.GetRotation(), PhysicsActorType::DynamicBody);
			actor_created = true;
		}
	}

	if (actor_created == true)
	{
		for (auto& collider : colliders)
		{
			collider->AttachShapeToActor();
		}
	}
}

void PhysicsManager::OnAddCollider(GameObject* object, Collider* collider)
{
	collider->SetSizeFollowingMesh();

	// ���Ͱ� ���� -> �ٸ� �ݶ��̴��� ������ٵ� ����.
	if (object->physicsActor == nullptr)
	{
		object->physicsActor = new PhysicsActor(object, PhysicsManager::GetInstance().physics_scene->scene, object->transform.GetPosition(), object->transform.GetRotation(), PhysicsActorType::StaticBody);

		collider->AttachShapeToActor();
		// �ٸ� �ݶ��̴��� �����Ǿ� �ִµ� PhysicsActor�� ���� ���� ����.
	}
	else
	{
		collider->AttachShapeToActor();
	}

	PhysicsManager::GetInstance().SetPhysicsLayer(collider, object->GetPhysicsLayer());
}

void PhysicsManager::OnChangeScale(Transform* transform)
{
	Vector3 scale = transform->GetScale();
	auto& colliders = transform->GetGameObject().GetEveryCollider();
	for (auto& c : colliders)
	{
		c->UpdateShape();
		c->AttachShapeToActor();
	}
}

RaycastResult PhysicsManager::Raycast(Vector3 origin, Vector3 direction, float length, bool return_on_first_contact, uint MAX_HIT)
{
	px::PxScene* scene = physics_scene->scene;

	direction.Normalize();
	px::PxVec3 _origin = ConvertToPxVec(origin);
	px::PxVec3 _direction = ConvertToPxVec(direction);

	px::PxHitFlags hit_flag;
	RaycastResult result;
	if (return_on_first_contact == false)
	{
		// ���� ���� �浹�� ���ÿ� �����ϴ� ��� 'touch'��� ����� ������ �浹 ������ ����.
		hit_flag.raise(px::PxHitFlag::eMESH_MULTIPLE);
		std::vector<px::PxRaycastHit> px_hits;
		px_hits.reserve(MAX_HIT);

		px::PxRaycastBuffer buffer(px_hits.data(), MAX_HIT);
		bool is_hit = scene->raycast(_origin, _direction, length, buffer, hit_flag);

		if (is_hit == false) return result;

		for (int i = 0; i < buffer.getNbTouches(); ++i)
		{
			px::PxRaycastHit px_hit = buffer.getTouch(i);
			RaycastHit hit;
			hit.collider = static_cast<Collider*>(px_hit.shape->userData);
			hit.object = static_cast<GameObject*>(px_hit.actor->userData);
			hit.position = Vector3{ px_hit.position.x, px_hit.position.y, px_hit.position.z };
			hit.distance = px_hit.distance;

			result.hits.push_back(hit);
		}

		return result;
	}
	else
	{
		// ���� �浹�� ���ؼ��� ó���ϴ� ���, ���� ����� �浹 ������ ���� 'block'�� ������ �ǹǷ� ������ PxRaycastBuffer�� ������ ������ �ʿ� ����.
		hit_flag.raise(px::PxHitFlag::eDEFAULT);
		px::PxRaycastBuffer buffer;

		bool is_hit = scene->raycast(_origin, _direction, length, buffer, hit_flag);

		if (is_hit == false) return result;

		px::PxRaycastHit px_hit = buffer.block;
		RaycastHit hit;
		hit.collider = static_cast<Collider*>(px_hit.shape->userData);
		hit.object = static_cast<GameObject*>(px_hit.actor->userData);
		hit.position = Vector3{ px_hit.position.x, px_hit.position.y, px_hit.position.z };
		hit.distance = px_hit.distance;

		result.hits.push_back(hit);

		return result;
	}
}

px::PxVec3 PhysicsManager::ConvertToPxVec(Vector3 vec)
{
	return *reinterpret_cast<px::PxVec3*>(&vec);
}

px::PxQuat PhysicsManager::ConvertToPxQuat(Quaternion quat)
{
	return *reinterpret_cast<px::PxQuat*>(&quat);
}

Vector3 PhysicsManager::ConvertToVector3(px::PxVec3 vec)
{
	return *reinterpret_cast<Vector3*>(&vec);
}

Quaternion PhysicsManager::ConvertToQuaternion(px::PxQuat quat)
{
	return *reinterpret_cast<Quaternion*>(&quat);
}
