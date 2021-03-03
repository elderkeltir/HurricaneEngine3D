namespace iface{
    struct RenderPipelineCollection{
    public:
        enum PipelineType {
            PT_mesh,
            PT_skybox,
            PT_deffered,
            PT_shadowMap,
            PT_size
        };
    };
}