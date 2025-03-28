# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/unframed/Documents/dev/VulkanLearing

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/unframed/Documents/dev/VulkanLearing

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/unframed/Documents/dev/VulkanLearing/CMakeFiles /home/unframed/Documents/dev/VulkanLearing//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/unframed/Documents/dev/VulkanLearing/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named VKRenderer

# Build rule for target.
VKRenderer: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 VKRenderer
.PHONY : VKRenderer

# fast build rule for target.
VKRenderer/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/build
.PHONY : VKRenderer/fast

dependencies/imgui/backends/imgui_impl_glfw.o: dependencies/imgui/backends/imgui_impl_glfw.cpp.o
.PHONY : dependencies/imgui/backends/imgui_impl_glfw.o

# target to build an object file
dependencies/imgui/backends/imgui_impl_glfw.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/backends/imgui_impl_glfw.cpp.o
.PHONY : dependencies/imgui/backends/imgui_impl_glfw.cpp.o

dependencies/imgui/backends/imgui_impl_glfw.i: dependencies/imgui/backends/imgui_impl_glfw.cpp.i
.PHONY : dependencies/imgui/backends/imgui_impl_glfw.i

# target to preprocess a source file
dependencies/imgui/backends/imgui_impl_glfw.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/backends/imgui_impl_glfw.cpp.i
.PHONY : dependencies/imgui/backends/imgui_impl_glfw.cpp.i

dependencies/imgui/backends/imgui_impl_glfw.s: dependencies/imgui/backends/imgui_impl_glfw.cpp.s
.PHONY : dependencies/imgui/backends/imgui_impl_glfw.s

# target to generate assembly for a file
dependencies/imgui/backends/imgui_impl_glfw.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/backends/imgui_impl_glfw.cpp.s
.PHONY : dependencies/imgui/backends/imgui_impl_glfw.cpp.s

dependencies/imgui/backends/imgui_impl_vulkan.o: dependencies/imgui/backends/imgui_impl_vulkan.cpp.o
.PHONY : dependencies/imgui/backends/imgui_impl_vulkan.o

# target to build an object file
dependencies/imgui/backends/imgui_impl_vulkan.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/backends/imgui_impl_vulkan.cpp.o
.PHONY : dependencies/imgui/backends/imgui_impl_vulkan.cpp.o

dependencies/imgui/backends/imgui_impl_vulkan.i: dependencies/imgui/backends/imgui_impl_vulkan.cpp.i
.PHONY : dependencies/imgui/backends/imgui_impl_vulkan.i

# target to preprocess a source file
dependencies/imgui/backends/imgui_impl_vulkan.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/backends/imgui_impl_vulkan.cpp.i
.PHONY : dependencies/imgui/backends/imgui_impl_vulkan.cpp.i

dependencies/imgui/backends/imgui_impl_vulkan.s: dependencies/imgui/backends/imgui_impl_vulkan.cpp.s
.PHONY : dependencies/imgui/backends/imgui_impl_vulkan.s

# target to generate assembly for a file
dependencies/imgui/backends/imgui_impl_vulkan.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/backends/imgui_impl_vulkan.cpp.s
.PHONY : dependencies/imgui/backends/imgui_impl_vulkan.cpp.s

dependencies/imgui/imgui.o: dependencies/imgui/imgui.cpp.o
.PHONY : dependencies/imgui/imgui.o

# target to build an object file
dependencies/imgui/imgui.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui.cpp.o
.PHONY : dependencies/imgui/imgui.cpp.o

dependencies/imgui/imgui.i: dependencies/imgui/imgui.cpp.i
.PHONY : dependencies/imgui/imgui.i

# target to preprocess a source file
dependencies/imgui/imgui.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui.cpp.i
.PHONY : dependencies/imgui/imgui.cpp.i

dependencies/imgui/imgui.s: dependencies/imgui/imgui.cpp.s
.PHONY : dependencies/imgui/imgui.s

# target to generate assembly for a file
dependencies/imgui/imgui.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui.cpp.s
.PHONY : dependencies/imgui/imgui.cpp.s

dependencies/imgui/imgui_draw.o: dependencies/imgui/imgui_draw.cpp.o
.PHONY : dependencies/imgui/imgui_draw.o

# target to build an object file
dependencies/imgui/imgui_draw.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_draw.cpp.o
.PHONY : dependencies/imgui/imgui_draw.cpp.o

dependencies/imgui/imgui_draw.i: dependencies/imgui/imgui_draw.cpp.i
.PHONY : dependencies/imgui/imgui_draw.i

# target to preprocess a source file
dependencies/imgui/imgui_draw.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_draw.cpp.i
.PHONY : dependencies/imgui/imgui_draw.cpp.i

dependencies/imgui/imgui_draw.s: dependencies/imgui/imgui_draw.cpp.s
.PHONY : dependencies/imgui/imgui_draw.s

# target to generate assembly for a file
dependencies/imgui/imgui_draw.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_draw.cpp.s
.PHONY : dependencies/imgui/imgui_draw.cpp.s

dependencies/imgui/imgui_tables.o: dependencies/imgui/imgui_tables.cpp.o
.PHONY : dependencies/imgui/imgui_tables.o

# target to build an object file
dependencies/imgui/imgui_tables.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_tables.cpp.o
.PHONY : dependencies/imgui/imgui_tables.cpp.o

dependencies/imgui/imgui_tables.i: dependencies/imgui/imgui_tables.cpp.i
.PHONY : dependencies/imgui/imgui_tables.i

# target to preprocess a source file
dependencies/imgui/imgui_tables.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_tables.cpp.i
.PHONY : dependencies/imgui/imgui_tables.cpp.i

dependencies/imgui/imgui_tables.s: dependencies/imgui/imgui_tables.cpp.s
.PHONY : dependencies/imgui/imgui_tables.s

# target to generate assembly for a file
dependencies/imgui/imgui_tables.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_tables.cpp.s
.PHONY : dependencies/imgui/imgui_tables.cpp.s

dependencies/imgui/imgui_widgets.o: dependencies/imgui/imgui_widgets.cpp.o
.PHONY : dependencies/imgui/imgui_widgets.o

# target to build an object file
dependencies/imgui/imgui_widgets.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_widgets.cpp.o
.PHONY : dependencies/imgui/imgui_widgets.cpp.o

dependencies/imgui/imgui_widgets.i: dependencies/imgui/imgui_widgets.cpp.i
.PHONY : dependencies/imgui/imgui_widgets.i

# target to preprocess a source file
dependencies/imgui/imgui_widgets.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_widgets.cpp.i
.PHONY : dependencies/imgui/imgui_widgets.cpp.i

dependencies/imgui/imgui_widgets.s: dependencies/imgui/imgui_widgets.cpp.s
.PHONY : dependencies/imgui/imgui_widgets.s

# target to generate assembly for a file
dependencies/imgui/imgui_widgets.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/dependencies/imgui/imgui_widgets.cpp.s
.PHONY : dependencies/imgui/imgui_widgets.cpp.s

engine/core/Materials/VKRCubemapMaterial.o: engine/core/Materials/VKRCubemapMaterial.cpp.o
.PHONY : engine/core/Materials/VKRCubemapMaterial.o

# target to build an object file
engine/core/Materials/VKRCubemapMaterial.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRCubemapMaterial.cpp.o
.PHONY : engine/core/Materials/VKRCubemapMaterial.cpp.o

engine/core/Materials/VKRCubemapMaterial.i: engine/core/Materials/VKRCubemapMaterial.cpp.i
.PHONY : engine/core/Materials/VKRCubemapMaterial.i

# target to preprocess a source file
engine/core/Materials/VKRCubemapMaterial.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRCubemapMaterial.cpp.i
.PHONY : engine/core/Materials/VKRCubemapMaterial.cpp.i

engine/core/Materials/VKRCubemapMaterial.s: engine/core/Materials/VKRCubemapMaterial.cpp.s
.PHONY : engine/core/Materials/VKRCubemapMaterial.s

# target to generate assembly for a file
engine/core/Materials/VKRCubemapMaterial.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRCubemapMaterial.cpp.s
.PHONY : engine/core/Materials/VKRCubemapMaterial.cpp.s

engine/core/Materials/VKRDebugMaterial.o: engine/core/Materials/VKRDebugMaterial.cpp.o
.PHONY : engine/core/Materials/VKRDebugMaterial.o

# target to build an object file
engine/core/Materials/VKRDebugMaterial.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRDebugMaterial.cpp.o
.PHONY : engine/core/Materials/VKRDebugMaterial.cpp.o

engine/core/Materials/VKRDebugMaterial.i: engine/core/Materials/VKRDebugMaterial.cpp.i
.PHONY : engine/core/Materials/VKRDebugMaterial.i

# target to preprocess a source file
engine/core/Materials/VKRDebugMaterial.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRDebugMaterial.cpp.i
.PHONY : engine/core/Materials/VKRDebugMaterial.cpp.i

engine/core/Materials/VKRDebugMaterial.s: engine/core/Materials/VKRDebugMaterial.cpp.s
.PHONY : engine/core/Materials/VKRDebugMaterial.s

# target to generate assembly for a file
engine/core/Materials/VKRDebugMaterial.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRDebugMaterial.cpp.s
.PHONY : engine/core/Materials/VKRDebugMaterial.cpp.s

engine/core/Materials/VKRMaterial.o: engine/core/Materials/VKRMaterial.cpp.o
.PHONY : engine/core/Materials/VKRMaterial.o

# target to build an object file
engine/core/Materials/VKRMaterial.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRMaterial.cpp.o
.PHONY : engine/core/Materials/VKRMaterial.cpp.o

engine/core/Materials/VKRMaterial.i: engine/core/Materials/VKRMaterial.cpp.i
.PHONY : engine/core/Materials/VKRMaterial.i

# target to preprocess a source file
engine/core/Materials/VKRMaterial.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRMaterial.cpp.i
.PHONY : engine/core/Materials/VKRMaterial.cpp.i

engine/core/Materials/VKRMaterial.s: engine/core/Materials/VKRMaterial.cpp.s
.PHONY : engine/core/Materials/VKRMaterial.s

# target to generate assembly for a file
engine/core/Materials/VKRMaterial.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRMaterial.cpp.s
.PHONY : engine/core/Materials/VKRMaterial.cpp.s

engine/core/Materials/VKRShader.o: engine/core/Materials/VKRShader.cpp.o
.PHONY : engine/core/Materials/VKRShader.o

# target to build an object file
engine/core/Materials/VKRShader.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRShader.cpp.o
.PHONY : engine/core/Materials/VKRShader.cpp.o

engine/core/Materials/VKRShader.i: engine/core/Materials/VKRShader.cpp.i
.PHONY : engine/core/Materials/VKRShader.i

# target to preprocess a source file
engine/core/Materials/VKRShader.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRShader.cpp.i
.PHONY : engine/core/Materials/VKRShader.cpp.i

engine/core/Materials/VKRShader.s: engine/core/Materials/VKRShader.cpp.s
.PHONY : engine/core/Materials/VKRShader.s

# target to generate assembly for a file
engine/core/Materials/VKRShader.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRShader.cpp.s
.PHONY : engine/core/Materials/VKRShader.cpp.s

engine/core/Materials/VKRTexture.o: engine/core/Materials/VKRTexture.cpp.o
.PHONY : engine/core/Materials/VKRTexture.o

# target to build an object file
engine/core/Materials/VKRTexture.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRTexture.cpp.o
.PHONY : engine/core/Materials/VKRTexture.cpp.o

engine/core/Materials/VKRTexture.i: engine/core/Materials/VKRTexture.cpp.i
.PHONY : engine/core/Materials/VKRTexture.i

# target to preprocess a source file
engine/core/Materials/VKRTexture.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRTexture.cpp.i
.PHONY : engine/core/Materials/VKRTexture.cpp.i

engine/core/Materials/VKRTexture.s: engine/core/Materials/VKRTexture.cpp.s
.PHONY : engine/core/Materials/VKRTexture.s

# target to generate assembly for a file
engine/core/Materials/VKRTexture.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Materials/VKRTexture.cpp.s
.PHONY : engine/core/Materials/VKRTexture.cpp.s

engine/core/Objects/VKRCubemap.o: engine/core/Objects/VKRCubemap.cpp.o
.PHONY : engine/core/Objects/VKRCubemap.o

# target to build an object file
engine/core/Objects/VKRCubemap.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRCubemap.cpp.o
.PHONY : engine/core/Objects/VKRCubemap.cpp.o

engine/core/Objects/VKRCubemap.i: engine/core/Objects/VKRCubemap.cpp.i
.PHONY : engine/core/Objects/VKRCubemap.i

# target to preprocess a source file
engine/core/Objects/VKRCubemap.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRCubemap.cpp.i
.PHONY : engine/core/Objects/VKRCubemap.cpp.i

engine/core/Objects/VKRCubemap.s: engine/core/Objects/VKRCubemap.cpp.s
.PHONY : engine/core/Objects/VKRCubemap.s

# target to generate assembly for a file
engine/core/Objects/VKRCubemap.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRCubemap.cpp.s
.PHONY : engine/core/Objects/VKRCubemap.cpp.s

engine/core/Objects/VKRLight.o: engine/core/Objects/VKRLight.cpp.o
.PHONY : engine/core/Objects/VKRLight.o

# target to build an object file
engine/core/Objects/VKRLight.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRLight.cpp.o
.PHONY : engine/core/Objects/VKRLight.cpp.o

engine/core/Objects/VKRLight.i: engine/core/Objects/VKRLight.cpp.i
.PHONY : engine/core/Objects/VKRLight.i

# target to preprocess a source file
engine/core/Objects/VKRLight.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRLight.cpp.i
.PHONY : engine/core/Objects/VKRLight.cpp.i

engine/core/Objects/VKRLight.s: engine/core/Objects/VKRLight.cpp.s
.PHONY : engine/core/Objects/VKRLight.s

# target to generate assembly for a file
engine/core/Objects/VKRLight.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRLight.cpp.s
.PHONY : engine/core/Objects/VKRLight.cpp.s

engine/core/Objects/VKRModel.o: engine/core/Objects/VKRModel.cpp.o
.PHONY : engine/core/Objects/VKRModel.o

# target to build an object file
engine/core/Objects/VKRModel.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRModel.cpp.o
.PHONY : engine/core/Objects/VKRModel.cpp.o

engine/core/Objects/VKRModel.i: engine/core/Objects/VKRModel.cpp.i
.PHONY : engine/core/Objects/VKRModel.i

# target to preprocess a source file
engine/core/Objects/VKRModel.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRModel.cpp.i
.PHONY : engine/core/Objects/VKRModel.cpp.i

engine/core/Objects/VKRModel.s: engine/core/Objects/VKRModel.cpp.s
.PHONY : engine/core/Objects/VKRModel.s

# target to generate assembly for a file
engine/core/Objects/VKRModel.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/Objects/VKRModel.cpp.s
.PHONY : engine/core/Objects/VKRModel.cpp.s

engine/core/VKRRenderPass.o: engine/core/VKRRenderPass.cpp.o
.PHONY : engine/core/VKRRenderPass.o

# target to build an object file
engine/core/VKRRenderPass.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRRenderPass.cpp.o
.PHONY : engine/core/VKRRenderPass.cpp.o

engine/core/VKRRenderPass.i: engine/core/VKRRenderPass.cpp.i
.PHONY : engine/core/VKRRenderPass.i

# target to preprocess a source file
engine/core/VKRRenderPass.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRRenderPass.cpp.i
.PHONY : engine/core/VKRRenderPass.cpp.i

engine/core/VKRRenderPass.s: engine/core/VKRRenderPass.cpp.s
.PHONY : engine/core/VKRRenderPass.s

# target to generate assembly for a file
engine/core/VKRRenderPass.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRRenderPass.cpp.s
.PHONY : engine/core/VKRRenderPass.cpp.s

engine/core/VKRScene.o: engine/core/VKRScene.cpp.o
.PHONY : engine/core/VKRScene.o

# target to build an object file
engine/core/VKRScene.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRScene.cpp.o
.PHONY : engine/core/VKRScene.cpp.o

engine/core/VKRScene.i: engine/core/VKRScene.cpp.i
.PHONY : engine/core/VKRScene.i

# target to preprocess a source file
engine/core/VKRScene.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRScene.cpp.i
.PHONY : engine/core/VKRScene.cpp.i

engine/core/VKRScene.s: engine/core/VKRScene.cpp.s
.PHONY : engine/core/VKRScene.s

# target to generate assembly for a file
engine/core/VKRScene.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRScene.cpp.s
.PHONY : engine/core/VKRScene.cpp.s

engine/core/VKRenderers.o: engine/core/VKRenderers.cpp.o
.PHONY : engine/core/VKRenderers.o

# target to build an object file
engine/core/VKRenderers.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRenderers.cpp.o
.PHONY : engine/core/VKRenderers.cpp.o

engine/core/VKRenderers.i: engine/core/VKRenderers.cpp.i
.PHONY : engine/core/VKRenderers.i

# target to preprocess a source file
engine/core/VKRenderers.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRenderers.cpp.i
.PHONY : engine/core/VKRenderers.cpp.i

engine/core/VKRenderers.s: engine/core/VKRenderers.cpp.s
.PHONY : engine/core/VKRenderers.s

# target to generate assembly for a file
engine/core/VKRenderers.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/core/VKRenderers.cpp.s
.PHONY : engine/core/VKRenderers.cpp.s

engine/editor/Editor.o: engine/editor/Editor.cpp.o
.PHONY : engine/editor/Editor.o

# target to build an object file
engine/editor/Editor.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/editor/Editor.cpp.o
.PHONY : engine/editor/Editor.cpp.o

engine/editor/Editor.i: engine/editor/Editor.cpp.i
.PHONY : engine/editor/Editor.i

# target to preprocess a source file
engine/editor/Editor.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/editor/Editor.cpp.i
.PHONY : engine/editor/Editor.cpp.i

engine/editor/Editor.s: engine/editor/Editor.cpp.s
.PHONY : engine/editor/Editor.s

# target to generate assembly for a file
engine/editor/Editor.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/editor/Editor.cpp.s
.PHONY : engine/editor/Editor.cpp.s

engine/filesystem/ResourceManager.o: engine/filesystem/ResourceManager.cpp.o
.PHONY : engine/filesystem/ResourceManager.o

# target to build an object file
engine/filesystem/ResourceManager.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/filesystem/ResourceManager.cpp.o
.PHONY : engine/filesystem/ResourceManager.cpp.o

engine/filesystem/ResourceManager.i: engine/filesystem/ResourceManager.cpp.i
.PHONY : engine/filesystem/ResourceManager.i

# target to preprocess a source file
engine/filesystem/ResourceManager.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/filesystem/ResourceManager.cpp.i
.PHONY : engine/filesystem/ResourceManager.cpp.i

engine/filesystem/ResourceManager.s: engine/filesystem/ResourceManager.cpp.s
.PHONY : engine/filesystem/ResourceManager.s

# target to generate assembly for a file
engine/filesystem/ResourceManager.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/filesystem/ResourceManager.cpp.s
.PHONY : engine/filesystem/ResourceManager.cpp.s

engine/filesystem/gltfReader.o: engine/filesystem/gltfReader.cpp.o
.PHONY : engine/filesystem/gltfReader.o

# target to build an object file
engine/filesystem/gltfReader.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/filesystem/gltfReader.cpp.o
.PHONY : engine/filesystem/gltfReader.cpp.o

engine/filesystem/gltfReader.i: engine/filesystem/gltfReader.cpp.i
.PHONY : engine/filesystem/gltfReader.i

# target to preprocess a source file
engine/filesystem/gltfReader.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/filesystem/gltfReader.cpp.i
.PHONY : engine/filesystem/gltfReader.cpp.i

engine/filesystem/gltfReader.s: engine/filesystem/gltfReader.cpp.s
.PHONY : engine/filesystem/gltfReader.s

# target to generate assembly for a file
engine/filesystem/gltfReader.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/filesystem/gltfReader.cpp.s
.PHONY : engine/filesystem/gltfReader.cpp.s

engine/main.o: engine/main.cpp.o
.PHONY : engine/main.o

# target to build an object file
engine/main.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/main.cpp.o
.PHONY : engine/main.cpp.o

engine/main.i: engine/main.cpp.i
.PHONY : engine/main.i

# target to preprocess a source file
engine/main.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/main.cpp.i
.PHONY : engine/main.cpp.i

engine/main.s: engine/main.cpp.s
.PHONY : engine/main.s

# target to generate assembly for a file
engine/main.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/main.cpp.s
.PHONY : engine/main.cpp.s

engine/video/VKBackend.o: engine/video/VKBackend.cpp.o
.PHONY : engine/video/VKBackend.o

# target to build an object file
engine/video/VKBackend.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/video/VKBackend.cpp.o
.PHONY : engine/video/VKBackend.cpp.o

engine/video/VKBackend.i: engine/video/VKBackend.cpp.i
.PHONY : engine/video/VKBackend.i

# target to preprocess a source file
engine/video/VKBackend.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/video/VKBackend.cpp.i
.PHONY : engine/video/VKBackend.cpp.i

engine/video/VKBackend.s: engine/video/VKBackend.cpp.s
.PHONY : engine/video/VKBackend.s

# target to generate assembly for a file
engine/video/VKBackend.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/video/VKBackend.cpp.s
.PHONY : engine/video/VKBackend.cpp.s

engine/video/VKRenderable.o: engine/video/VKRenderable.cpp.o
.PHONY : engine/video/VKRenderable.o

# target to build an object file
engine/video/VKRenderable.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/video/VKRenderable.cpp.o
.PHONY : engine/video/VKRenderable.cpp.o

engine/video/VKRenderable.i: engine/video/VKRenderable.cpp.i
.PHONY : engine/video/VKRenderable.i

# target to preprocess a source file
engine/video/VKRenderable.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/video/VKRenderable.cpp.i
.PHONY : engine/video/VKRenderable.cpp.i

engine/video/VKRenderable.s: engine/video/VKRenderable.cpp.s
.PHONY : engine/video/VKRenderable.s

# target to generate assembly for a file
engine/video/VKRenderable.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/VKRenderer.dir/build.make CMakeFiles/VKRenderer.dir/engine/video/VKRenderable.cpp.s
.PHONY : engine/video/VKRenderable.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... VKRenderer"
	@echo "... dependencies/imgui/backends/imgui_impl_glfw.o"
	@echo "... dependencies/imgui/backends/imgui_impl_glfw.i"
	@echo "... dependencies/imgui/backends/imgui_impl_glfw.s"
	@echo "... dependencies/imgui/backends/imgui_impl_vulkan.o"
	@echo "... dependencies/imgui/backends/imgui_impl_vulkan.i"
	@echo "... dependencies/imgui/backends/imgui_impl_vulkan.s"
	@echo "... dependencies/imgui/imgui.o"
	@echo "... dependencies/imgui/imgui.i"
	@echo "... dependencies/imgui/imgui.s"
	@echo "... dependencies/imgui/imgui_draw.o"
	@echo "... dependencies/imgui/imgui_draw.i"
	@echo "... dependencies/imgui/imgui_draw.s"
	@echo "... dependencies/imgui/imgui_tables.o"
	@echo "... dependencies/imgui/imgui_tables.i"
	@echo "... dependencies/imgui/imgui_tables.s"
	@echo "... dependencies/imgui/imgui_widgets.o"
	@echo "... dependencies/imgui/imgui_widgets.i"
	@echo "... dependencies/imgui/imgui_widgets.s"
	@echo "... engine/core/Materials/VKRCubemapMaterial.o"
	@echo "... engine/core/Materials/VKRCubemapMaterial.i"
	@echo "... engine/core/Materials/VKRCubemapMaterial.s"
	@echo "... engine/core/Materials/VKRDebugMaterial.o"
	@echo "... engine/core/Materials/VKRDebugMaterial.i"
	@echo "... engine/core/Materials/VKRDebugMaterial.s"
	@echo "... engine/core/Materials/VKRMaterial.o"
	@echo "... engine/core/Materials/VKRMaterial.i"
	@echo "... engine/core/Materials/VKRMaterial.s"
	@echo "... engine/core/Materials/VKRShader.o"
	@echo "... engine/core/Materials/VKRShader.i"
	@echo "... engine/core/Materials/VKRShader.s"
	@echo "... engine/core/Materials/VKRTexture.o"
	@echo "... engine/core/Materials/VKRTexture.i"
	@echo "... engine/core/Materials/VKRTexture.s"
	@echo "... engine/core/Objects/VKRCubemap.o"
	@echo "... engine/core/Objects/VKRCubemap.i"
	@echo "... engine/core/Objects/VKRCubemap.s"
	@echo "... engine/core/Objects/VKRLight.o"
	@echo "... engine/core/Objects/VKRLight.i"
	@echo "... engine/core/Objects/VKRLight.s"
	@echo "... engine/core/Objects/VKRModel.o"
	@echo "... engine/core/Objects/VKRModel.i"
	@echo "... engine/core/Objects/VKRModel.s"
	@echo "... engine/core/VKRRenderPass.o"
	@echo "... engine/core/VKRRenderPass.i"
	@echo "... engine/core/VKRRenderPass.s"
	@echo "... engine/core/VKRScene.o"
	@echo "... engine/core/VKRScene.i"
	@echo "... engine/core/VKRScene.s"
	@echo "... engine/core/VKRenderers.o"
	@echo "... engine/core/VKRenderers.i"
	@echo "... engine/core/VKRenderers.s"
	@echo "... engine/editor/Editor.o"
	@echo "... engine/editor/Editor.i"
	@echo "... engine/editor/Editor.s"
	@echo "... engine/filesystem/ResourceManager.o"
	@echo "... engine/filesystem/ResourceManager.i"
	@echo "... engine/filesystem/ResourceManager.s"
	@echo "... engine/filesystem/gltfReader.o"
	@echo "... engine/filesystem/gltfReader.i"
	@echo "... engine/filesystem/gltfReader.s"
	@echo "... engine/main.o"
	@echo "... engine/main.i"
	@echo "... engine/main.s"
	@echo "... engine/video/VKBackend.o"
	@echo "... engine/video/VKBackend.i"
	@echo "... engine/video/VKBackend.s"
	@echo "... engine/video/VKRenderable.o"
	@echo "... engine/video/VKRenderable.i"
	@echo "... engine/video/VKRenderable.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

