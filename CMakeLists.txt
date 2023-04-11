cmake_minimum_required(VERSION 3.20)
project(browser)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
option(sanitize OFF)
include(FindPkgConfig)
find_package(Boost REQUIRED COMPONENTS coroutine)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(WEBKIT REQUIRED IMPORTED_TARGET webkit2gtk-4.0)
add_executable(gKiosk main.cc)
target_link_libraries(gKiosk PRIVATE PkgConfig::GTK PkgConfig::WEBKIT)
target_compile_options(gKiosk PRIVATE -Werror=all -Werror=extra -Werror=pedantic -Og -g)

macro(sanitize TARGET)
    target_compile_options(${TARGET} PRIVATE -fsanitize=undefined -fsanitize=address)
    target_link_options(${TARGET} PRIVATE -fsanitize=undefined -fsanitize=address)
endmacro()





add_executable(taskHelper taskHelper.cpp)
target_include_directories(taskHelper PRIVATE ${CMAKE_SOURCE_DIR}/boost-headers/include)
target_include_directories(taskHelper PRIVATE ${MY_INCLUDE_DIRS})
set(MY_LIBRARIES Boost::coroutine)
target_link_libraries(taskHelper PRIVATE ${MY_LIBRARIES})

if(sanitize)
    message(STATUS "sanitize enabled")
    sanitize(gKiosk)
    sanitize(taskHelper)
endif()