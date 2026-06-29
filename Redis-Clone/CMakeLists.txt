file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(Redis-Clone ${SOURCES}
        src/section_3/client.cpp
)

target_include_directories(Redis-Clone PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)