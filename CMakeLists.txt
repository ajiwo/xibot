cmake_minimum_required (VERSION 2.8.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(xibot)

find_package(LibXlfParser REQUIRED)
find_package(LibXmdsClient REQUIRED)

set(srcs
    src/player.c
    src/xmdsclient.c
    src/simple-time.c
    src/thrlck.c
    src/xibot.c
    src/state.c
    src/xignal.c
)

set(install_headers
    src/attrs.h
    src/player.h
    src/thrlck.h
    src/xibot.h
    src/xignal.h
)

add_library(xibot SHARED ${srcs})
target_link_libraries(xibot LINK_PUBLIC xmdsclient ${LIBXLFPARSER_LIBRARIES} ${LIBXMDSCLIENT_LIBRARIES} "-pthread")
set_target_properties(xibot PROPERTIES COMPILE_FLAGS "-Wall -pedantic -pedantic-errors -pthread")

add_executable(xibotcli example/main.c)
target_link_libraries(xibotcli LINK_PUBLIC xibot "-pthread")
set_target_properties(xibotcli PROPERTIES COMPILE_FLAGS "-Wall -ansi -pedantic -pedantic-errors  -pthread")

install(
    TARGETS xibot
    DESTINATION lib
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
)

install(
    FILES ${install_headers}
    DESTINATION include/xibot
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
)
