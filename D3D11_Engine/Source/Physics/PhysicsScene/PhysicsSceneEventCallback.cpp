#include "Physics/PhysicsScene/PhysicsSceneEventCallback.h"
#include "Physics/Struct/CollisionInfo.h"
#include "GameObject/Base/GameObject.h"




void PhysicsSceneEventCallback::onContact(const px::PxContactPairHeader& pairHeader, const px::PxContactPair* pairs, px::PxU32 pairCount)
{
    GameObject* obj[2];
    obj[0] = static_cast<GameObject*>(pairHeader.actors[0]->userData);
    obj[1] = static_cast<GameObject*>(pairHeader.actors[1]->userData);
    Collider* collider[2];

    for (px::PxU32 i = 0; i < pairCount; ++i)
    {
        const px::PxContactPair& pair = pairs[i];

        collider[0] = static_cast<Collider*>(pair.shapes[0]->userData);
        collider[1] = static_cast<Collider*>(pair.shapes[1]->userData);

        bool is_shapes_aligned{ true };
        if (pair.shapes[0]->getActor() == pairHeader.actors[1])
        {
            std::swap(collider[0], collider[1]);
            is_shapes_aligned = false;
        }

        CollisionInfo info[2];
        info[0].opponent = obj[1];
        info[0].my_collider = collider[0];
        info[0].opponents_collider = collider[1];

        info[1].opponent = obj[0];
        info[1].my_collider = collider[1];
        info[1].opponents_collider = collider[0];

        if (pair.events & px::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            obj[0]->OnCollisionEnter(info[0]);
            obj[1]->OnCollisionEnter(info[1]);
        }
        else if (pair.events & px::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
        {
            obj[0]->OnCollisionStay(info[0]);
            obj[1]->OnCollisionStay(info[1]);
        }
        else if (pair.events & px::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            // 오브젝트는 씬 루프의 마지막에 제거되고, 이 때 PxActor와 PxShape도 함께 제거되므로 충돌, 충돌 유지 이벤트에서는 문제가 생길 수 없지만,
            // 충돌 정지 이벤트는 PhysX가 어떻게 동작하느냐에 따라 삭제된 액터에 대해서도 발생할 수 있으므로 이를 처리해줘야 한다.

            bool is_removed[2]{ false, false };

            if (pair.flags & px::PxContactPairFlag::eREMOVED_SHAPE_0)
            {
                if (is_shapes_aligned)
                {
                    is_removed[0] = true;
                }
                else
                {
                    is_removed[1] = true;
                }
            }
            if (pair.flags & px::PxContactPairFlag::eREMOVED_SHAPE_1)
            {
                if (is_shapes_aligned)
                {
                    is_removed[1] = true;
                }
                else
                {
                    is_removed[0] = true;
                }
            }

            for (int i = 0; i < 2; ++i)
            {
                if (is_removed[i])
                {
                    int other = i ^ 1;
                    info[other].opponent = nullptr;
                    info[other].opponents_collider = nullptr;
                }
            }

            if (is_removed[0] == false) obj[0]->OnCollisionExit(info[0]);
            if (is_removed[1] == false) obj[1]->OnCollisionExit(info[1]);
        }
    }
}

void PhysicsSceneEventCallback::onTrigger(px::PxTriggerPair* pairs, px::PxU32 count)
{
    for (px::PxU32 i = 0; i < count; ++i)
    {
        const px::PxTriggerPair& pair = pairs[i];

        CollisionInfo info;
        info.opponent = static_cast<GameObject*>(pair.otherActor->userData);
        info.my_collider = static_cast<Collider*>(pair.triggerShape->userData);
        info.opponents_collider = static_cast<Collider*>(pair.otherShape->userData);

        GameObject* triggered_obj = static_cast<GameObject*>(pair.triggerActor->userData);

        if (pair.status == px::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            triggered_obj->OnTriggerEnter(info);
        }
        else if (pair.status == px::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
        {
            triggered_obj->OnTriggerStay(info);
        }
        else if (pair.status == px::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            // 오브젝트는 씬 루프의 마지막에 제거되고, 이 때 PxActor와 PxShape도 함께 제거되므로 충돌, 충돌 유지 이벤트에서는 문제가 생길 수 없지만,
            // 충돌 정지 이벤트는 PhysX가 어떻게 동작하느냐에 따라 삭제된 액터에 대해서도 발생할 수 있으므로 이를 처리해줘야 한다.

            if (pair.flags & px::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER)
            {
                return;
            }
            if (pair.flags & px::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)
            {
                info.opponent = nullptr;
                info.opponents_collider = nullptr;
            }
            triggered_obj->OnTriggerExit(info);
        }
    }
}