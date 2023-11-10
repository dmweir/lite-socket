cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_STANDARD 99)
set(CMAKE_SUPPRESS_REGENERATION ON)

if(WIN32)
#   add_definitions(-D__WINDOWS__)
else()
  add_definitions(-D__UNIX__)
endif()

project(lite-socket VERSION 1.0)

set(INC "${PROJECT_SOURCE_DIR}/src")
set(SRC "${PROJECT_SOURCE_DIR}/src")
set(BIN "${PROJECT_SOURCE_DIR}/bin")
set(LIB "${PROJECT_SOURCE_DIR}/lib")

# DLL TARGET
add_library(shared_lite_socket SHARED ${SRC}/lite_socket.c)
target_include_directories(shared_lite_socket PUBLIC ${INC})
target_compile_definitions(shared_lite_socket PUBLIC LITE_SOCKET_C_EXPORTS)

if(MSVC)
    target_compile_options(shared_lite_socket PUBLIC /MT /W4)
else()
    target_compile_options(shared_lite_socket PUBLIC -Wno-reorder)
    target_compile_options(shared_lite_socket PUBLIC -Wall -Wextra -Wpedantic)
endif()

set_target_properties(shared_lite_socket PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${LIB}")
set_target_properties(shared_lite_socket PROPERTIES RUNTIME_OUTPUT_NAME "lite_socket")

# STATIC LIB TARGET
add_library(static_lite_socket STATIC ${SRC}/lite_socket.c)
target_include_directories(static_lite_socket PUBLIC ${INC})

if(MSVC)
    target_compile_options(static_lite_socket PUBLIC /MT /W4)
else()
    target_compile_options(static_lite_socket PUBLIC -Wno-reorder)
    target_compile_options(static_lite_socket PUBLIC -Wall -Wextra -Wpedantic)
endif()
set_target_properties(static_lite_socket PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${LIB}")
set_target_properties(static_lite_socket PROPERTIES ARCHIVE_OUTPUT_NAME "lite_socket")

# TEST EXAMPLE
add_executable(lite_socket_test ${PROJECT_SOURCE_DIR}/examples/C/lite_socket_test.c ${SRC}/lite_socket.c)

target_include_directories(lite_socket_test PUBLIC ${INC})

if(MSVC)
    target_compile_options(lite_socket_test PUBLIC /MT /W4)
else()
    target_compile_options(lite_socket_test PUBLIC -Wno-reorder)
    target_compile_options(lite_socket_test PUBLIC -Wall -Wextra -Wpedantic)
endif()
set_target_properties(lite_socket_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BIN}")