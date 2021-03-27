#pragma once

class RenderObject; // TODO

namespace iface{
    struct RenderBackend{
        RenderBackend() = default;
        virtual void Initialize(const char * rootFolder) = 0;
        virtual void Render(float dt) = 0;
        virtual bool IsRunning() = 0;
        virtual ~RenderBackend() {};

        // TODO: forgive me lord
        virtual RenderObject * CreateObject(float* mx, bool texturedMesh) = 0;
    };
}