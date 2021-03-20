#pragma once

namespace iface{
    struct RenderPipelineCollection{
    public:
        enum PipelineType {
            PT_mesh = 0,
            PT_primitive,
            PT_skybox,
            PT_deffered,
            PT_shadowMap,
            PT_size
        };
    };
}