# Copyright 2021 DeepAlign Tech

cmake_minimum_required(VERSION 3.15)
project(da_seg VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT APP_ID OR NOT APP_CODE)
    message( FATAL_ERROR "Credential needed. command: cmake .. -DAPP_ID=YOUR_ID -DAPP_CODE=YOUR_CODE" )
endif()

string (LENGTH ${APP_ID} APP_ID_LEN)
string (LENGTH ${APP_CODE} APP_CODE_LEN)

message("APP_ID=${APP_ID}")
message("APP_CODE=${APP_CODE}")

if( APP_CODE_LEN LESS 9 )
    message( FATAL_ERROR "Credential format is not correct!" )
endif()

add_definitions( -DAPP_ID="${APP_ID}" )
add_definitions( -DAPP_CODE="${APP_CODE}" )
add_definitions( -DSERVER_URL="http://47.98.115.12/segmentation/job" )

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(zip GIT_REPOSITORY https://github.com/nih-at/libzip.git GIT_TAG 66e496489bdae81bfda8b0088172871d8fda0032) # the commit hash for 1.7.3
FetchContent_MakeAvailable(zip)

include_directories(include)

add_executable (seg seg.cpp)
target_link_libraries(seg PRIVATE cpr::cpr)
target_link_libraries(seg PRIVATE zip)