#pragma once

struct CollisionInfo
{
	class GameObject* opponent{ nullptr };
	class Collider* my_collider{ nullptr };
	class Collider* opponents_collider{ nullptr };
};