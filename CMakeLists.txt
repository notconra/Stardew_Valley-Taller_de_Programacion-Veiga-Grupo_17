cmake_minimum_required(VERSION 3.22)
project(stardew_taller CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_definitions(_POSIX_C_SOURCE=200809L)
add_compile_options(-Wall -Wextra -Wpedantic)

find_program(CCACHE ccache)
if(CCACHE)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

find_program(LLD ld.lld)
if(LLD)
  add_link_options(-fuse-ld=lld)
endif()

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

file(GLOB_RECURSE COMMON_FUENTES CONFIGURE_DEPENDS common/*.cpp)
file(GLOB_RECURSE SERVER_FUENTES CONFIGURE_DEPENDS server/*.cpp)
file(GLOB_RECURSE CLIENT_FUENTES CONFIGURE_DEPENDS client/*.cpp)
list(FILTER SERVER_FUENTES EXCLUDE REGEX "/main\\.cpp$")
list(FILTER CLIENT_FUENTES EXCLUDE REGEX "/main\\.cpp$")

set(ENCABEZADOS_PRECOMPILADOS
    <algorithm> <cstddef> <cstdint> <map> <memory> <optional> <set> <string> <utility> <variant>
    <vector>)

add_library(lib_common STATIC ${COMMON_FUENTES})
target_include_directories(lib_common PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(lib_common PUBLIC Threads::Threads)
target_precompile_headers(lib_common PRIVATE ${ENCABEZADOS_PRECOMPILADOS})

add_library(lib_server STATIC ${SERVER_FUENTES})
target_include_directories(lib_server PUBLIC ${PROJECT_SOURCE_DIR}/server)
target_link_libraries(lib_server PUBLIC lib_common)
target_precompile_headers(lib_server PRIVATE ${ENCABEZADOS_PRECOMPILADOS})

add_library(lib_client STATIC ${CLIENT_FUENTES})
target_include_directories(
  lib_client PUBLIC ${PROJECT_SOURCE_DIR}/client ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_directories(lib_client PUBLIC ${SDL2_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS})
target_link_libraries(
  lib_client PUBLIC lib_common Qt6::Widgets ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
target_precompile_headers(lib_client PRIVATE ${ENCABEZADOS_PRECOMPILADOS})

add_executable(server server/main.cpp)
target_link_libraries(server PRIVATE lib_server)

add_executable(client client/main.cpp)
target_link_libraries(client PRIVATE lib_client)

set_target_properties(lib_client client PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

find_package(GTest)
if(GTest_FOUND)
  enable_testing()
  file(GLOB_RECURSE TEST_FUENTES CONFIGURE_DEPENDS tests/*.cpp)
  add_executable(tests EXCLUDE_FROM_ALL ${TEST_FUENTES})
  target_link_libraries(tests PRIVATE GTest::gtest_main lib_server lib_client)
  add_test(NAME tests COMMAND tests)
else()
  message(WARNING "GoogleTest no esta instalado: no se compilan los tests (sudo apt install libgtest-dev)")
endif()
