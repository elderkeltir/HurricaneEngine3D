#pragma once

#include <foundation/PxVec3.h>

namespace physx {
	class PxController;
}


class CharacterController{
public:
    void Initialize(physx::PxController* cct);
    void Move(const physx::PxVec3 &disp, float dt, bool jump);
    physx::PxVec3 GetPosition() const;

private:
    physx::PxController * m_cct;
    physx::PxVec3 m_lastPos;
    physx::PxVec3 m_velocity;
};