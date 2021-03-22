#include "Physics.h"
#include "PxPhysicsAPI.h"

physx::PxDefaultErrorCallback gDefaultErrorCallback;
physx::PxDefaultAllocator gDefaultAllocatorCallback;


void PhysSDK::Init() {
    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    if (!m_foundation)
        printf("PxCreateFoundation failed!");

    bool recordMemoryAllocations = true;

    //mPvd = physx::PxCreatePvd(*m_foundation);
    //physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    //mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);


    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
        physx::PxTolerancesScale(), recordMemoryAllocations, /*mPvd*/nullptr);
    if (!m_physics)
        printf("PxCreatePhysics failed!");
}

void PhysSDK::Shutdown() {
    m_physics->release();
    m_foundation->release();
}

void PhysSDK::CreateScene(){
    m_scene = 
}