�� ��ǻ�Ϳ��� vcpkg�� �� ��.
����� �� Ȯ���غ��� �� �Ǹ� �깫�� �����ϰ� ���� ��ο� �߰��� ��.

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

<< PhysX ���� ��� >>
1. PhysicsManager -> ���� PhysicsScene ����
2. ������Ʈ�� Rigidbody �Ǵ� Collider �߰� -> PhysicsActor �߰� / �ʿ��� ��� Shape ���� / �浹 ���͸� ���̾� �⺻��(0) �ʱ�ȭ
3. FixedUpdate ���� PxScene ������Ʈ �� ��ġ -> PhysicsActor�� ������ ������Ʈ�� Transform ����
a. �� ��ȯ�� PhysicsManager�� PhysicsScene ����
�� Raycast, Sweep ���� ������ ���� PxScene ���¿� ���� ���� ������ �̷����.