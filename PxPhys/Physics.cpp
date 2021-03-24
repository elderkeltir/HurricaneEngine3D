#include "Physics.h"

#include "PxPhysicsAPI.h"
#include "PxScene.h"
#include "PxRigidDynamic.h"

#include <cassert>

using namespace physx;

PxDefaultErrorCallback gDefaultErrorCallback;
PxDefaultAllocator gDefaultAllocatorCallback;

// TODO:

PxRigidDynamic * gBox;

void PhysSDK::Init() {
    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    if (!m_foundation)
        printf("PxCreateFoundation failed!");

    bool recordMemoryAllocations = true;

    //mPvd = PxCreatePvd(*m_foundation);
    //PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    //mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);


    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
        PxTolerancesScale(), recordMemoryAllocations, /*mPvd*/nullptr);
    if (!m_physics)
        printf("PxCreatePhysics failed!");

	// scene
    CreateScene();
    assert(m_scene);

	// Default material for now
    m_defaul_material = m_physics->createMaterial(0.5f, 0.5f, 0.1f);

	PxVec3 linVel = PxVec3(0.f, 10.f, 0.f);
	gBox = CreateBox(PxVec3(0.f, 0.f, 0.f), PxVec3(1.f, 1.f, 1.f), &linVel, 0.1, false);
	gBox->addForce(PxVec3(0.f, 10.f, 0.f));
}

void PhysSDK::Shutdown() {
    m_physics->release();
    m_foundation->release();
}

void PhysSDK::Simulate(float dt){
	PxSceneWriteLock scopedLock(*m_scene);

	m_scene->simulate(dt);
    m_scene->fetchResults(true);

	PxVec3 pose = gBox->getGlobalPose().p;
	printf("After Sim: (%f,%f,%f)\n", pose.x, pose.y, pose.z);
}

void PhysSDK::CreateScene(){
    PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
    sceneDesc.setToDefault(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

// #if PX_SUPPORT_GPU_PHYSX
// 	if(!sceneDesc.cudaContextManager)
// 		sceneDesc.cudaContextManager = mCudaContextManager;
// #endif

	//sceneDesc.frictionType = PxFrictionType::eTWO_DIRECTIONAL;
	//sceneDesc.frictionType = PxFrictionType::eONE_DIRECTIONAL;
	//sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
    sceneDesc.flags |= PxSceneFlag::eREQUIRE_RW_LOCK;
	//sceneDesc.flags |= PxSceneFlag::eENABLE_AVERAGE_POINT;
	sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
	//sceneDesc.flags |= PxSceneFlag::eADAPTIVE_FORCE;
	sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
	sceneDesc.sceneQueryUpdateMode = PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_DISABLED;

	//sceneDesc.flags |= PxSceneFlag::eDISABLE_CONTACT_CACHE;
	//sceneDesc.broadPhaseType =  PxBroadPhaseType::eGPU;
	//sceneDesc.broadPhaseType = PxBroadPhaseType::eSAP;
	sceneDesc.gpuMaxNumPartitions = 8;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	if(!sceneDesc.cpuDispatcher)
	{
		m_cpuDispatcher = PxDefaultCpuDispatcherCreate(1);// TODO: mNbThreads
		assert(m_cpuDispatcher);
		sceneDesc.cpuDispatcher	= m_cpuDispatcher;
	}

	//sceneDesc.solverType = PxSolverType::eTGS;

	//customizeSceneDesc(sceneDesc);

	m_scene = m_physics->createScene(sceneDesc);
    //assert(m_scene);
}

PxRigidDynamic* PhysSDK::CreateBox(const PxVec3& pos, const PxVec3& dims, const PxVec3* linVel, double density, bool kin)
{
	PxSceneWriteLock scopedLock(*m_scene);
	PxRigidDynamic* box = PxCreateDynamic(*m_physics, PxTransform(pos), PxBoxGeometry(dims), *m_defaul_material, density);
	PX_ASSERT(box);

	// SetupDefaultRigidDynamic(*box);
	box->setActorFlag(PxActorFlag::eVISUALIZATION, true);
	box->setAngularDamping(0.5f);
	box->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kin);
	m_scene->addActor(*box);

	if(linVel)
		box->setLinearVelocity(*linVel);

	return box;
}

///////////////////////////////////////////////////////////////////////////////

PxRigidDynamic* PhysSDK::CreateSphere(const PxVec3& pos, double radius, const PxVec3* linVel, double density, bool kin)
{
	PxSceneWriteLock scopedLock(*m_scene);
	PxRigidDynamic* sphere = PxCreateDynamic(*m_physics, PxTransform(pos), PxSphereGeometry(radius), *m_defaul_material, density);
	PX_ASSERT(sphere);

	// SetupDefaultRigidDynamic(*sphere);
	sphere->setActorFlag(PxActorFlag::eVISUALIZATION, true);
	sphere->setAngularDamping(0.5f);
	sphere->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kin);
	m_scene->addActor(*sphere);

	if(linVel)
		sphere->setLinearVelocity(*linVel);

	return sphere;
}