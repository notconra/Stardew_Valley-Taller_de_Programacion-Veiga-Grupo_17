cmake_minimum_required(VERSION 3.22)
project(stardew_taller CXX)

# C++20 y POSIX 2008
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(_POSIX_C_SOURCE=200809L)
add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Threads REQUIRED)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Todos los .cpp de cada carpeta se compilan solos.
file(GLOB_RECURSE COMMON CONFIGURE_DEPENDS common/*.cpp)
file(GLOB_RECURSE SERVER CONFIGURE_DEPENDS server/*.cpp)
file(GLOB_RECURSE CLIENT CONFIGURE_DEPENDS client/*.cpp)

add_executable(server ${SERVER} ${COMMON})
target_include_directories(server PRIVATE common server)
target_link_libraries(server PRIVATE Threads::Threads)

add_executable(client ${CLIENT} ${COMMON})
target_include_directories(client PRIVATE common client)

target_link_libraries(client PRIVATE Threads::Threads Qt6::Widgets)

# Tests (opcional: si GoogleTest no esta instalado, el resto compila igual)
find_package(GTest)
if(GTest_FOUND)
  enable_testing()
  # Los tests van a usar el codigo de server y client, pero NO sus main.cpp
  set(SERVER_CLASSES ${SERVER})
  set(CLIENT_CLASSES ${CLIENT})
  list(FILTER SERVER_CLASSES EXCLUDE REGEX "main\\.cpp$")
  list(FILTER CLIENT_CLASSES EXCLUDE REGEX "main\\.cpp$")
  file(GLOB_RECURSE TEST CONFIGURE_DEPENDS tests/*.cpp)

  add_executable(tests ${TEST} ${COMMON} ${SERVER_CLASSES} ${CLIENT_CLASSES})
  target_include_directories(tests PRIVATE common server client)

 target_link_libraries(tests PRIVATE GTest::gtest_main Threads::Threads Qt6::Widgets)

  add_test(NAME tests COMMAND tests)
else()
  message(WARNING "GoogleTest no esta instalado: no se compilan los tests (sudo apt install libgtest-dev)")
endif()
