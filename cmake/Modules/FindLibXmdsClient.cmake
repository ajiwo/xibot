# LIBXMDSCLIENT_FOUND - true if library and headers were found
# LIBXMDSCLIENT_INCLUDE_DIRS - include directories
# LIBXMDSCLIENT_LIBRARIES - library directories

find_package(PkgConfig)
pkg_check_modules(PC_LIBXMDSCLIENT QUIET xmdsclient)

if(DEFINED ENV{LIBXMDSCLIENT_DIR})
    set(MY_LIBXMDSCLIENT_INCLUDEDIR $ENV{LIBXMDSCLIENT_DIR}/include)
    set(MY_LIBXMDSCLIENT_LIBRARYDIR $ENV{LIBXMDSCLIENT_DIR}/lib)
else()
    set(MY_LIBXMDSCLIENT_INCLUDEDIR "")
    set(MY_LIBXMDSCLIENT_LIBRARYDIR "")
endif()

find_path(LIBXMDSCLIENT_INCLUDE_DIR xmdsclient/xmds.h
	HINTS ${MY_LIBXMDSCLIENT_INCLUDEDIR} ${PC_LIBXMDSCLIENT_INCLUDEDIR} ${PC_LIBXMDSCLIENT_INCLUDE_DIRS})

find_library(LIBXMDSCLIENT_LIBRARY NAMES xmdsclient libxmdsclient
	HINTS ${MY_LIBXMDSCLIENT_LIBRARYDIR} ${PC_LIBXMDSCLIENT_LIBDIR} ${PC_LIBXMDSCLIENT_LIBRARY_DIRS})

set(LIBXMDSCLIENT_LIBRARIES ${LIBXMDSCLIENT_LIBRARY})
set(LIBXMDSCLIENT_INCLUDE_DIRS ${LIBXMDSCLIENT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LIBXMDSCLIENT DEFAULT_MSG LIBXMDSCLIENT_LIBRARY LIBXMDSCLIENT_INCLUDE_DIR)

mark_as_advanced(LIBXMDSCLIENT_INCLUDE_DIR LIBXMDSCLIENT_LIBRARY)
