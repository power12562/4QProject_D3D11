내 컴퓨터에서 vcpkg로 안 깔림.
인재원 컴 확인해보고 안 되면 깃무시 설정하고 내부 경로에 추가할 것.

1. Component
Collider/Collider
Collider/BoxCollider
Collider/SphereCollider
Collider/CapsuleCollider
Rigidbody/Rigidbody

2. Physics
PhysicsManager
PhysicsActor
PhysicsScene
enum/CollisionDetectionMode
enum/ForceMode
struct/CollisionInfo
struct/RaycastResult
PhysicsMaterial/PhysicsMaterial

<< PhysX 동작 방식 >>
1. Scene 생성 -> PhysicsScene 생성
2. 오브젝트에 Rigidbody 또는 Collider 추가 -> PhysicsActor 추가 / 필요한 경우 Shape 부착 / 충돌 필터링 레이어 기본값(0) 초기화
3. FixedUpdate 이후 PxScene 업데이트 후 패치 -> PhysicsActor가 부착된 오브젝트의 Transform 갱신
※ Raycast, Sweep 같은 쿼리는 현재 PxScene 상태에 대해 임의 시점에 이루어짐.