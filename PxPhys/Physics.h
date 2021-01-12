#pragma once
#define WIN64

#ifdef PXPHYS_EXPORTS
#define PXPHYS_EXPORTS __declspec(dllexport)
#else
#define PXPHYS_EXPORTS __declspec(dllimport)
#endif

namespace physx {
	class PxFoundation;
	class PxPhysics;
}

class PhysSDK {

public:
	void PXPHYS_EXPORTS Init();
	void PXPHYS_EXPORTS Shutdown();

private:
	physx::PxFoundation* mFoundation;
	//physx::PxPvd* mPvd;
	physx::PxPhysics* mPhysics;
};