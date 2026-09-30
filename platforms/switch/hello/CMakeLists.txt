cmake_minimum_required(VERSION 3.13)
project(k2s_cocos_hello CXX C)

add_subdirectory(../cocos2dx cocos2dx)

add_executable(cocos_hello main.cpp)
target_link_libraries(cocos_hello PRIVATE cocos2d)
set_target_properties(cocos_hello PROPERTIES CXX_STANDARD 11)

nx_generate_nacp(cocos_hello.nacp NAME "K2S cocos hello" AUTHOR "Kirikiroid2-Switch" VERSION "0.0.1")
nx_create_nro(cocos_hello NACP cocos_hello.nacp)
