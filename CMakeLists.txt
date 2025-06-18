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
if(ANDROID)
	# openssl
	FetchContent_Declare(android_openssl URL https://github.com/kdab/android_openssl/archive/refs/heads/master.zip)
	FetchContent_MakeAvailable(android_openssl)
	include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
	add_android_openssl_libraries(${PROJECT_NAME})

	# generate icons
	list(APPEND ICON_COMMANDS COMMAND "sh" "-c" "mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-\\{l,m,h,xh,xxh,xxxh\\}dpi")
	list(APPEND ICON_COMMANDS COMMAND "magick" "${CMAKE_SOURCE_DIR}/assets/${PROJECT_NAME}.svg" "-resize" "36x36" "${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-ldpi/${PROJECT_NAME}.png")
	list(APPEND ICON_COMMANDS COMMAND "magick" "${CMAKE_SOURCE_DIR}/assets/${PROJECT_NAME}.svg" "-resize" "48x48" "${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-mdpi/${PROJECT_NAME}.png")
	list(APPEND ICON_COMMANDS COMMAND "magick" "${CMAKE_SOURCE_DIR}/assets/${PROJECT_NAME}.svg" "-resize" "72x72" "${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-hdpi/${PROJECT_NAME}.png")
	list(APPEND ICON_COMMANDS COMMAND "magick" "${CMAKE_SOURCE_DIR}/assets/${PROJECT_NAME}.svg" "-resize" "96x96" "${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xhdpi/${PROJECT_NAME}.png")
	list(APPEND ICON_COMMANDS COMMAND "magick" "${CMAKE_SOURCE_DIR}/assets/${PROJECT_NAME}.svg" "-resize" "144x144" "${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxhdpi/${PROJECT_NAME}.png")
	list(APPEND ICON_COMMANDS COMMAND "magick" "${CMAKE_SOURCE_DIR}/assets/${PROJECT_NAME}.svg" "-resize" "192x192" "${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxxhdpi/${PROJECT_NAME}.png")
	add_custom_target(android-integration ${ICON_COMMANDS})

	string(REPLACE "." "" VERSION_CODE "${PROJECT_VERSION}")
	set_target_properties(${PROJECT_NAME} PROPERTIES QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android" QT_ANDROID_PACKAGE_NAME "com.mggross.${PROJECT_NAME}" QT_ANDROID_APP_ICON "@drawable/${PROJECT_NAME}" QT_ANDROID_APP_NAME "Tag Along" QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}" QT_ANDROID_VERSION_CODE ${VERSION_CODE})
else()
	install(TARGETS ${PROJECT_NAME} RUNTIME)
endif()
