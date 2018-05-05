cmake_minimum_required(VERSION 2.8)

set(INCLUDES
    IService.hpp
    Service.hpp
    IAsio.hpp
    Asio.hpp
    IManager.hpp
    Manager.hpp
    IFactory.hpp
    Factory.hpp
    IWorker.hpp
    Worker.hpp
    Handlers.hpp
)
set(SOURCES
    Factory.cpp
    Asio.cpp
    Manager.cpp
    Service.cpp
    Worker.cpp
)

find_package(Threads REQUIRED)
find_package(Boost 1.58.0 COMPONENTS REQUIRED system)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_library(RatingServiceLib ${INCLUDES} ${SOURCES})
    add_executable(${PROJECT_NAME} main.cpp)
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} RatingServiceLib ${CMAKE_THREAD_LIBS_INIT})
endif()

#TODO:
#enable_testing()
#add_test(NAME Tests COMMAND Tests)

add_subdirectory(Tests)
