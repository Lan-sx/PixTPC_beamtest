cmake_minimum_required(VERSION 3.10.0)
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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#Garfield++ CERN
if(NOT TARGET Garfield::Garfield)
    find_package(Garfield REQUIRED)
endif()

#ROOT CERN
find_package(ROOT REQUIRED COMPONENTS  RIO Hist Tree Net Spectrum MathCore MathMore)

#spdlog 
find_package(spdlog REQUIRED)

#nlohmann 
find_package(nlohmann_json REQUIRED)

include(${ROOT_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/dataModel)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/dataModel/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/dataModel/*.h)

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})

ROOT_GENERATE_DICTIONARY(PixelTPCdata_dict ${PROJECT_SOURCE_DIR}/dataModel/PixelTPCdata.h 
                                           MODULE PixelTPCdata 
                                           LINKDEF ${PROJECT_SOURCE_DIR}/dataModel/LinkDef_PixelTPCdata.h)

ROOT_GENERATE_DICTIONARY(MCTrackdata_dict ${PROJECT_SOURCE_DIR}/dataModel/MCTrackdata.h 
                                           MODULE MCTrackdata 
                                           LINKDEF ${PROJECT_SOURCE_DIR}/dataModel/LinkDef_MCTrackdata.h)
add_library(PixelTPCdata SHARED
            ${PROJECT_SOURCE_DIR}/dataModel/PixelTPCdata.cpp
            PixelTPCdata_dict)

add_library(MCTrackdata SHARED
            ${PROJECT_SOURCE_DIR}/dataModel/MCTrackdata.cpp
            MCTrackdata_dict)

target_link_libraries(PixelTPCdata PUBLIC ${ROOT_LIBRARIES} Garfield::Garfield)
target_link_libraries(MCTrackdata  PUBLIC ${ROOT_LIBRARIES} Garfield::Garfield)

#add_executable(${PROJECT_NAME} mainPixTPC.cpp PixelTPCdata.h PixelTPCdata.cpp ${sources} ${headers})
add_executable(${PROJECT_NAME} mainPixTPC.cpp  ${sources} ${headers})
#target_link_libraries(${PROJECT_NAME} Garfield::Garfield ${ROOT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PixelTPCdata MCTrackdata nlohmann_json::nlohmann_json ${ROOT_LIBRARIES} spdlog::spdlog)
#target_link_libraries(${PROJECT_NAME} ${ROOT_LIBRARIES})

#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

#install 
make_directory(${CMAKE_SOURCE_DIR}/install)
set(INSTALL_BIN_DIR "${CMAKE_SOURCE_DIR}/install/bin" CACHE PATH "Installation dir for executables")
set(INSTALL_LIB_DIR "${CMAKE_SOURCE_DIR}/install/lib" CACHE PATH "Installation dir for lib")
set(INSTALL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/install/include" CACHE PATH "Installation dir for include")

FILE(GLOB PCM_FILES "${CMAKE_CURRENT_BINARY_DIR}/*.pcm")
FILE(GLOB ROOTMAP_FILES "${CMAKE_CURRENT_BINARY_DIR}/*.rootmap")
FILE(GLOB HEADERFILES "${PROJECT_SOURCE_DIR}/include/*.h" "${PROJECT_SOURCE_DIR}/dataModel/*.h")

install(TARGETS ${PROJECT_NAME} DESTINATION "${INSTALL_BIN_DIR}")
install(TARGETS PixelTPCdata MCTrackdata DESTINATION "${INSTALL_LIB_DIR}")
install(FILES ${HEADERFILES} DESTINATION "${INSTALL_INCLUDE_DIR}")
install(FILES ${PCM_FILES} ${ROOTMAP_FILES} DESTINATION "${INSTALL_LIB_DIR}")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
