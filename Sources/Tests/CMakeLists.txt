project(RatingServiceTests)
cmake_minimum_required(VERSION 2.8)

set(INCLUDES Mocks.hpp)
set(SOURCES main.cpp ServiceTests.cpp ManagerTests.cpp)

find_package(Threads)
find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES} ${INCLUDES})
target_link_libraries(
    ${PROJECT_NAME}
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    libgmock.a
    RatingServiceLib)
