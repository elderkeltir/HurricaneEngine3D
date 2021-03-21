#pragma once

#include <inttypes.h>

namespace iface{
    struct RenderCamera{
        void virtual Rotate(float x, float y) = 0;
        void virtual Move(int x, int y, int z) = 0;
    };
}