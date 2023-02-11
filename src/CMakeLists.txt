# CMakeList.txt: proyecto de CMake para ZeusEngine, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

# Agregue un origen al ejecutable de este proyecto.
add_executable (ZeusEngine  "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ZeusEngine PROPERTY CXX_STANDARD 20)
  set_property(TARGET ZeusEngine PROPERTY WIN32_EXECUTABLE TRUE)
endif()

set(SOURCE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/public")

include_directories("${SOURCE_INCLUDES}")