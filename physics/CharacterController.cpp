#include "CharacterController.h"

#include <characterkinematic/PxController.h>
#include <PxSceneLock.h>

using namespace physx;

void CharacterController::Initialize(PxController* cct, physx::PxScene * scene){
    m_cct = cct;
    r_scene = scene;
}

void CharacterController::Move(const PxVec3 &disp, float dt, bool jump){
    PxSceneWriteLock scopedLock(*r_scene);
    PxControllerFilters filter;
    m_cct->move(PxVec3(0.1f * dt, -0.1f, 0.0f),0.f, dt, filter);
}

physx::PxVec3 CharacterController::GetPosition() const{
    return toVec3(m_cct->getPosition());
}