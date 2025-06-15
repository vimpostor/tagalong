cmake_minimum_required(VERSION 4.0)
project(tag-along VERSION 1.0 DESCRIPTION "Sheet music viewer")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND QT_MODULES Core Qml Quick QuickControls2 Svg Pdf Network Xml)
find_package(Qt6 6.9 COMPONENTS ${QT_MODULES} REQUIRED)
qt_standard_project_setup(REQUIRES ${Qt6_VERSION})
list(TRANSFORM QT_MODULES PREPEND "Qt6::")

list(APPEND LINK_LIBS ${QT_MODULES})

include(FetchContent)
FetchContent_Declare(quartz GIT_REPOSITORY https://github.com/vimpostor/quartz.git GIT_TAG 4f083b619025638ef871cdc2eb1b97cb9ca448ec)
FetchContent_MakeAvailable(quartz)

include_directories("src")

file(GLOB_RECURSE SRCS "src/*.cpp")
file(GLOB_RECURSE HDRS "src/*.hpp")
file(GLOB_RECURSE QMLS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/qml/*.qml")

qt_add_resources(RESOURCES "${CMAKE_SOURCE_DIR}/icons.qrc")

qt_add_executable(${PROJECT_NAME} ${SRCS} ${RESOURCES})

qt_add_qml_module(${PROJECT_NAME} URI "Backend" VERSION "${PROJECT_VERSION}" QML_FILES ${QMLS})
set_target_properties(${PROJECT_NAME} PROPERTIES QT_QMLCACHEGEN_ARGUMENTS "--only-bytecode")

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})
quartz_link(${PROJECT_NAME})

# install
install(TARGETS ${PROJECT_NAME} RUNTIME)
