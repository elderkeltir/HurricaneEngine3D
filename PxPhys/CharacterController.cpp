#include "CharacterController.h"

#include <characterkinematic/PxController.h>

using namespace physx;

void CharacterController::Initialize(PxController* cct){
    m_cct = cct;
}

void CharacterController::Move(const PxVec3 &disp, float dt, bool jump){
    PxControllerFilters filter;
    m_cct->move(PxVec3(0.1f, 0.1f, 0.1f),0.f, dt, filter);
}

physx::PxVec3 CharacterController::GetPosition() const{
    return toVec3(m_cct->getPosition());
}