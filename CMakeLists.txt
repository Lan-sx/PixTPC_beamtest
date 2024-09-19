cmake_minimum_required(VERSION 3.0.0)
project(cepcPixTPC)

include(CTest)
enable_testing()

#---Generate JSON file for VIM 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#---Gdb Debug
#SET(CMAKE_BUILD_TYPE "Debug")
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g3 -ggdb -gdwarf-2")
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#set(CMAKE_CXX_STANDARD 14 CACHE STRING "")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

if(NOT TARGET Garfield::Garfield)
    find_package(Garfield REQUIRED)
endif()


find_package(ROOT REQUIRED COMPONENTS  RIO Hist Tree Net  Spectrum)

include(${ROOT_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})

ROOT_GENERATE_DICTIONARY(PixelTPCdata_dict PixelTPCdata.h MODULE PixelTPCdata LINKDEF LinkDef.h)

add_library(PixelTPCdata SHARED
            ${PROJECT_SOURCE_DIR}/PixelTPCdata.cpp
            PixelTPCdata_dict)

target_link_libraries(PixelTPCdata PUBLIC ${ROOT_LIBRARIES} Garfield::Garfield)

add_executable(${PROJECT_NAME} mainPixTPC.cpp PixelTPCdata.h PixelTPCdata.cpp ${sources} ${headers})
#target_link_libraries(${PROJECT_NAME} Garfield::Garfield ${ROOT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PixelTPCdata ${ROOT_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} ${ROOT_LIBRARIES})

#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
