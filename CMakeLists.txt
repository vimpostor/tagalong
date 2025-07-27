cmake_minimum_required(VERSION 4.0)
project(tagalong VERSION 1.0 DESCRIPTION "Sheet music viewer")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND QT_MODULES Core Qml Quick QuickControls2 Svg Pdf Network Sql)
find_package(Qt6 6.9 COMPONENTS ${QT_MODULES} REQUIRED)
qt_standard_project_setup(REQUIRES ${Qt6_VERSION})
list(TRANSFORM QT_MODULES PREPEND "Qt6::")

list(APPEND LINK_LIBS ${QT_MODULES})

include(FetchContent)
FetchContent_Declare(quartz GIT_REPOSITORY https://github.com/vimpostor/quartz.git GIT_TAG e912ced5880d959231e25b3aaf4c2079c0ae560d)
FetchContent_MakeAvailable(quartz)

include_directories("src" "src/barbershop")

file(GLOB_RECURSE SRCS "src/*.cpp")
file(GLOB_RECURSE HDRS "src/*.hpp")
file(GLOB_RECURSE QMLS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/qml/*.qml")
if(ANDROID)
	list(APPEND SRCS "android/src/com/mggross/tagalong/QtIntentActivity.java")
endif()

qt_add_resources(RESOURCES "${CMAKE_SOURCE_DIR}/icons.qrc")

qt_add_executable(${PROJECT_NAME} ${SRCS} ${RESOURCES})

qt_add_qml_module(${PROJECT_NAME} URI "Backend" VERSION "${PROJECT_VERSION}" QML_FILES ${QMLS} SOURCES "src/barbershop/tagcompletionsmodel.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})
quartz_link(${PROJECT_NAME})

# install
quartz_generate_launcher_icons(${PROJECT_NAME})
if(ANDROID)
	# openssl
	quartz_bundle_android_openssl(${PROJECT_NAME})

	string(REPLACE "." "" VERSION_CODE "${PROJECT_VERSION}")
	set_target_properties(${PROJECT_NAME} PROPERTIES QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android" QT_ANDROID_PACKAGE_NAME "com.mggross.${PROJECT_NAME}" QT_ANDROID_APP_ICON "@drawable/${PROJECT_NAME}" QT_ANDROID_APP_NAME "Tag Along" QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}" QT_ANDROID_VERSION_CODE ${VERSION_CODE})
else()
	install(TARGETS ${PROJECT_NAME} RUNTIME)
	install(FILES "assets/${PROJECT_NAME}.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
endif()
