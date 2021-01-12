#include "Physics.h"
#include "PxPhysicsAPI.h"

physx::PxDefaultErrorCallback gDefaultErrorCallback;
physx::PxDefaultAllocator gDefaultAllocatorCallback;


void PhysSDK::Init() {
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    if (!mFoundation)
        printf("PxCreateFoundation failed!");

    bool recordMemoryAllocations = true;

    //mPvd = physx::PxCreatePvd(*mFoundation);
    //physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    //mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);


    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation,
        physx::PxTolerancesScale(), recordMemoryAllocations, /*mPvd*/nullptr);
    if (!mPhysics)
        printf("PxCreatePhysics failed!");
}

void PhysSDK::Shutdown() {
    mPhysics->release();
    mFoundation->release();
}