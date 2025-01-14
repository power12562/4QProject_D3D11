내 컴퓨터에서 vcpkg로 안 깔림.
인재원 컴 확인해보고 안 되면 깃무시 설정하고 내부 경로에 추가할 것.

0. External
v GameObject/Base/GameObject
	v OnAddRigidbody -> PhysicsManager::OnAddRigidbody
	v OnAddColliders -> PhysicsManager::OnAddCollider
	v GetCollider
v Scene/Base/Scene
v framework

1. Component
v Collider/Collider
v Collider/BoxCollider
v Collider/SphereCollider
v Collider/CapsuleCollider
v Rigidbody/Rigidbody

2. Physics
v PhysicsManager
v PhysicsLayerManager
v PhysicsActor
v PhysicsScene
v PhysicsSceneEventCallback
v enum/CollisionDetectionMode
v enum/ForceMode
v struct/CollisionInfo
v struct/RaycastResult
v PhysicsMaterial/PhysicsMaterial

<< PhysX 동작 방식 >>
1. PhysicsManager -> 단일 PhysicsScene 생성
2. 오브젝트에 Rigidbody 또는 Collider 추가 -> PhysicsActor 추가 / 필요한 경우 Shape 부착 / 충돌 필터링 레이어 기본값(0) 초기화
3. FixedUpdate 이후 PxScene 업데이트 후 패치 -> PhysicsActor가 부착된 오브젝트의 Transform 갱신
a. 씬 전환시 PhysicsManager의 PhysicsScene 제거
※ Raycast, Sweep 같은 쿼리는 현재 PxScene 상태에 대해 임의 시점에 이루어짐.