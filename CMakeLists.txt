

add_executable(main)

target_sources(main PRIVATE main.cpp)


cmake_minimum_required(VERSION 3.22.1)


set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")




project(web_crawler)






find_package(CURL REQUIRED)
target_link_libraries(main PRIVATE CURL::libcurl)

find_package(unofficial-gumbo CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::gumbo::gumbo)
