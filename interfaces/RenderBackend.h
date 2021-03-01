namespace iface{
    struct RenderBackend{
        RenderBackend() = default;
        virtual void Initialize() = 0;
        virtual void Render() = 0;

    };
}