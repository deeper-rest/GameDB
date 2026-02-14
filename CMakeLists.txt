cmake_minimum_required(VERSION 3.16)

project(GameDB VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui Charts SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui Charts SerialPort)

set(PROJECT_SOURCES
        main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(GameDB
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        
        libs/mainwindow.h
        libs/filelisttab.h
        libs/gamedata.h
        libs/gamescanner.h
        libs/gamemanager.h
        libs/gameinfodialog.h
        libs/gamelisttab.h
        libs/tagmanager.h
        libs/tagmanagerdialog.h
        libs/thumbnailmanager.h
        libs/gamedetailwidget.h

        src/mainwindow.cpp
        src/filelisttab.cpp
        src/gamescanner.cpp
        src/gamemanager.cpp
        src/gameinfodialog.cpp
        src/gamelisttab.cpp
        src/tagmanager.cpp
        src/tagmanagerdialog.cpp
        src/thumbnailmanager.cpp
        src/gamedetailwidget.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET GameDB APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(GameDB SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(GameDB
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(GameDB PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::SerialPort)
target_include_directories(GameDB PUBLIC libs ./)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.GameDB)
endif()
set_target_properties(GameDB PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    # WIN32_EXECUTABLE ON
)

include(GNUInstallDirs)
install(TARGETS GameDB
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(GameDB)
endif()
