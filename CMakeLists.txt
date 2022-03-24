# Copyright 2021 DeepAlign Tech
# Copyright 2022 ChohoTech

cmake_minimum_required(VERSION 3.15)
project(da_seg VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT USER_ID OR NOT USER_TOKEN OR NOT SERVER_URL)
    message( FATAL_ERROR "Credential needed. command: cmake .. -DUSER_ID=YOUR_ID -DUSER_TOKEN=YOUR_TOKEN -DSERVER_URL=SERVER_URL" )
endif()

string (LENGTH ${USER_ID} USER_ID_LEN)
string (LENGTH ${USER_TOKEN} USER_TOKEN_LEN)

message("USER_ID=${USER_ID}")
message("USER_TOKEN=${USER_TOKEN}")
message("SERVER_URL=${SERVER_URL}")

if( USER_TOKEN_LEN LESS 9 )
    message( FATAL_ERROR "Credential format is not correct!" )
endif()

add_definitions( -DUSER_ID="${USER_ID}" )
add_definitions( -DUSER_TOKEN="${USER_TOKEN}" )
add_definitions( -DSERVER_URL="${SERVER_URL}" )

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://gitee.com/chohotech/cpr.git GIT_TAG 709145af4a44fc18a37db8c271c9c3e2e187cfe1) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

include_directories(include)

add_executable (seg seg.cpp base64.cpp)
target_link_libraries(seg PRIVATE cpr::cpr)