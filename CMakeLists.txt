cmake_minimum_required(VERSION 3.16)

project(RandomGenerateAlgorithm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RandomGenerateAlgorithm main.cpp)

include(GNUInstallDirs)
install(TARGETS RandomGenerateAlgorithm
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
