cmake_minimum_required(VERSION 3.20)
project(inkwell VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Widgets
    Svg
    Sql
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Stroke.cpp
    src/core/Page.cpp
    src/core/Notebook.cpp
    src/core/UndoCommands.cpp
    src/canvas/InkCanvas.cpp
    src/canvas/InputHandler.cpp
    src/canvas/StrokeRenderer.cpp
    src/storage/Database.cpp
    src/library/NotebookManager.cpp
    src/library/LibraryModel.cpp
    src/ui/ThemeManager.cpp
)

set(HEADERS
    src/core/Types.h
    src/core/Stroke.h
    src/core/Page.h
    src/core/Notebook.h
    src/core/UndoCommands.h
    src/canvas/InkCanvas.h
    src/canvas/InputHandler.h
    src/canvas/StrokeRenderer.h
    src/storage/Database.h
    src/library/NotebookManager.h
    src/library/LibraryModel.h
    src/ui/ThemeManager.h
)

# Add application executable
qt6_add_executable(inkwell
    ${SOURCES}
    ${HEADERS}
    resources.qrc
)

target_include_directories(inkwell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(inkwell PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Svg
    Qt6::Sql
)

# Optional Poppler-Qt6 support if available
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(POPPLER_QT6 poppler-qt6)
    if (POPPLER_QT6_FOUND)
        target_include_directories(inkwell PRIVATE ${POPPLER_QT6_INCLUDE_DIRS})
        target_link_libraries(inkwell PRIVATE ${POPPLER_QT6_LIBRARIES})
        target_compile_definitions(inkwell PRIVATE HAVE_POPPLER_QT6=1)
    endif()
endif()

# Installation rules for Fedora Linux
include(GNUInstallDirs)
install(TARGETS inkwell DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/inkwell.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES resources/icons/inkwell.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
