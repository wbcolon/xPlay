cmake_minimum_required(VERSION 3.10)

project(xPlay)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pthread -pedantic -Wno-deprecated-declarations -ggdb -fno-omit-frame-pointer -DXPLAY_APP_NAME=\\\"xPlayDebug\\\"")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pedantic -Wno-deprecated-declarations -ggdb -fno-omit-frame-pointer -fsanitize=address -DXPLAY_APP_NAME=\\\"xPlayDebug\\\"")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -pedantic -pthread -Wno-deprecated-declarations -O3 -DQT_NO_DEBUG_OUTPUT -ggdb -DXPLAY_APP_NAME=\\\"xPlay\\\"")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

option(USE_STREAMING "Enable the streaming view in xPlay" ON)
option(USE_TESTS "Build the tests for xPlay instead of the application" OFF)
configure_file(xPlayerConfig.h.in xPlayerConfig.h)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Phonon4Qt5)
find_package(SQLite3)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(TAGLIB taglib)
set(CMAKE_REQUIRED_INCLUDES "${TAGLIB_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_LIBRARIES "${TAGLIB_LIBRARIES}")

pkg_check_modules(PROJECTM libprojectM)
set(CMAKE_REQUIRED_INCLUDES "${PROJECTM_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_LIBRARIES "${PROJECTM_LIBRARIES}")

pkg_check_modules(VLCLIB libvlc)
set(CMAKE_REQUIRED_INCLUDES "${VLCLIB_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_LIBRARIES "${VLCLIB_LIBRARIES}")

set(QT_USE_QTMULTIMEDIA TRUE)
set(QT_USE_QTMULTIMEDIAWIDGETS TRUE)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${SQLITE3_INCLUDE_DIRS})

set(xPlay_sources
        xMusicLibraryEntry.cpp
        xMusicLibraryArtistEntry.cpp
        xMusicLibraryAlbumEntry.cpp
        xMusicLibraryTrackEntry.cpp
        xMusicLibrary.cpp
        xMusicLibraryFilter.cpp
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
        xPlayerListWidget.cpp
        xPlayerMusicSearchWidget.cpp
        xPlayerMusicWidget.cpp
        xPlayerVolumeWidget.cpp
        xPlayerSliderWidget.cpp
        xPlayerBalanceWidget.cpp
        xPlayerControlButtonWidget.cpp
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
        xPlayerDBus.cpp
        )

set(xPlay_libraries
        Qt5::WebEngineWidgets Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets Qt5::DBus
        qwt-qt5
        Phonon::phonon4qt5
        sqlite3
        ${TAGLIB_LIBRARIES}
        ${PROJECTM_LIBRARIES}
        OpenGL::GL
        ${VLCLIB_LIBRARIES})

if (USE_TESTS)
    add_executable(test_xPlay
            ${xPlay_sources}
            tests/test_xMusicLibraryTrackName.cpp
            tests/test_xMusicLibraryEntry.cpp
            tests/test_xMusicLibrary.cpp
            tests/test_xMovieLibrary.cpp
            tests/test_xPlay.cpp)
    target_link_libraries(test_xPlay Qt5::Test ${xPlay_libraries})
else()
    add_executable(xPlay
            ${xPlay_sources}
            xPlay.cpp
            xPlay.qrc)
    target_link_libraries(xPlay ${xPlay_libraries})
    target_include_directories(xPlay PUBLIC "${PROJECT_BINARY_DIR}")
endif()

