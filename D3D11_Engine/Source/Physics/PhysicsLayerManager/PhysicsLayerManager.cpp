#include "Physics/PhysicsLayerManager/PhysicsLayerManager.h"

PhysicsLayerManager::PhysicsLayerManager()
{
}

void PhysicsLayerManager::SetCollision(unsigned int layer_slot, bool enable, std::initializer_list<unsigned int> target_layers)
{
	if (layer_slot == -1 || layer_slot >= SLOT_MAX)
	{
		//Debug::LogError("Physics layer slot num is 32.");
		return;
	}

	if (enable == true)
	{
		for (unsigned int target : target_layers)
		{
			if (target >= SLOT_MAX)
			{
				//Debug::LogError("Physics layer slot num is 32.");
				continue;
			}
			collision_bitflag[layer_slot] |= (1 << target);
		}
	}
	else if (enable == false)
	{
		for (unsigned int target : target_layers)
		{
			if (target >= SLOT_MAX)
			{
				//Debug::LogError("Physics layer slot num is 32.");
				continue;
			}
			collision_bitflag[layer_slot] &= ~(1 << target);
		}
	}
}






unsigned int PhysicsLayerManager::GetCollisionBitFlag(unsigned int layer_slot)
{
	if (layer_slot >= SLOT_MAX)
	{
		//Debug::LogError("Physics layer slot num is 32.");
		return 0;
	}

	return collision_bitflag[layer_slot];
}

bool PhysicsLayerManager::IsValidSlot(unsigned int slot)
{
	if (slot < SLOT_MAX) return true;
	return false;
}