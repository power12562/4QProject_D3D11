#pragma once
#include <numeric>
#include <initializer_list>

class PhysicsLayerManager
{
	static constexpr int SLOT_MAX{ 32 };
	
	unsigned int collision_bitflag[SLOT_MAX]{ INT_MAX, };
public:
	PhysicsLayerManager();
public:
	void SetCollision(unsigned int layer_slot, bool enable, std::initializer_list<unsigned int> target_layers);


public:
	unsigned int GetCollisionBitFlag(unsigned int layer_slot);
	bool IsValidSlot(unsigned int slot);
};