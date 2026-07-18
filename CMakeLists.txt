cmake_minimum_required(VERSION 3.15)
project(allocator LANGUAGES CXX)

option(BUILD_DRIVER  "Build the example 'main' executable" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ALLOCATOR_SRC
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_library(allocator SHARED ${ALLOCATOR_SRC})

target_include_directories(allocator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

if(BUILD_DRIVER)
    add_executable(main main.cpp)
    target_link_libraries(main
      PRIVATE
        allocator
    )

    add_custom_target(example
        DEPENDS main
    )
endif()

install(TARGETS allocator
        EXPORT allocatorTargets
        LIBRARY DESTINATION lib)

install(DIRECTORY include/allocator
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

install(EXPORT allocatorTargets
        FILE allocatorTargets.cmake
        NAMESPACE allocator::
        DESTINATION lib/cmake/allocator)