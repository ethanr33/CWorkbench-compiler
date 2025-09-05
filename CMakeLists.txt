cmake_minimum_required(VERSION 3.31)
project(cpp-compiler)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(MAIN
    main.cpp
)

set(SOURCES
    tools/debug.cpp
    tools/memory.cpp
    lexer/Lexer.cpp 
    lexer/Token.cpp
    parser/CFG.cpp
    parser/AST.cpp
    parser/Parser.cpp
    codegen/AssemblyGenerator.cpp
)

set(SOURCES_TEST
    ${SOURCES}
    tests/test_cfg.cc
)

add_executable(compiler ${MAIN} ${SOURCES})

add_executable(tests ${SOURCES_TEST})
target_link_libraries(tests gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

add_test(NAME unit_tests COMMAND tests)