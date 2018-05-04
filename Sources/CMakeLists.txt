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
)
set(SOURCES
    Asio.cpp
    Service.cpp
    Manager.cpp
    Factory.cpp
)

find_package(Threads REQUIRED)
find_package(Boost 1.58.0 COMPONENTS REQUIRED system)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_library(RatingServiceLib ${INCLUDES} ${SOURCES})
    add_executable(${PROJECT_NAME} main.cpp)
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} RatingServiceLib)
endif()

#TODO:
#enable_testing()
#add_test(NAME Tests COMMAND Tests)

add_subdirectory(Tests)
