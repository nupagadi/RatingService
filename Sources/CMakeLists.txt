cmake_minimum_required(VERSION 2.8)

set(INCLUDES
    Header/Asio.hpp
    Header/Data.hpp
    Header/Factory.hpp
    Header/Handlers.hpp
    Interface/IAsio.hpp
    Interface/IData.hpp
    Interface/IFactory.hpp
    Interface/IManager.hpp
    Interface/IService.hpp
    Interface/IWorker.hpp
    Header/Manager.hpp
    Header/RawMessageTools.hpp
    Header/Service.hpp
    Header/Types.hpp
    Header/Worker.hpp
)
set(SOURCES
    Source/Asio.cpp
    Source/Data.cpp
    Source/Factory.cpp
    Source/Manager.cpp
    Source/Service.cpp
    Source/Worker.cpp
)

include_directories(Interface)
include_directories(Header)

find_package(Threads REQUIRED)
find_package(Boost 1.58.0 COMPONENTS REQUIRED system)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_library(RatingServiceLib ${INCLUDES} ${SOURCES})
    add_executable(../${PROJECT_NAME} main.cpp)
    target_link_libraries(../${PROJECT_NAME} ${Boost_LIBRARIES} RatingServiceLib ${CMAKE_THREAD_LIBS_INIT})
endif()

#TODO:
#enable_testing()
#add_test(NAME Tests COMMAND Tests)

add_subdirectory(Tests)
