cmake_minimum_required(VERSION 3.0.0)
project(config-pp VERSION 0.1.0)

include(CTest)
enable_testing()

link_libraries(-lyaml-cpp)
add_library(config-pp env.cpp config.cpp ./reader/yaml.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
