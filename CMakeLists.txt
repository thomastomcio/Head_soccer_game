cmake_minimum_required(VERSION 3.0.0)

# set project name
project(Headsoccer VERSION 0.1.0)

set( SOURCE_FILES
     ./src/glad.c
     ./src/main.cpp
)

add_executable(Headsoccer ${SOURCE_FILES})

# list of static libraries to link wit the target
target_link_libraries(Headsoccer "${CMAKE_SOURCE_DIR}/lib/libglfw3.a")

# list of paths to search for include files
target_include_directories(Headsoccer PUBLIC "./include" )


# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)




