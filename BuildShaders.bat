glslc --target-env=vulkan1.3 engine/shaders/Standard.vert -o 	engine/shaders/vert.spv
glslc --target-env=vulkan1.3 engine/shaders/Standard.frag -o 	engine/shaders/frag.spv
glslc --target-env=vulkan1.3 engine/shaders/Debug.vert -o 	 	engine/shaders/dbgVert.spv
glslc --target-env=vulkan1.3 engine/shaders/Debug.frag -o 	 	engine/shaders/dbgFrag.spv
glslc --target-env=vulkan1.3 engine/shaders/Shadow.vert -o 	engine/shaders/shadow.vert.spv
glslc --target-env=vulkan1.3 engine/shaders/Cubemap.vert -o 	engine/shaders/cubemap.vert.spv
glslc --target-env=vulkan1.3 engine/shaders/Cubemap.frag -o 	engine/shaders/cubemap.frag.spv
