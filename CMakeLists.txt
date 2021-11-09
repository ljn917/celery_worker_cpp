cmake_minimum_required(VERSION 3.15)

project(celery_worker_cpp)

option(BUILD_TESTS "Build the C++ tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(LIBS "")

# jsoncpp, https://github.com/open-source-parsers/jsoncpp
find_package(Jsoncpp REQUIRED)
set(LIBS ${LIBS} Jsoncpp::Jsoncpp)
#include_directories(${JSONCPP_INCLUDE_DIRS})

find_package(Hiredis REQUIRED)
set(LIBS ${LIBS} Hiredis::Hiredis)

find_package(Rabbitmqc REQUIRED)
set(LIBS ${LIBS} Rabbitmqc::Rabbitmqc)

#find_package(OpenSSL REQUIRED)
#set(LIBS ${LIBS} OpenSSL::SSL OpenSSL::Crypto)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin_release)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin_debug)
endif()


# source
include_directories(${CMAKE_SOURCE_DIR}/celery_worker)
# add_subdirectory(celery_worker)

if(BUILD_TESTS)
include(CTest)
add_subdirectory(test)
endif()
