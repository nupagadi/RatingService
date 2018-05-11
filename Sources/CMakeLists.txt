cmake_minimum_required(VERSION 2.8)

set(INCLUDES
    Asio.hpp
    Data.hpp
    Factory.hpp
    Handlers.hpp
    IAsio.hpp
    IData.hpp
    IFactory.hpp
    IManager.hpp
    IService.hpp
    IWorker.hpp
    Manager.hpp
    RawMessageTools.hpp
    Service.hpp
    Types.hpp
    Worker.hpp
)
set(SOURCES
    Asio.cpp
    Data.cpp
    Factory.cpp
    Manager.cpp
    Service.cpp
    Worker.cpp
)

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
