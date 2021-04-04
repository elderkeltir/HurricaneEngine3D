#pragma once

#include <foundation/PxVec3.h>

namespace physx {
	class PxController;
    class PxScene;
}


class CharacterController{
public:
    void Initialize(physx::PxController* cct, physx::PxScene * scene);
    void Move(const physx::PxVec3 &disp, float dt, bool jump);
    physx::PxVec3 GetPosition() const;

private:
    physx::PxController * m_cct;
    physx::PxScene * r_scene;
    physx::PxVec3 m_lastPos;
    physx::PxVec3 m_velocity;
};