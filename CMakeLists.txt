cmake_minimum_required(VERSION 3.0.2)

project(SilkEdit VERSION 0.13.1)
set(BUILD_NUMBER $ENV{SILK_BUILD_NUMBER})

include(${PROJECT_SOURCE_DIR}/conanbuildinfo.cmake)

if(NOT DEFINED QTDIR AND DEFINED ENV{QTDIR})
  set(QTDIR $ENV{QTDIR})
endif()

if(DEFINED QTDIR)
  message(STATUS "QTDIR: ${QTDIR}")
else()
  message( SEND_ERROR "QTDIR is not defined" )
endif()

message(STATUS "BUILD_NUMBER: ${BUILD_NUMBER}")

option (BUILD_EDGE "Build edge version" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (BUILD_EDGE)
  ADD_DEFINITIONS(-DBUILD_EDGE)
endif ()

# Node.js defines HAVE_OPENSSL by default
ADD_DEFINITIONS(-DHAVE_OPENSSL)

# '+' will automatically be performed as the QStringBuilder '%' everywhere.
ADD_DEFINITIONS(-DQT_USE_QSTRINGBUILDER)

# These are necessary to import v8 and libuv symbols in node.dll
ADD_DEFINITIONS(-DUSING_V8_SHARED)
ADD_DEFINITIONS(-DUSING_UV_SHARED)

if (APPLE)
  ADD_DEFINITIONS(-D__POSIX__)
endif ()

file(GLOB_RECURSE SILK_SOURCES src/*.cpp)
file(GLOB_RECURSE SILK_HEADERS src/*.h)
file(GLOB_RECURSE SILK_QMLS resources/qmls/*.qml)

set(SILK_RESOURCES resources/style.qrc resources/images_common.qrc resources/files.qrc resources/qmls.qrc)

if (BUILD_EDGE)
  set(SILK_RESOURCES ${SILK_RESOURCES} resources/images_edge.qrc)
else ()
  set(SILK_RESOURCES ${SILK_RESOURCES} resources/images.qrc)
endif ()

# bundle fonts on Windows
if (MSVC)
  file(GLOB_RECURSE FONTS_RESOURCES resources/fonts/*.qrc)
  set(SILK_RESOURCES ${SILK_RESOURCES} ${FONTS_RESOURCES})
endif()

file(GLOB SILK_TRANSLATIONS resources/translations/*.ts)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)
find_package(Boost REQUIRED)

  find_library(NODE_LIBRARY NAMES ${CONAN_LIBS_NODE} PATHS ${CONAN_LIB_DIRS_NODE} NO_DEFAULT_PATH)
  if (NODE_LIBRARY)
    message(STATUS "NODE_LIBRARY: ${NODE_LIBRARY}")
  endif()
  if (NOT NODE_LIBRARY)
    message(FATAL_ERROR "NODE_LIBRARY not found")
  endif()

  find_library(YAML_CPP NAMES ${CONAN_LIBS_YAML-CPP} PATHS ${CONAN_LIB_DIRS_YAML-CPP})
  if (YAML_CPP)
    message(STATUS "YAML_CPP: ${YAML_CPP}")
  endif()
  if (NOT YAML_CPP)
    message(FATAL_ERROR "YAML_CPP not found")
  endif()

if (APPLE)
  set( BREAKPAD_LIBS "vendor/breakpad/src/client/mac/build/${CMAKE_BUILD_TYPE}")
  find_library(Breakpad Breakpad PATHS ${BREAKPAD_LIBS})
  if (Breakpad)
    message(STATUS "Breakpad: ${Breakpad}")
  endif()
  if (NOT Breakpad)
    message(FATAL_ERROR "Breakpad not found")
  endif()

  set( CARTHAGE_PATH "Carthage/Build/Mac")

  find_library(Squirrel Squirrel PATHS ${CARTHAGE_PATH})
  if (Squirrel)
    message(STATUS "Squirrel: ${Squirrel}")
  endif()
  if (NOT Squirrel)
    message(FATAL_ERROR "Squirrel not found")
  endif()

  find_library(Mantle Mantle PATHS ${CARTHAGE_PATH})
  if (Mantle)
    message(STATUS "Mantle: ${Mantle}")
  endif()
  if (NOT Mantle)
    message(FATAL_ERROR "Mantle not found")
  endif()

  find_library(ReactiveCocoa ReactiveCocoa PATHS ${CARTHAGE_PATH})
  if (ReactiveCocoa)
    message(STATUS "ReactiveCocoa: ${ReactiveCocoa}")
  endif()
  if (NOT ReactiveCocoa)
    message(FATAL_ERROR "ReactiveCocoa not found")
  endif()

endif ()

qt5_add_translation(QM_FILES ${SILK_TRANSLATIONS})

# configure a header file to pass some of the CMake settings to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/version.h.in"
  "${PROJECT_BINARY_DIR}/src/version.h"
)

if (APPLE)
  # icon files to copy in the bundle
  set( OSX_ICON_FILES resources/silkedit.icns )

  if (BUILD_EDGE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME} Edge")
  else()
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
  endif()
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "io.silkedit.${PROJECT_NAME}")
  set(MACOSX_BUNDLE_ICON_FILE silkedit.icns)
  set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set_source_files_properties( ${QM_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/translations)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" )
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
  set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2014-2016 SilkEdit")

  # include the icns files in the target
  set( SILK_SOURCES ${SILK_SOURCES} ${OSX_ICON_FILES} )

  # -Wno-gnu-zero-variadic-macro-arguments: to suppress warnings of qCDebug(category, ...)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -Wno-gnu-zero-variadic-macro-arguments -g")
  include_directories(SYSTEM
    # for version.h generated by CMake
    ${PROJECT_BINARY_DIR}/src
    /usr/local/include
    src
    widgets/src
    vendor/breakpad/src
    ${CONAN_INCLUDE_DIRS_NODE}
    ${CONAN_INCLUDE_DIRS_YAML-CPP}
    )

  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS} ${SILK_RESOURCES} ${QM_FILES} ${SILK_QMLS})

  # <OUTPUT_NAME>.app will be generated
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MACOSX_BUNDLE_BUNDLE_NAME})
elseif (MSVC)
  string(REGEX REPLACE "/Z[iI7]" ""
         CMAKE_CXX_FLAGS_RELEASE
         "${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  string(REGEX REPLACE "/INCREMENTAL:NO" ""
         CMAKE_EXE_LINKER_FLAGS_RELEASE
         "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug /OPT:REF")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS /source-charset:utf-8 /execution-charset:utf-8")

  #breakpad libs under 'lib' folder
  set( BREAKPAD_LIBS "breakpad/${CMAKE_BUILD_TYPE}")

  # SYSTEM tells the compiler to skip warning for these header files.
  include_directories(SYSTEM
    ${PROJECT_BINARY_DIR}/src
    include
    src
    widgets/src
    ${Boost_INCLUDE_DIRS}
    vendor/breakpad/src
    ${CONAN_INCLUDE_DIRS_NODE}
    ${CONAN_INCLUDE_DIRS_YAML-CPP}
  )
  link_directories(${PROJECT_SOURCE_DIR}/lib)

  if (BUILD_EDGE)
    set( SILK_ICON resources/silkedit_edge.rc)
  else ()
    set( SILK_ICON resources/silkedit.rc)
  endif ()

  add_executable(${PROJECT_NAME} WIN32 ${SILK_SOURCES} ${SILK_HEADERS} ${SILK_RESOURCES} ${QM_FILES} ${SILK_ICON} ${SILK_QMLS})
endif ()

INCLUDE(Dart)
# enable_testing should be in the root
enable_testing()
add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(crashreporter)
add_subdirectory(test)

if(APPLE)
  target_link_libraries(${PROJECT_NAME}
                        ${PROJECT_NAME}_widgets
                        ${Breakpad}
                        ${Squirrel}
                        ${Mantle}
                        ${ReactiveCocoa}
  )
elseif (MSVC)
  target_link_libraries(${PROJECT_NAME}
                        ${PROJECT_NAME}_widgets
  )
  target_link_libraries(${PROJECT_NAME} ${BREAKPAD_LIBS}/common ${BREAKPAD_LIBS}/exception_handler ${BREAKPAD_LIBS}/crash_generation_client)
endif()

if(APPLE)
  # bundle themes
  file(COPY ${PROJECT_SOURCE_DIR}/resources/themes DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources)

  # bundle builtin packages
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources)

  # copy npm
  file(COPY
     ${PROJECT_SOURCE_DIR}/vendor/npm/bin/npm-cli.js
     DESTINATION
     ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/npm/bin)

  file(COPY
     ${PROJECT_SOURCE_DIR}/vendor/npm/lib
     ${PROJECT_SOURCE_DIR}/vendor/npm/node_modules
     ${PROJECT_SOURCE_DIR}/vendor/npm/package.json
     DESTINATION
     ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/npm)

  # bundle jslib
  set(JSLIB_DIR ${PROJECT_SOURCE_DIR}/minified/jslib)
  if(NOT IS_DIRECTORY ${JSLIB_DIR})
    message(FATAL_ERROR "${JSLIB_DIR} doesn't exits. Please run 'npm run minify'")
  endif()

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/jslib)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${JSLIB_DIR} ${PROJECT_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/jslib)

  set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist.in)
  set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist.in)
  set_target_properties( ${PROJECT_NAME} PROPERTIES FRAMEWORK true )

  # run install_name_tool to update the path to libnode.dylib
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND install_name_tool -change /usr/local/lib/libnode.dylib @executable_path/../Frameworks/libnode_debug.dylib ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/MacOS/${MACOSX_BUNDLE_BUNDLE_NAME})
  else ()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND install_name_tool -change /usr/local/lib/libnode.dylib @executable_path/../Frameworks/libnode.dylib ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/MacOS/${MACOSX_BUNDLE_BUNDLE_NAME})
  endif()

  # Use special icon for edge build
  if (BUILD_EDGE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_SOURCE_DIR}/resources/silkedit_edge.icns ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/silkedit.icns)
  endif (BUILD_EDGE)

  # bundle fonts
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp -r ${PROJECT_SOURCE_DIR}/resources/fonts ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources)

  file(COPY ${QTDIR}/translations/qt_ja.qm
            ${QTDIR}/translations/qtmultimedia_ja.qm
            ${QTDIR}/translations/qtbase_ja.qm
            ${QTDIR}/translations/qtquick1_ja.qm
            ${QTDIR}/translations/qtscript_ja.qm
            ${QTDIR}/translations/qtxmlpatterns_ja.qm DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/translations)

  # breakpad framework
  file(COPY ${Breakpad} DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Frameworks)
  # Squirrel framework
  file(COPY ${Squirrel} DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Frameworks)
  # Mantle framework
  file(COPY ${Mantle} DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Frameworks)
  # ReactiveCocoa framework
  file(COPY ${ReactiveCocoa} DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Frameworks)

  # copy libnode.dylib
  file(COPY ${NODE_LIBRARY} DESTINATION ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Frameworks)

  # copy CrashReporter
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp -R ${PROJECT_BINARY_DIR}/crashreporter/CrashReporter.app/Contents/MacOS/CrashReporter ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/MacOS)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_BINARY_DIR}/crashreporter/crashreporter_*.qm ${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Resources/translations)

elseif (MSVC)
  # Copy themes
  file(COPY ${PROJECT_SOURCE_DIR}/resources/themes DESTINATION silk_resources)

  # Copy packages directory to the same directory of exe
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION silk_resources)

  # bundle jslib
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory silk_resources/jslib)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/minified/jslib silk_resources/jslib)

  # copy npm
  file(COPY
     ${PROJECT_SOURCE_DIR}/vendor/npm/bin/npm-cli.js
     DESTINATION silk_resources/npm/bin)

  file(COPY
     ${PROJECT_SOURCE_DIR}/vendor/npm/lib
     ${PROJECT_SOURCE_DIR}/vendor/npm/node_modules
     ${PROJECT_SOURCE_DIR}/vendor/npm/package.json
     DESTINATION silk_resources/npm)

  # Copy onig.dll
  file(COPY ${PROJECT_SOURCE_DIR}/lib/onig.dll DESTINATION .)

  # Copy node.dll
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(COPY ${PROJECT_SOURCE_DIR}/vendor/node/Debug/node.dll DESTINATION .)
  else ()
    file(COPY ${CONAN_BIN_DIRS_NODE}/node.dll DESTINATION .)
  endif ()

  # Copy pdb
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    file(COPY "${PROJECT_SOURCE_DIR}/lib/yaml-cpp static md.pdb" DESTINATION .)
    file(COPY ${PROJECT_SOURCE_DIR}/lib/icu/icuMDd.pdb DESTINATION .)
  endif()

  # Copy Qt translation files
  file(COPY ${QTDIR}/translations/qt_ja.qm
            ${QTDIR}/translations/qtmultimedia_ja.qm
            ${QTDIR}/translations/qtbase_ja.qm
            ${QTDIR}/translations/qtquick1_ja.qm
            ${QTDIR}/translations/qtscript_ja.qm
            ${QTDIR}/translations/qtxmlpatterns_ja.qm DESTINATION ${PROJECT_BINARY_DIR}/silk_resources/translations)

  # copy CrashReporter
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/crashreporter/crashreporter.exe ${PROJECT_BINARY_DIR})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/crashreporter/crashreporter_ja.qm ${PROJECT_BINARY_DIR})

  # Move silkedit_ja.qm to translations directory
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/silkedit_ja.qm ${PROJECT_BINARY_DIR}/silk_resources/translations/silkedit_ja.qm)
endif()
