�� ��ǻ�Ϳ��� vcpkg�� �� ��.
����� �� Ȯ���غ��� �� �Ǹ� �깫�� �����ϰ� ���� ��ο� �߰��� ��.

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

<< PhysX ���� ��� >>
1. Scene ���� -> PhysicsScene ����
2. ������Ʈ�� Rigidbody �Ǵ� Collider �߰� -> PhysicsActor �߰� / �ʿ��� ��� Shape ���� / �浹 ���͸� ���̾� �⺻��(0) �ʱ�ȭ
3. FixedUpdate ���� PxScene ������Ʈ �� ��ġ -> PhysicsActor�� ������ ������Ʈ�� Transform ����
�� Raycast, Sweep ���� ������ ���� PxScene ���¿� ���� ���� ������ �̷����.