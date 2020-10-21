cmake_minimum_required(VERSION 3.1.0)

project(xPlay)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -pedantic -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -pedantic -O3 -DQT_NO_DEBUG_OUTPUT")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Phonon4Qt5)

set(QT_USE_QTMULTIMEDIA TRUE)
set(QT_USE_QTMULTIMEDIAWIDGETS TRUE)

add_executable(xPlay
        xMusicPlayer.cpp
        xMusicPlayerQt.cpp
        xMusicPlayerPhonon.cpp
        xMusicLibrary.cpp
        xPlayerWidget.cpp
        xMainWidget.cpp
        xApplication.cpp
        xPlay.cpp
        xPlay.qrc
)

target_link_libraries(xPlay Qt5::Widgets Phonon::phonon4qt5)

QT5_USE_MODULES(xPlay Multimedia MultimediaWidgets)
