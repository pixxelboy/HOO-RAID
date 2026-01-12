cmake_minimum_required(VERSION 3.16)

project(hoo-raid VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find OBS SDK
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Find libcurl
find_package(CURL REQUIRED)

# Find QtKeychain
find_package(Qt6Keychain QUIET)
if(NOT Qt6Keychain_FOUND)
    find_package(QtKeychain REQUIRED)
endif()

# nlohmann/json - header only, include directly or fetch
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Plugin sources
set(PLUGIN_SOURCES
    src/plugin.cpp
    src/models/streamer.cpp
    src/models/filter-preset.cpp
    src/models/oauth-credentials.cpp
    src/models/session-state.cpp
    src/models/greylist-entry.cpp
    src/services/async-worker.cpp
    src/services/oauth-manager.cpp
    src/services/twitch-api.cpp
    src/services/streamer-selector.cpp
    src/storage/credential-store.cpp
    src/storage/settings-manager.cpp
    src/ui/raid-picker-dock.cpp
    src/ui/preset-selector.cpp
    src/ui/streamer-preview.cpp
    src/ui/filter-editor.cpp
    src/ui/greylist-manager.cpp
)

set(PLUGIN_HEADERS
    src/plugin.h
    src/models/streamer.h
    src/models/filter-preset.h
    src/models/oauth-credentials.h
    src/models/session-state.h
    src/models/greylist-entry.h
    src/models/game.h
    src/services/async-worker.h
    src/services/oauth-manager.h
    src/services/twitch-api.h
    src/services/streamer-selector.h
    src/storage/credential-store.h
    src/storage/settings-manager.h
    src/ui/raid-picker-dock.h
    src/ui/preset-selector.h
    src/ui/streamer-preview.h
    src/ui/filter-editor.h
    src/ui/greylist-manager.h
)

# Create the plugin shared library
add_library(hoo-raid MODULE
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

target_include_directories(hoo-raid PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(hoo-raid PRIVATE
    OBS::libobs
    OBS::obs-frontend-api
    Qt6::Widgets
    Qt6::Network
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Link QtKeychain
if(Qt6Keychain_FOUND)
    target_link_libraries(hoo-raid PRIVATE Qt6Keychain::Qt6Keychain)
else()
    target_link_libraries(hoo-raid PRIVATE ${QTKEYCHAIN_LIBRARIES})
    target_include_directories(hoo-raid PRIVATE ${QTKEYCHAIN_INCLUDE_DIRS})
endif()

# Set plugin output name
set_target_properties(hoo-raid PROPERTIES
    PREFIX ""
    OUTPUT_NAME "hoo-raid"
)

# Platform-specific settings
if(WIN32)
    set_target_properties(hoo-raid PROPERTIES
        SUFFIX ".dll"
    )
elseif(APPLE)
    set_target_properties(hoo-raid PROPERTIES
        SUFFIX ".so"
    )
else()
    set_target_properties(hoo-raid PROPERTIES
        SUFFIX ".so"
    )
endif()

# Install rules
if(DEFINED ENV{OBS_PLUGIN_DIR})
    install(TARGETS hoo-raid
        LIBRARY DESTINATION $ENV{OBS_PLUGIN_DIR}
    )
endif()

# Copy i18n files
file(GLOB I18N_FILES "${CMAKE_SOURCE_DIR}/src/i18n/*.json")
foreach(I18N_FILE ${I18N_FILES})
    get_filename_component(I18N_NAME ${I18N_FILE} NAME)
    configure_file(${I18N_FILE} ${CMAKE_BINARY_DIR}/data/obs-plugins/hoo-raid/locale/${I18N_NAME} COPYONLY)
endforeach()
