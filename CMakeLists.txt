cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_C_COMPILER "clang-6.0")
set(CMAKE_CXX_COMPILER "clang++-6.0")


project(style_checker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wcast-qual -Wconversion -Wunreachable-code -Wmissing-noreturn -Wshadow -Wfatal-errors")

include_directories(/usr/lib/llvm-6.0/include)

file(GLOB SRC *.cpp)

add_executable(style_checker ${SRC})

file(GLOB CLANG_FORMAT_FILE .clang-format)

target_link_libraries(style_checker clang)


find_program(CLANG_FORMAT clang-format) 
if(EXISTS ${CLANG_FORMAT})
  add_custom_target(clangformat
        COMMAND clang-format
        -style=file
        -assume-filename=${CLANG_FORMAT_FILE}
        -i
        ${SRC}
  )

  # XXX: We should run cppcheck target before gfaserver and gfaserverd
  add_dependencies(style_checker clangformat)
else()
  message(AUTHOR_WARNING "clang-format is not installed, auto formating will not be performed")
endif()




