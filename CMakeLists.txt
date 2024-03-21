
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_executable(main)

target_sources(main PRIVATE main.cpp)


cmake_minimum_required(VERSION 3.22.1)


set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

target_link_libraries(main PRIVATE libmysqlcppconn8-static.a resolv)



project(web_crawler)







find_package(CURL REQUIRED)
target_link_libraries(main PRIVATE CURL::libcurl)

find_package(unofficial-gumbo CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::gumbo::gumbo)

find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::mysql-connector-cpp::connector)