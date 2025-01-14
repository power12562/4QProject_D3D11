#pragma once
#include "Physics/PhysXFramework.h"

class PhysicsSceneEventCallback : public px::PxSimulationEventCallback
{
public:
    void onContact(const px::PxContactPairHeader& pairHeader, const px::PxContactPair* pairs, px::PxU32 pairCount) override;

    void onTrigger(px::PxTriggerPair* pairs, px::PxU32 count) override;

    void onConstraintBreak(px::PxConstraintInfo* constraints, px::PxU32 count) override {}

    void onWake(px::PxActor** actors, px::PxU32 count) override {}

    void onSleep(px::PxActor** actors, px::PxU32 count) override {}

    void onAdvance(const px::PxRigidBody* const* bodyBuffer, const px::PxTransform* poseBuffer, const px::PxU32 count) override {}
};
