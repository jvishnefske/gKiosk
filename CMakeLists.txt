cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(kiosk)
include(FindPkgConfig)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(WEBKIT REQUIRED IMPORTED_TARGET webkit2gtk-4.0)
add_executable(gKiosk main.cc)
target_link_libraries(gKiosk PRIVATE PkgConfig::GTK PkgConfig::WEBKIT)
target_compile_options(gKiosk PRIVATE -Werror=all -Werror=extra -Werror=pedantic)


