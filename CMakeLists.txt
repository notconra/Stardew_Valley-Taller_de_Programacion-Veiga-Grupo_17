cmake_minimum_required(VERSION 3.22)
project(stardew_taller CXX)

# C++20 y POSIX 2008
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(_POSIX_C_SOURCE=200809L)
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Todos los .cpp de cada carpeta se compilan solos.
file(GLOB_RECURSE COMMON CONFIGURE_DEPENDS common/*.cpp)
file(GLOB_RECURSE SERVER CONFIGURE_DEPENDS server/*.cpp)
file(GLOB_RECURSE CLIENT CONFIGURE_DEPENDS client/*.cpp)

# La demo grafica (client/vista_grafica) necesita SDL2, que el cliente de
# consola no usa. Se la saca del resto de CLIENT para que "client" no
# termine con dos main().
set(CLIENT_GRAFICO)
foreach(archivo ${CLIENT})
  if(archivo MATCHES "/vista_grafica/")
    list(APPEND CLIENT_GRAFICO ${archivo})
  endif()
endforeach()
list(REMOVE_ITEM CLIENT ${CLIENT_GRAFICO})

# Puede haber mas de un main*.cpp sueltos en el disco (demos distintas). Las
# clases comunes van sin ningun main, y cada ejecutable agrega SOLO el suyo.
set(CLIENT_GRAFICO_COMUN ${CLIENT_GRAFICO})
list(FILTER CLIENT_GRAFICO_COMUN EXCLUDE REGEX "/vista_grafica/main[^/]*\\.cpp$")

add_executable(server ${SERVER} ${COMMON})
target_include_directories(server PRIVATE common server)
target_link_libraries(server PRIVATE Threads::Threads)

add_executable(client ${CLIENT} ${COMMON})
target_include_directories(client PRIVATE common client)
target_link_libraries(client PRIVATE Threads::Threads)

add_executable(client_grafico ${CLIENT_GRAFICO_COMUN} client/vista_grafica/main.cpp ${COMMON})
target_include_directories(client_grafico PRIVATE common client/vista_grafica
                                                   client/vista_grafica/externo/tinyxml2 ${SDL2_INCLUDE_DIRS}
                                                   ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(client_grafico PRIVATE Threads::Threads ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
target_link_directories(client_grafico PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS})

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
  target_link_libraries(tests PRIVATE GTest::gtest_main Threads::Threads)
  add_test(NAME tests COMMAND tests)
else()
  message(WARNING "GoogleTest no esta instalado: no se compilan los tests (sudo apt install libgtest-dev)")
endif()