cmake_minimum_required(VERSION 3.10)

project(xPlay)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pthread -pedantic -Wno-deprecated-declarations -ggdb -fno-omit-frame-pointer -DXPLAY_APP_NAME=\\\"xPlayDebug\\\"")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pedantic -Wno-deprecated-declarations -ggdb -fno-omit-frame-pointer -fsanitize=address -DXPLAY_APP_NAME=\\\"xPlayDebug\\\"")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -pedantic -pthread -Wno-deprecated-declarations -O3 -DQT_NO_DEBUG_OUTPUT -ggdb -DXPLAY_APP_NAME=\\\"xPlay\\\"")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

option(USE_PHONON "Use the Phonon based media player instead of Qt" ON)
option(USE_VLC "Use the VLC based movie media player instead of Qt" ON)
option(USE_QWT "Use the Qwt based player widget instead of Qt" ON)
option(USE_STREAMING "Enable the streaming view in xPlay" ON)
configure_file(xPlayerConfig.h.in xPlayerConfig.h)

find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Phonon4Qt5)
find_package(Soci)
find_package(PkgConfig REQUIRED)

pkg_check_modules(TAGLIB taglib)
set(CMAKE_REQUIRED_INCLUDES "${TAGLIB_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_LIBRARIES "${TAGLIB_LIBRARIES}")

set(QT_USE_QTMULTIMEDIA TRUE)
set(QT_USE_QTMULTIMEDIAWIDGETS TRUE)

if (USE_PHONON)
    set(XMUSICPLAYERX musicplayer/xMusicPlayerPhonon.cpp)
else()
    set(XMUSICPLAYERX musicplayer/xMusicPlayerQt.cpp)
endif()

if (USE_VLC)
    pkg_check_modules(VLCLIB libvlc)
    set(CMAKE_REQUIRED_INCLUDES "${VLCLIB_INCLUDE_DIRS}")
    set(CMAKE_REQUIRED_LIBRARIES "${VLCLIB_LIBRARIES}")

    set(XMOVIEPLAYERX movieplayer/xMoviePlayerVLC.cpp)
else()
    set(XMOVIEPLAYERX movieplayer/xMoviePlayerPhonon.cpp)
endif()

if (USE_QWT)
    set(XPLAYERWIDGETX playerwidget/xPlayerSliderWidgetQwt.cpp playerwidget/xPlayerVolumeWidgetQwt.cpp playerwidget/xPlayerBalanceWidgetQwt.cpp)
else()
    set(XPLAYERWIDGETX playerwidget/xPlayerSliderWidgetQt.cpp playerwidget/xPlayerVolumeWidgetQt.cpp playerwidget/xPlayerBalanceWidgetQt.cpp)
endif()

include_directories(${PROJECT_SOURCE_DIR} ${SOCI_INCLUDE_DIRS})

add_executable(xPlay
        xPlayerConfig.h
        xPlayerUI.cpp
        xPlayerConfiguration.cpp
        xPlayerDatabase.cpp
        xMusicLibrary.cpp
        xMovieLibrary.cpp
        xMusicPlayer.cpp
        xMusicFile.cpp
        ${XMOVIEPLAYERX}
        ${XMUSICPLAYERX}
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
        ${XPLAYERWIDGETX}
        xPlayerRotelWidget.cpp
        xPlayerMovieWidget.cpp
        xPlayerMusicAlbumSelectorWidget.cpp
        xMainMusicWidget.cpp
        xMainMovieWidget.cpp
        xMainStreamingWidget.cpp
        xApplication.cpp
        xPlayerDBus.cpp
        xPlay.cpp
        xPlay.qrc
)

target_include_directories(xPlay PUBLIC "${PROJECT_BINARY_DIR}")

if (USE_QWT)
    target_link_libraries(xPlay Qt5::WebEngineWidgets qwt-qt5 Qt5::Widgets Qt5::DBus Phonon::phonon4qt5 ${TAGLIB_LIBRARIES} ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${VLCLIB_LIBRARIES})
else()
    target_link_libraries(xPlay Qt5::WebEngineWidgets Qt5::Widgets Qt5::DBus Phonon::phonon4qt5 ${TAGLIB_LIBRARIES} ${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN} ${VLCLIB_LIBRARIES})
endif()

QT5_USE_MODULES(xPlay Multimedia MultimediaWidgets)
