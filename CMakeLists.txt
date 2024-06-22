cmake_minimum_required(VERSION 3.10)

project(xPlay)

# Default build is Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default flags for all builds.
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread -pedantic -Wno-deprecated-declarations")
# Additional flags for 'Debug' build.
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -fno-omit-frame-pointer -DXPLAY_APP_NAME=\\\"xPlayDebug\\\"")
# Additional flags for 'Release' build.
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DQT_NO_DEBUG_OUTPUT -DXPLAY_APP_NAME=\\\"xPlay\\\"")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

option(USE_STREAMING "Enable the streaming view in xPlay" ON)
option(USE_TESTS "Build the tests for xPlay instead of the application" OFF)
configure_file(xPlayerConfig.h.in xPlayerConfig.h)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(Qt6 COMPONENTS Widgets DBus REQUIRED)
find_package(Qt6Multimedia REQUIRED)
find_package(Qt6MultimediaWidgets REQUIRED)
find_package(Qt6OpenGL REQUIRED)
find_package(Qt6OpenGLWidgets REQUIRED)
find_package(Qt6WebEngineWidgets REQUIRED)
find_package(Qt6Test REQUIRED)
find_package(Phonon4Qt6)
find_package(SQLite3)
find_package(CURL)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(PUGIXML pugixml REQUIRED)
set(CMAKE_REQUIRED_INCLUDES "${PUGIXML_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_CFLAGS "${PUGIXML_CFLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${PUGIXML_LIBRARIES}")
pkg_check_modules(TAGLIB taglib REQUIRED)

set(CMAKE_REQUIRED_INCLUDES "${TAGLIB_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_CFLAGS "${TAGLIB_CFLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${TAGLIB_LIBRARIES}")

pkg_check_modules(PROJECTM libprojectM REQUIRED)
set(CMAKE_REQUIRED_INCLUDES "${PROJECTM_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_CFLAGS "${PROJECTM_CFLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${PROJECTM_LIBRARIES}")

pkg_check_modules(VLCLIB libvlc REQUIRED)
set(CMAKE_REQUIRED_INCLUDES "${VLCLIB_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_CFLAGS "${VLCLIB_CFLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${VLCLIB_LIBRARIES}")

pkg_check_modules(PALIB libpulse REQUIRED)
set(CMAKE_REQUIRED_INCLUDES "${PALIB_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_CFLAGS "${PALIB_CFLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${PALIB_LIBRARIES}")

set(QT_USE_QTMULTIMEDIA TRUE)
set(QT_USE_QTMULTIMEDIAWIDGETS TRUE)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${SQLITE3_INCLUDE_DIRS})

add_definitions(${PUGIXML_CFLAGS})
add_definitions(${TAGLIB_CFLAGS})
add_definitions(${PROJECTM_CFLAGS})
add_definitions(${VLCLIB_CFLAGS})
add_definitions(${PALIB_CFLAGS})

set(xPlay_sources
        xMusicLibraryEntry.cpp
        xMusicLibraryArtistEntry.cpp
        xMusicLibraryAlbumEntry.cpp
        xMusicLibraryTrackEntry.cpp
        xMusicLibrary.cpp
        xMusicLibraryFilter.cpp
        xMovieLibraryEntry.cpp
        xMovieLibrary.cpp
        xPlayerConfig.h.in
        xPlayerUI.cpp
        xPlayerConfiguration.cpp
        xPlayerDatabase.cpp
        xMusicPlayer.cpp
        xMoviePlayer.cpp
        xPlayerArtistInfo.cpp
        xPlayerConfigurationDialog.cpp
        xPlayerPlaylistDialog.cpp
        xPlayerTagsDialog.cpp
        xPlayerListWidgetItem.cpp
        xPlayerListWidget.cpp
        xPlayerMusicSearchWidget.cpp
        xPlayerMusicWidget.cpp
        xPlayerVolumeWidget.cpp
        xPlayerSliderWidget.cpp
        xPlayerBalanceWidget.cpp
        xPlayerControlButtonWidget.cpp
        xPlayerPulseAudioControls.cpp
        xPlayerBluOSControl.cpp
        xPlayerRotelControls.cpp
        xPlayerRotelWidget.cpp
        xPlayerMovieWidget.cpp
        xPlayerMusicArtistSelectorWidget.cpp
        xPlayerMusicAlbumSelectorWidget.cpp
        xPlayerMusicLibraryWidget.cpp
        xPlayerVisualizationWidget.cpp
        xMainMusicWidget.cpp
        xMainMovieWidget.cpp
        xMainStreamingWidget.cpp
        xMainMobileSyncWidget.cpp
        xApplication.cpp
        xPlayerDBus.cpp)

set(xPlay_libraries
        Qt6::WebEngineWidgets Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets Qt6::OpenGLWidgets Qt6::DBus
        Phonon::phonon4qt6
        sqlite3
        ${TAGLIB_LIBRARIES}
        ${PROJECTM_LIBRARIES}
        OpenGL::GL
        curl
        ${PUGIXML_LIBRARIES}
        ${PALIB_LIBRARIES}
        ${VLCLIB_LIBRARIES})

if (USE_TESTS)
    add_executable(test_xPlay
            ${xPlay_sources}
            tests/test_xMusicLibraryTrackName.cpp
            tests/test_xMusicLibraryEntry.cpp
            tests/test_xMusicLibrary.cpp
            tests/test_xMovieLibrary.cpp
            tests/test_xPlayerRotelControls.cpp
            tests/test_xPlay.cpp)
    target_link_libraries(test_xPlay Qt5::Test ${xPlay_libraries})
else()
    add_executable(xPlay
            ${xPlay_sources}
            xPlay.cpp
            xPlay.qrc)
    target_link_libraries(xPlay ${xPlay_libraries})
    target_include_directories(xPlay PUBLIC "${PROJECT_BINARY_DIR}")
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

