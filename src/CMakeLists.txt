# CMakeList.txt: proyecto de CMake para ZeusEngine, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

# Agregue un origen al ejecutable de este proyecto.
add_executable (ZeusEngine  "main.cpp" "public/Engine/Core.h" "private/Engine/Core.cpp" "public/Engine/Util.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ZeusEngine PROPERTY CXX_STANDARD 20)
  set_property(TARGET ZeusEngine PROPERTY WIN32_EXECUTABLE TRUE)
endif()

set(SOURCE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(DX_LIB "d3d12.lib;dxgi.lib;d3dcompiler.lib")
set(GLOBAL_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../include")

target_link_libraries(ZeusEngine "${DX_LIB}")
include_directories("${SOURCE_INCLUDES}")
include_directories("${GLOBAL_INCLUDES}")