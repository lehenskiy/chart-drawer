cmake_minimum_required(VERSION 3.25)
project(chart_drawer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Sql
        Charts
        REQUIRED)

add_executable(chart_drawer
        main.cpp
        ChartDrawer.h
        DataExtractor.h
        IOCContainer.h
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)
target_link_libraries(chart_drawer
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
        Qt5::Charts
)

