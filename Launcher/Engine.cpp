#include "Engine.h"

#include "common/timer.h"
#include "Physics.h"
#include "VulkanBackend.h"

void Engine::Initialize(const char * rootPath)
{
    m_rootPath = rootPath;
    m_physicsEngine = new PhysicsEngine;
    m_renderEngine = new VulkanBackend;

    m_physicsEngine->Init();
    m_renderEngine->Initialize(m_rootPath.c_str());


    {
        PhysicsObject *box2 = m_physicsEngine->CreateObject(0.f, -1.f, 0.f, true);
        float mx2[7];
        box2->GetMx(mx2);

        RenderObject *rBox2 = m_renderEngine->CreateObject(mx2);
        Object obj2;
        obj2.pObject = box2;
        obj2.rObject = rBox2;
        m_objects.push_back(obj2);
    }
    {
        PhysicsObject *box1 = m_physicsEngine->CreateObject(0.f, 1.f, 0.f, false);
        float mx1[7];
        box1->GetMx(mx1);

        RenderObject *rBox1 = m_renderEngine->CreateObject(mx1);
        Object obj1;
        obj1.pObject = box1;
        obj1.rObject = rBox1;
        m_objects.push_back(obj1);
    }
}

void Engine::Run(){
	cmn::timer _timer;
	int cycles = 0u;
	float elapsed = 0.f;

	do{
		cycles++;
		float delta = _timer.elapsed_time();
		elapsed+=delta;
		_timer.restart();

		if (elapsed > 5.0f){
			printf("FPS: %d for 5 secs\n", cycles/5);
			cycles = 0;
			elapsed = 0.f;
		}

		m_physicsEngine->Simulate(delta);

        for (auto &obj : m_objects){
            float mx[7];
            obj.pObject->GetMx(mx);
            obj.rObject->Update(mx);
        }

		m_renderEngine->Render(delta);
	}
    while(m_renderEngine->IsRunning());
}

void Engine::Shutdown(){
    m_physicsEngine->Shutdown();
    delete m_physicsEngine;
    m_physicsEngine = nullptr;
    delete m_renderEngine;
    m_renderEngine = nullptr;
}

	

	

	

	