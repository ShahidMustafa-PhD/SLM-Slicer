cmake_minimum_required(VERSION 3.28)
project(MarcSLM VERSION 1.0 LANGUAGES CXX)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type to Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_BUILD_TYPE Release)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set default install prefix to install-debug folder in project root
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install-debug" CACHE PATH "Install prefix" FORCE)
endif()
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_VERBOSE_MAKEFILE ON)

# ========================================
# QT CONFIGURATION
# ========================================

# Qt helpers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_WIN32_EXECUTABLE TRUE)

<<<<<<< HEAD
set(Qt6_DIR "C:/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6")
# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg Xml SvgWidgets Concurrent)
set(VTK_DIR "C:/vtk_install/lib/cmake/vtk-9.5")
=======
set(Qt6_DIR "C:/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6")
# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg Xml SvgWidgets Concurrent)
set(VTK_DIR "C:/vtk_install2/lib/cmake/vtk-9.5")
>>>>>>> 455fd16 (Initial commit-architectural changes)
# Find VTK with vcpkg
find_package(VTK REQUIRED COMPONENTS
  CommonCore
  IOGeometry
  FiltersSources
  RenderingCore
  RenderingOpenGL2
  InteractionStyle
  GUISupportQt
  RenderingAnnotation
)

# ========================================
# NEW ARCHITECTURE: Clean Layered Design
# ========================================

add_subdirectory(core)
add_subdirectory(infrastructure)
add_subdirectory(presentation)

# ========================================
# EXISTING UI CODE (Legacy - To Be Refactored)
# ========================================

# =========================
# EXISTING UI CODE
# =========================
set(LIBDIRQT ${CMAKE_CURRENT_SOURCE_DIR}/marc_qtsrc)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/MarcSLM)

set(UI_FILES ${LIBDIRQT}/mainwindow.ui)
set(SRC_FILES
    ${LIBDIRQT}/Main.cpp
    ${LIBDIRQT}/mainwindow.cpp
    ${LIBDIRQT}/LayerViewer.cpp
    ${LIBDIRQT}/SvgLoaderWorker.cpp
    ${LIBDIRQT}/ConfigDialog.cpp
    ${LIBDIRQT}/StlViewer.cpp
    ${LIBDIRQT}/StlLoader.cpp
    ${LIBDIRQT}/CustomInteractorStyle.cpp
    ${LIBDIRQT}/OrientationOptimizer.cpp
    ${LIBDIRQT}/OrientationOptimizerInterface.cpp
)

set(HDR_FILES
    ${LIBDIRQT}/mainwindow.h
    ${LIBDIRQT}/LayerViewer.h
    ${LIBDIRQT}/SvgLoaderWorker.h
    ${LIBDIRQT}/ConfigDialog.h
    ${LIBDIRQT}/StlViewer.h
    ${LIBDIRQT}/StlLoader.h
    ${LIBDIRQT}/MarcAPIinterface.h
    ${LIBDIRQT}/CustomInteractorStyle.h
    ${LIBDIRQT}/OrientationOptimizer.h
    ${LIBDIRQT}/OrientationOptimizerInterface.h
    ${LIBDIRQT}/slmcommons.h
    ${LIBDIRQT}/CustomVTKWidget.h
    ${LIBDIRQT}/toDllFromDll.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBDIRQT})

qt_add_resources(MY_RESOURCES ${LIBDIRQT}/resources.qrc)

# =========================
# CREATE EXECUTABLE
# =========================
add_executable(MarcSLM
    ${SRC_FILES}
    ${HDR_FILES}
    ${UI_FILES}
    ${MY_RESOURCES}
)

set(MAIN_APP_TARGET MarcSLM)  # Must be set AFTER add_executable

set_target_properties(MarcSLM PROPERTIES WIN32_EXECUTABLE TRUE)
target_compile_definitions(MarcSLM PRIVATE QT_NO_TRANSLATION)

target_link_libraries(MarcSLM PRIVATE
    MarcCore
    MarcInfrastructure
    MarcPresentation

    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::Xml
    Qt6::Concurrent
    Qt6::SvgWidgets

    VTK::CommonCore
    VTK::IOGeometry
    VTK::FiltersSources
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::InteractionStyle
    VTK::GUISupportQt
    VTK::RenderingAnnotation

    ${CMAKE_CURRENT_SOURCE_DIR}/libmarcapi.dll.a
)

# =========================
# MSVC SETTINGS
# =========================
if(MSVC)
    target_compile_options(MarcSLM PRIVATE 
        /std:c++17
        /wd4251
        /wd4275
        /permissive-
    )
    target_compile_definitions(MarcSLM PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

# =========================
# POST-BUILD: Deploy Qt Dependencies
# =========================

# Get Qt6 installation directory for windeployqt
get_target_property(Qt6Core_LOCATION Qt6::Core IMPORTED_LOCATION_DEBUG)
get_filename_component(Qt6_BIN_DIR "${Qt6Core_LOCATION}" DIRECTORY)
get_filename_component(Qt6_ROOT "${Qt6_BIN_DIR}" DIRECTORY)
get_filename_component(Qt6_INSTALL_PREFIX "${Qt6_ROOT}" DIRECTORY)
message(STATUS "Qt6 installation prefix: ${Qt6_INSTALL_PREFIX}")

# Ensure vcpkg / CMake know which windeployqt to use
set(WINDEPLOYQT_EXECUTABLE "${Qt6_ROOT}/bin/windeployqt.exe" CACHE FILEPATH "Path to windeployqt.exe")

# Post-build: Deploy Qt dependencies using windeployqt
if(WIN32)
    add_custom_command(TARGET ${MAIN_APP_TARGET} POST_BUILD
        COMMAND "${Qt6_ROOT}/bin/windeployqt.exe"
            --dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            $<$<CONFIG:Debug>:--debug>
            $<$<CONFIG:Release>:--release>
            --no-translations
            "$<TARGET_FILE:${MAIN_APP_TARGET}>"
        COMMENT "Running windeployqt for $<CONFIG> build"
    )
    # Copy the MARC API DLL to the output directory
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/MarcAPI.dll")
        add_custom_command(TARGET ${MAIN_APP_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/MarcAPI.dll" "$<TARGET_FILE_DIR:${MAIN_APP_TARGET}>"
            COMMENT "Copying MarcAPI.dll to output directory"
        )
    endif()
endif()

# =========================
# INSTALLER & CUSTOM TARGET
# =========================
install(TARGETS ${MAIN_APP_TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install Qt plugins
if(WIN32)
    get_target_property(Qt6Core_LOCATION Qt6::Core IMPORTED_LOCATION_DEBUG)
    get_filename_component(Qt6_BIN_DIR "${Qt6Core_LOCATION}" DIRECTORY)
    get_filename_component(Qt6_ROOT "${Qt6_BIN_DIR}" DIRECTORY)
    get_filename_component(Qt6_INSTALL_PREFIX "${Qt6_ROOT}" DIRECTORY)
    set(Qt6_PLUGINS_DIR "${Qt6_INSTALL_PREFIX}/plugins")
    
    # install(DIRECTORY "${Qt6_PLUGINS_DIR}/platforms" DESTINATION bin)
    # install(DIRECTORY "${Qt6_PLUGINS_DIR}/imageformats" DESTINATION bin)
    # install(DIRECTORY "${Qt6_PLUGINS_DIR}/styles" DESTINATION bin)
endif()

# Install VTK runtime DLLs into the installer `bin` folder
if(WIN32)
    # Try to discover imported locations for a known VTK target
    get_target_property(_vtk_loc_debug VTK::CommonCore IMPORTED_LOCATION_DEBUG)
    get_target_property(_vtk_loc_release VTK::CommonCore IMPORTED_LOCATION_RELEASE)

    set(_vtk_dlls)

    if(_vtk_loc_debug)
        get_filename_component(_vtk_bin_debug "${_vtk_loc_debug}" DIRECTORY)
        file(GLOB _vtk_dlls_debug "${_vtk_bin_debug}/*.dll")
        list(APPEND _vtk_dlls ${_vtk_dlls_debug})
    endif()

    if(_vtk_loc_release)
        get_filename_component(_vtk_bin_release "${_vtk_loc_release}" DIRECTORY)
        file(GLOB _vtk_dlls_release "${_vtk_bin_release}/*.dll")
        list(APPEND _vtk_dlls ${_vtk_dlls_release})
    endif()

    list(REMOVE_DUPLICATES _vtk_dlls)

    if(_vtk_dlls)
        message(STATUS "Installing VTK runtime DLLs: ${_vtk_dlls}")
        install(FILES ${_vtk_dlls} DESTINATION bin)
    else()
        message(WARNING "VTK: no runtime DLLs discovered to install. Ensure VTK was found and has IMPORTED_LOCATION properties.")
    endif()
endif()

add_custom_target(InstallDebug ALL
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${CMAKE_INSTALL_PREFIX}" --config Debug --verbose
    DEPENDS ${MAIN_APP_TARGET}
    COMMENT "Installing application to ${CMAKE_INSTALL_PREFIX}/bin"
)
add_dependencies(InstallDebug ${MAIN_APP_TARGET})

add_subdirectory(installer)
