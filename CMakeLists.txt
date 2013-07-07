cmake_minimum_required(VERSION 2.8.9)

project(libstatsd)

include_directories("src/")
include_directories("include/")

file(GLOB_RECURSE
    source_files
    src/*
)

enable_testing()

add_executable(
    statsd-test
    test/statsd-test.c
    ${source_files}
)

add_test(Libstatsd statsd-test)