#ifndef GPU_PARTICLE_H
#define GPU_PARTICLE_H

#include "Types.h"

namespace VKR
{
    namespace render
    {
        namespace GPU
        {
            struct Particle
            {
                glm::vec4 pos_vel;
                glm::vec4 color;
            };
        }
    }
}

#endif