cmake_minimum_required(VERSION 3.16)

project(PhysicsSandbox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options(-fsanitize=thread -fPIE -pie -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s -flto)
  #add_compile_options(-fPIE -pie -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s -flto)
  add_link_options(-fsanitize=thread -fPIE -pie -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s -flto)
  #add_compile_options(-fPIE -pie -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s -flto)
endif()
set(BOX2D_PATH /home/ben/Documents/Git_Repositories/box2d)
set(BOX2D_LIB_SUBDIR "")
set(SKIA_LIB_SUBDIR "")
set(SKIA_PATH "")
if (ANDROID)
  #arm64-v8a, armeabi-v7a, x86, x86_64
  set(BOX2D_LIB_SUBDIR build-android-${ANDROID_ABI}/src)
  if (${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(SKIA_PATH /home/ben/Downloads/Skia-m132-9b3c42e2f9-3-android-Release-arm64)
    set(SKIA_LIB_SUBDIR out/Release-android-arm64)
  elseif (${ANDROID_ABI} STREQUAL "x86_64")
    set(SKIA_PATH /home/ben/Downloads/Skia-m132-9b3c42e2f9-3-android-Release-x64)
    set(SKIA_LIB_SUBDIR out/Release-android-x64)
  else()
    message(CHECK_FAIL "Skia not available for Android ABI ${ANDROID_ABI}")
  endif()
elseif(LINUX)
  set(BOX2D_LIB_SUBDIR build/src)
  #set(SKIA_PATH /home/ben/Downloads/Skia-m132-9b3c42e2f9-3-linux-Release-x64)
  set(SKIA_PATH /home/ben/Documents/Git_Repositories/skia)
  #set(SKIA_LIB_SUBDIR out/Release-linux-x64)
  set(SKIA_LIB_SUBDIR out/Static-Debug)
endif()


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Qml Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGL OpenGLWidgets Widgets Qml Quick)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)
qt_add_resources(PROJECT_SOURCES resources.qrc)
include_directories(BEFORE ${BOX2D_PATH}/include ${SKIA_PATH})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PhysicsSandbox
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
				physworldview.h physworldview.cpp

        resources.qrc
        physworldview_gl_quick.h
        physworldview_gl_quick.cpp
    )
  qt_add_qml_module(PhysicsSandbox
      URI Extras
      QML_FILES main.qml
      SOURCES SkiaQuickItem.h
      SOURCES SkiaQuickItem.cpp
      SOURCES StarQuickItem.h
  )
# Define target properties for Android with Qt 6 as:
set_target_properties(${target} PROPERTIES
    QT_ANDROID_PACKAGE_NAME "org.qtproject.example.PhysicsSandbox"
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
)

# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
set_property(TARGET PhysicsSandbox APPEND PROPERTY QT_ANDROID_MIN_SDK_VERSION 27)
else()
    if(ANDROID)
        add_library(PhysicsSandbox SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PhysicsSandbox
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_directories(PhysicsSandbox BEFORE PRIVATE
  ${BOX2D_PATH}/${BOX2D_LIB_SUBDIR}
  ${SKIA_PATH}/${SKIA_LIB_SUBDIR}
)
message(STATUS "Skia static lib path: ${SKIA_PATH}/${SKIA_LIB_SUBDIR}")
target_link_libraries(PhysicsSandbox PRIVATE
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::OpenGL
	Qt${QT_VERSION_MAJOR}::OpenGLWidgets
  Qt${QT_VERSION_MAJOR}::Qml
  Qt${QT_VERSION_MAJOR}::Quick
  box2d
  skia
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PhysicsSandbox)
endif()
set_target_properties(PhysicsSandbox PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PhysicsSandbox
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PhysicsSandbox)
  qt_generate_deploy_qml_app_script(
    TARGET PhysicsSandbox
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
  NO_UNSUPPORTED_PLATFORM_ERROR
  DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM)
endif()
