cmake_minimum_required(VERSION 3.7)
project(boost_asio_lua_binding)

set(CMAKE_CXX_STANDARD 11)

include_directories(lib/lua5.3)
link_directories(lib/lua5.3)
link_libraries(lua)
link_libraries(boost_system)
link_libraries(boost_thread)

file(GLOB SOURCE_FILES
        "main.cpp"
        "src/*.cpp"
        "src/tcp/*.cpp"
        )

add_executable(boost_asio_lua_binding ${SOURCE_FILES})