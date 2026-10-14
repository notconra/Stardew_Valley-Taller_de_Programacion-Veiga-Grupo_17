cmake_minimum_required(VERSION 3.22)
project(stardew_taller CXX)

# C++20 y POSIX 2008
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(_POSIX_C_SOURCE=200809L)
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)

# Todos los .cpp de cada carpeta se compilan solos.
file(GLOB COMMON CONFIGURE_DEPENDS common/*.cpp)
file(GLOB SERVER CONFIGURE_DEPENDS server/*.cpp)
file(GLOB CLIENT CONFIGURE_DEPENDS client/*.cpp)

add_executable(server ${SERVER} ${COMMON})
target_include_directories(server PRIVATE common server)
target_link_libraries(server PRIVATE Threads::Threads)

add_executable(client ${CLIENT} ${COMMON})
target_include_directories(client PRIVATE common client)
target_link_libraries(client PRIVATE Threads::Threads)

# Tests (opcional: si GoogleTest no esta instalado, el resto compila igual)
find_package(GTest)
if(GTest_FOUND)
  enable_testing()
  # Los tests van a usar el codigo de server y client, pero NO sus main.cpp
  set(SERVER_CLASSES ${SERVER})
  set(CLIENT_CLASSES ${CLIENT})
  list(FILTER SERVER_CLASSES EXCLUDE REGEX "main\\.cpp$")
  list(FILTER CLIENT_CLASSES EXCLUDE REGEX "main\\.cpp$")
  file(GLOB TEST CONFIGURE_DEPENDS tests/*.cpp)

  add_executable(tests ${TEST} ${COMMON} ${SERVER_CLASSES} ${CLIENT_CLASSES})
  target_include_directories(tests PRIVATE common server client)
  target_link_libraries(tests PRIVATE GTest::gtest_main Threads::Threads)
  add_test(NAME tests COMMAND tests)
else()
  message(WARNING "GoogleTest no esta instalado: no se compilan los tests (sudo apt install libgtest-dev)")
endif()

# Comprobacion de SDL y Qt (programas chicos en checks/; después se borra y se modifica para q compile lo que tengamos)
find_package(PkgConfig)
if(PkgConfig_FOUND)
  pkg_check_modules(SDL2_ALL IMPORTED_TARGET sdl2 SDL2_image SDL2_mixer SDL2_ttf)
endif()
if(SDL2_ALL_FOUND)
  add_executable(check_sdl checks/check_sdl.cpp)
  target_link_libraries(check_sdl PRIVATE PkgConfig::SDL2_ALL)
else()
  message(WARNING "Faltan bibliotecas de SDL: no se compila check_sdl (make deps)")
endif()

find_package(Qt5 COMPONENTS Widgets)
if(Qt5_FOUND)
  set(CMAKE_AUTOMOC ON)   # Qt necesita un paso extra (moc) para signals/slots; esto lo hace solo
  add_executable(check_qt checks/check_qt.cpp)
  target_link_libraries(check_qt PRIVATE Qt5::Widgets)
else()
  message(WARNING "Falta Qt5: no se compila check_qt (make deps)")
endif()
