#include <vector>
#include <string>

#include "interfaces/RenderBackend.h"


class RenderObject;
class PhysicsObject;
class PhysicsEngine;

class Engine {
public:
    void Initialize(const char * rootPath);
    void Run();
    void Shutdown();
private:
    struct Object{
        RenderObject *rObject;
        PhysicsObject *pObject;
        void Release(PhysicsEngine * physEng);
    };
    std::vector<Engine::Object> m_objects;
    std::string m_rootPath;

    PhysicsEngine *m_physicsEngine;
    iface::RenderBackend *m_renderEngine;
};