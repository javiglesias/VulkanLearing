glslc --target-env=vulkan1.3 -std=450 resources/Shaders/Standard.vert -o resources/Shaders/vert.spv
glslc --target-env=vulkan1.3 -std=450 resources/Shaders/Standard.frag -o resources/Shaders/frag.spv
glslc --target-env=vulkan1.3 -std=450 resources/Shaders/Debug.vert -o 	 resources/Shaders/dbgVert.spv
glslc --target-env=vulkan1.3 -std=450 resources/Shaders/Debug.frag -o 	 resources/Shaders/dbgFrag.spv
glslc --target-env=vulkan1.3 -std=450 resources/Shaders/Shadow.vert -o 	 resources/Shaders/shadow.vert.spv
glslc --target-env=vulkan1.3 -std=450 resources/Shaders/Cubemap.vert -o 	 resources/Shaders/cubemap.vert.spv
glslc --target-env=vulkan1.3 -std=450 resources/Shaders/Cubemap.frag -o 	 resources/Shaders/cubemap.frag.spv