cmake_minimum_required(VERSION 3.16)

project(Inventorymanagmentsystem VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SQL module here
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)

qt_add_executable(Inventorymanagmentsystem
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        admin_page.h admin_page.cpp admin_page.ui
        user_page.h user_page.cpp user_page.ui
        backend_classes.h
        backend_classes.cpp
        settingspage.h settingspage.cpp settingspage.ui
        checkout_page.h checkout_page.cpp checkout_page.ui
        database_manager.h
        database_manager.cpp
    )
else()
    if(ANDROID)
        add_library(Inventorymanagmentsystem SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Inventorymanagmentsystem
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Add SQL linking here
target_link_libraries(Inventorymanagmentsystem PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

# Bundle settings
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Inventorymanagmentsystem)
endif()

set_target_properties(Inventorymanagmentsystem PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Inventorymanagmentsystem
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Inventorymanagmentsystem)
endif()
