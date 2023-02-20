# CMakeList.txt: proyecto de CMake para ZeusEngine, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

# Agregue un origen al ejecutable de este proyecto.
add_executable (ZeusEngine "main.cpp" "public/Engine/Core.h" "private/Engine/Core.cpp" "public/Util/Util.h" "public/Engine/Shader.h" "private/Engine/Shader.cpp" "public/Engine/Vertex.h" "public/Engine/GameObject/GameObject.h" "private/Engine/GameObject/GameObject.cpp" "public/Math/Vector3.h" "private/Math/Vector3.cpp"  "public/Math/Transform.h" "private/Math/Transform.cpp" "public/Engine/Scene/SceneManager.h" "private/Engine/Scene/SceneManager.cpp" "public/Engine/Scene/Scene.h" "private/Engine/Scene/Scene.cpp" "public/Engine/WVP.h"  "public/Engine/ScreenQuad.h" "private/Engine/ScreenQuad.cpp" "public/Engine/Camera/Camera.h" "private/Engine/Camera/Camera.cpp" "public/Engine/Input.h" "private/Engine/Input.cpp" "private/Engine/Camera/EditorCamera.cpp" "public/Engine/Camera/EditorCamera.h"  "private/Engine/Editor/Editor.cpp" "public/Engine/Editor/Editor.h" "public/Module/Time.h" "private/Module/Time.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ZeusEngine PROPERTY CXX_STANDARD 20)
  set_property(TARGET ZeusEngine PROPERTY WIN32_EXECUTABLE TRUE)
endif()

set(SOURCE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(DX_LIB "d3d12.lib;dxgi.lib;d3dcompiler.lib;${CMAKE_CURRENT_SOURCE_DIR}/../lib/directx/DirectXTK12.lib")
set(GLOBAL_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(ASSIMP_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib/assimp/assimp.lib")

set(IMGUI_DIR "${GLOBAL_INCLUDES}/imgui")
set(IMGUIZMO_DIR "${GLOBAL_INCLUDES}/imguizmo")

file(GLOB "IMGUI_FILES" 
	"${IMGUI_DIR}/imgui.cpp"
	"${IMGUI_DIR}/imgui_draw.cpp"
	"${IMGUI_DIR}/imgui_tables.cpp"
	"${IMGUI_DIR}/imgui_widgets.cpp"
	"${IMGUI_DIR}/backends/imgui_impl_dx12.cpp"
	"${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
)

file(GLOB "IMGUIZMO_FILES"
	"${IMGUIZMO_DIR}/*.cpp"
	"${IMGUIZMO_DIR}/*.h"
)

add_compile_definitions(NOMINMAX)

add_library("ImGUI" "${IMGUI_FILES}")
add_library("imguizmo" "${IMGUIZMO_FILES}")

target_link_libraries(ZeusEngine "${DX_LIB}")
target_link_libraries(ZeusEngine "${ASSIMP_LIB}")
target_link_libraries(ZeusEngine ImGUI)
target_link_libraries(ZeusEngine imguizmo)
include_directories("${SOURCE_INCLUDES}")
include_directories("${GLOBAL_INCLUDES}")