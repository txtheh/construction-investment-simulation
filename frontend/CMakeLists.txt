cmake_minimum_required(VERSION 3.21)
project(MegapolisFrontend VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(MegapolisFrontend
    main.cpp
    MainWindow.h
    MainWindow.cpp
    CityMapWidget.h
    CityMapWidget.cpp
    GameTypes.h
    GameSetupDialog.h
    GameSetupDialog.cpp
    ResultsDialog.h
    ResultsDialog.cpp
)

target_link_libraries(MegapolisFrontend PRIVATE 
    Qt6::Widgets
    backend
)

set_target_properties(MegapolisFrontend PROPERTIES MACOSX_BUNDLE TRUE)

target_compile_options(MegapolisFrontend PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
)
