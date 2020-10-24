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

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pedantic -ggdb -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -pedantic -O3 -DQT_NO_DEBUG_OUTPUT")

option(USE_PHONON "Use the Phonon based media player instead of Qt" ON)
option(USE_QWT "Use the Qwt based player widget instead of Qt" ON)
configure_file(xPlayConfig.h.in xPlayConfig.h)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Phonon4Qt5)
find_package(PkgConfig REQUIRED)

pkg_check_modules(TAGLIB taglib)
set(CMAKE_REQUIRED_INCLUDES "${TAGLIB_INCLUDE_DIRS}")
set(CMAKE_REQUIRED_LIBRARIES "${TAGLIB_LIBRARIES}")

set(QT_USE_QTMULTIMEDIA TRUE)
set(QT_USE_QTMULTIMEDIAWIDGETS TRUE)

if (USE_PHONON)
    set(XMUSICPLAYER_FILES xMusicPlayerPhonon.cpp)
else()
    set(XMUSICPLAYER_FILES xMusicPlayerQt.cpp)
endif()

if (USE_QWT)
    set(XPLAYERWIDGET_FILES xPlayerSliderWidgetQwt.cpp xPlayerVolumeWidgetQwt.cpp)
else()
    set(XPLAYERWIDGET_FILES xPlayerSliderWidgetQt.cpp xPlayerVolumeWidgetQt.cpp)
endif()

add_executable(xPlay
        xPlayConfig.h
        xMusicPlayer.cpp
        ${XMUSICPLAYER_FILES}
        xMusicLibrary.cpp
        xPlayerWidget.cpp
        xPlayerVolumeWidget.cpp
        xPlayerSliderWidget.cpp
        ${XPLAYERWIDGET_FILES}
        xMainWidget.cpp
        xApplication.cpp
        xPlay.cpp
        xPlay.qrc
)

target_include_directories(xPlay PUBLIC "${PROJECT_BINARY_DIR}")

if (USE_QWT)
    target_link_libraries(xPlay qwt-qt5 Qt5::Widgets Phonon::phonon4qt5 ${TAGLIB_LIBRARIES} )
else()
    target_link_libraries(xPlay Qt5::Widgets Phonon::phonon4qt5 ${TAGLIB_LIBRARIES})
endif()

QT5_USE_MODULES(xPlay Multimedia MultimediaWidgets)
