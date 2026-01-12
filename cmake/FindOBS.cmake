# FindOBS.cmake - Find OBS Studio SDK
#
# This module defines:
#   libobs_FOUND - System has libobs
#   obs-frontend-api_FOUND - System has obs-frontend-api
#   OBS::libobs - Imported target for libobs
#   OBS::obs-frontend-api - Imported target for obs-frontend-api

# Try to find OBS via pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LIBOBS QUIET libobs)
    pkg_check_modules(PC_OBS_FRONTEND QUIET obs-frontend-api)
endif()

# Find libobs
find_path(LIBOBS_INCLUDE_DIR
    NAMES obs-module.h obs.h
    PATHS
        ${PC_LIBOBS_INCLUDE_DIRS}
        /usr/include/obs
        /usr/local/include/obs
        /opt/homebrew/include/obs
        "$ENV{OBS_SDK_PATH}/include"
        "$ENV{OBS_SDK_PATH}/libobs"
    PATH_SUFFIXES
        obs libobs
)

find_library(LIBOBS_LIBRARY
    NAMES obs libobs
    PATHS
        ${PC_LIBOBS_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/homebrew/lib
        "$ENV{OBS_SDK_PATH}/build/libobs"
        "$ENV{OBS_SDK_PATH}/bin/64bit"
    PATH_SUFFIXES
        obs-plugins obs-studio
)

# Find obs-frontend-api
find_path(OBS_FRONTEND_INCLUDE_DIR
    NAMES obs-frontend-api.h
    PATHS
        ${PC_OBS_FRONTEND_INCLUDE_DIRS}
        /usr/include/obs
        /usr/local/include/obs
        /opt/homebrew/include/obs
        "$ENV{OBS_SDK_PATH}/include"
        "$ENV{OBS_SDK_PATH}/UI/obs-frontend-api"
    PATH_SUFFIXES
        obs obs-frontend-api
)

find_library(OBS_FRONTEND_LIBRARY
    NAMES obs-frontend-api libobs-frontend-api
    PATHS
        ${PC_OBS_FRONTEND_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/homebrew/lib
        "$ENV{OBS_SDK_PATH}/build/UI/obs-frontend-api"
        "$ENV{OBS_SDK_PATH}/bin/64bit"
    PATH_SUFFIXES
        obs-plugins obs-studio
)

# Handle standard find_package arguments
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libobs
    REQUIRED_VARS LIBOBS_LIBRARY LIBOBS_INCLUDE_DIR
)

find_package_handle_standard_args(obs-frontend-api
    REQUIRED_VARS OBS_FRONTEND_LIBRARY OBS_FRONTEND_INCLUDE_DIR
)

# Create imported targets
if(libobs_FOUND AND NOT TARGET OBS::libobs)
    add_library(OBS::libobs UNKNOWN IMPORTED)
    set_target_properties(OBS::libobs PROPERTIES
        IMPORTED_LOCATION "${LIBOBS_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBOBS_INCLUDE_DIR}"
    )
endif()

if(obs-frontend-api_FOUND AND NOT TARGET OBS::obs-frontend-api)
    add_library(OBS::obs-frontend-api UNKNOWN IMPORTED)
    set_target_properties(OBS::obs-frontend-api PROPERTIES
        IMPORTED_LOCATION "${OBS_FRONTEND_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OBS_FRONTEND_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(
    LIBOBS_INCLUDE_DIR
    LIBOBS_LIBRARY
    OBS_FRONTEND_INCLUDE_DIR
    OBS_FRONTEND_LIBRARY
)
