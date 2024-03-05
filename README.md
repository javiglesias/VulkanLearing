# VulkanLearing
Learn Vulkan API and Rendering concepts.
Este es un repo personal en el que voy aprendiendo como hacer un motor de videojuegos con un backend en Vulkan,
de momento.
Mas adelante me gustaria meter otro backend de Directx12 (No se si meter otro de OGL).

# Linux Enviro config
sudo apt install make
sudo apt install g++
sudo apt install vulkan-tools
sudo apt install libvulkan-dev
sudo apt install vulkan-validationlayers-dev spirv-tools
sudo apt install libglfw3-dev
sudo apt install libglm-dev

# TO-DO
Estos TO-DO los voy poniendo segun se me van ocurriendo, no tienen orden en concreto.

- Render de primitivas simples para DEBUG en otra pipeline
- blinn-phong
- Hot-Reload de Shaders en tiempo de ejecucion.
- Uber Shaders
- Render 2D (quads, etc)
- Compilacion de Shaders en runtime (ahora mismo se hacen en post-compilacion con glslc.exe)
- ordenar modelos segun material a la hora de pintar
- ordenar modelos segun posicion respecto a la camara
- Point lights
- mega-texturas
- forward+deferred
- Transparencias
- SSAO
- PBR
- Raytracing

# 3rd Party libraries
 - assimp
 - glad
 - glfw
 - glm
 - imgui
 - stb_image
## Futuras bibliotecas
 - https://github.com/aiekick/ImGuiFileDialog
 - https://github.com/thedmd/imgui-node-editor
 - FMOD
