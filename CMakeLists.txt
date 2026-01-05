cmake_minimum_required(VERSION 3.28)
project(MarcSLM VERSION 1.0 LANGUAGES CXX)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set default install prefix to install folder in project root
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Install prefix" FORCE)
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

set(Qt6_DIR "C:/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6")
# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg Xml SvgWidgets Concurrent)

set(VTK_DIR "C:/vtk_install2/lib/cmake/vtk-9.5")

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
# POST-BUILD: Deploy Qt Dependencies (Development Only)
# =========================
# Note: This is ONLY for running from the build directory during development.
# Production deployment uses the install rules below.

# Get Qt6 installation directory for windeployqt
get_target_property(Qt6Core_LOCATION Qt6::Core IMPORTED_LOCATION_RELEASE)
if(NOT Qt6Core_LOCATION)
    get_target_property(Qt6Core_LOCATION Qt6::Core IMPORTED_LOCATION_DEBUG)
endif()
if(NOT Qt6Core_LOCATION)
    get_target_property(Qt6Core_LOCATION Qt6::Core IMPORTED_LOCATION)
endif()
get_filename_component(Qt6_BIN_DIR "${Qt6Core_LOCATION}" DIRECTORY)
get_filename_component(Qt6_ROOT "${Qt6_BIN_DIR}" DIRECTORY)
message(STATUS "Qt6 root directory: ${Qt6_ROOT}")

# Ensure vcpkg / CMake know which windeployqt to use
set(WINDEPLOYQT_EXECUTABLE "${Qt6_ROOT}/bin/windeployqt.exe" CACHE FILEPATH "Path to windeployqt.exe")

# Post-build: Deploy Qt dependencies using windeployqt (development only)
if(WIN32)
    add_custom_command(TARGET ${MAIN_APP_TARGET} POST_BUILD
        COMMAND "${Qt6_ROOT}/bin/windeployqt.exe"
            --dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            "$<IF:$<CONFIG:Debug>,--debug,--release>"
            "$<TARGET_FILE:${MAIN_APP_TARGET}>"
        COMMENT "Running windeployqt for development build ($<CONFIG>)"
    )
    
    # Copy the MARC API DLL to the output directory (development only)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/MarcAPI.dll")
        add_custom_command(TARGET ${MAIN_APP_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                "${CMAKE_CURRENT_SOURCE_DIR}/MarcAPI.dll" 
                "$<TARGET_FILE_DIR:${MAIN_APP_TARGET}>"
            COMMENT "Copying MarcAPI.dll for development build"
        )
    endif()
endif()

# ============================================================================
# PROFESSIONAL INSTALLATION CONFIGURATION
# ============================================================================
# This section implements a clean install tree with:
#   - Executable in install root (install/MarcSLM.exe)
#   - All DLLs in bin/ (install/bin/*.dll)
#   - Qt plugins in plugins/ (install/plugins/*)
#   - Data files in data/ and shaders/
#
# Usage:
#   cmake --build build --config Release
#   cmake --install build --config Release
# ============================================================================

# Include the installer module that provides the helper functions
add_subdirectory(installer)

message(STATUS "========================================")
message(STATUS "Installation Configuration")
message(STATUS "========================================")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Target executable: ${MAIN_APP_TARGET}.exe")
message(STATUS "========================================")

# ---------------------------------------------------------------------------
# 1. Install the main executable to the ROOT of the install directory
# ---------------------------------------------------------------------------

install(TARGETS ${MAIN_APP_TARGET}
  RUNTIME DESTINATION .
  COMPONENT Application
)

message(STATUS "Executable will be installed to: ${CMAKE_INSTALL_PREFIX}/")

# ---------------------------------------------------------------------------
# 2. Install the MARC API DLL to bin/ (project-specific dependency)
# ---------------------------------------------------------------------------

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/MarcAPI.dll")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/MarcAPI.dll"
    DESTINATION bin
    COMPONENT Runtime
  )
  message(STATUS "MarcAPI.dll will be installed to: bin/")
else()
  message(WARNING "MarcAPI.dll not found at: ${CMAKE_CURRENT_SOURCE_DIR}/MarcAPI.dll")
endif()

# ---------------------------------------------------------------------------
# 3. Install all runtime dependencies (Qt, VTK, MSVC runtime, etc.) to bin/
# ---------------------------------------------------------------------------

install_runtime_dependencies(
  TARGET ${MAIN_APP_TARGET}
  DESTINATION bin
  QT_ROOT "${Qt6_ROOT}"
  VTK_DIR "${VTK_DIR}"
)

message(STATUS "Runtime dependencies will be collected and installed to: bin/")

# ---------------------------------------------------------------------------
# 4. Install Qt plugins required for the application
# ---------------------------------------------------------------------------

install_qt_plugins(
  DESTINATION plugins
  PLUGINS platforms imageformats styles iconengines
  QT_ROOT "${Qt6_ROOT}"
)

message(STATUS "Qt plugins will be installed to: plugins/")

# ---------------------------------------------------------------------------
# 5. Install additional data files (if they exist)
# ---------------------------------------------------------------------------

# Install shaders directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    DESTINATION .
    COMPONENT Data
    FILES_MATCHING 
      PATTERN "*.vert" 
      PATTERN "*.frag" 
      PATTERN "*.glsl"
      PATTERN "*.hlsl"
  )
  message(STATUS "Shaders will be installed to: shaders/")
endif()

# Install data directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data")
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
    DESTINATION .
    COMPONENT Data
  )
  message(STATUS "Data files will be installed to: data/")
endif()

# ---------------------------------------------------------------------------
# 6. Create a launcher batch file for easy execution
# ---------------------------------------------------------------------------

set(LAUNCHER_SCRIPT_CONTENT "@echo off
REM ============================================================================
REM MarcSLM Launcher Script
REM ============================================================================
REM This script sets up the environment and launches the MarcSLM application.
REM It ensures that all required DLLs are found without modifying system PATH.
REM ============================================================================

setlocal

REM Get the directory where this script is located
set \"APP_DIR=%~dp0\"

REM Add the bin directory to PATH for this session only
set \"PATH=%APP_DIR%bin;%PATH%\"

REM Display launch information
echo ============================================================================
echo MarcSLM - Professional 3D Model Slicer
echo ============================================================================
echo Application directory: %APP_DIR%
echo DLL search path: %APP_DIR%bin
echo.
echo Starting application...
echo ============================================================================
echo.

REM Launch the application with all arguments passed to this script
\"%APP_DIR%${MAIN_APP_TARGET}.exe\" %*

REM Capture the exit code
set EXIT_CODE=%ERRORLEVEL%

REM If the application crashed or returned an error, pause to show the message
if %EXIT_CODE% NEQ 0 (
  echo.
  echo ============================================================================
  echo Application exited with error code: %EXIT_CODE%
  echo ============================================================================
  echo.
  pause
)

endlocal
exit /b %EXIT_CODE%
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/MarcSLM.bat" "${LAUNCHER_SCRIPT_CONTENT}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/MarcSLM.bat"
  DESTINATION .
  COMPONENT Application
)

message(STATUS "Launcher script will be installed to: MarcSLM.bat")

# ---------------------------------------------------------------------------
# 7. Create a README for the installation
# ---------------------------------------------------------------------------

set(README_CONTENT "# MarcSLM Installation

## Directory Structure

install/
├── MarcSLM.exe          ← Main application executable
├── MarcSLM.bat          ← Launcher script (recommended)
├── qt.conf             ← Qt configuration
├── README.txt          ← This file
├── bin/                ← All runtime DLLs
│   ├── Qt6Core.dll
│   ├── Qt6Widgets.dll
│   ├── Qt6Gui.dll
│   ├── vtkCommonCore-9.5.dll
│   ├── MarcAPI.dll
│   └── ... (other DLLs)
├── plugins/            ← Qt plugins
│   ├── platforms/
│   ├── imageformats/
│   ├── styles/
│   └── iconengines/
├── data/               ← Application data
└── shaders/            ← Shader files

## Running the Application

### Option 1: Using the Launcher (Recommended)
Double-click MarcSLM.bat or run from command line:
  MarcSLM.bat

### Option 2: Direct Execution
Double-click MarcSLM.exe or run from command line:
  MarcSLM.exe

The application will automatically find DLLs in the bin/ subdirectory.

## Troubleshooting

### DLL Not Found Errors

If you encounter \"DLL not found\" errors:

1. Verify that the bin/ directory contains all required DLLs
2. Check that qt.conf points to the correct plugins directory
3. Try running via MarcSLM.bat instead of directly

### Manual Dependency Check

To verify all dependencies are resolved:
  dumpbin /dependents MarcSLM.exe
  dumpbin /dependents bin\\Qt6Core.dll

### Missing Qt Plugins

If Qt plugins are not loading:
1. Verify plugins/ directory exists and contains subdirectories
2. Check that qt.conf exists in the same directory as MarcSLM.exe
3. Set the environment variable: set QT_DEBUG_PLUGINS=1

## Support

For issues or questions, please contact the MarcSLM development team.
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/README.txt" "${README_CONTENT}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/README.txt"
  DESTINATION .
  COMPONENT Documentation
)

message(STATUS "README.txt will be installed to: ./")

# ---------------------------------------------------------------------------
# 8. Installation summary (runs at install time)
# ---------------------------------------------------------------------------

install(CODE "
  message(STATUS \"\")
  message(STATUS \"============================================================================\")
  message(STATUS \"MarcSLM Installation Complete\")
  message(STATUS \"============================================================================\")
  message(STATUS \"Install location: \${CMAKE_INSTALL_PREFIX}\")
  message(STATUS \"\")
  message(STATUS \"Installation Layout:\")
  message(STATUS \"  Executable:     \${CMAKE_INSTALL_PREFIX}/${MAIN_APP_TARGET}.exe\")
  message(STATUS \"  Runtime DLLs:   \${CMAKE_INSTALL_PREFIX}/bin/\")
  message(STATUS \"  Qt Plugins:     \${CMAKE_INSTALL_PREFIX}/plugins/\")
  message(STATUS \"  Launcher:       \${CMAKE_INSTALL_PREFIX}/MarcSLM.bat\")
  message(STATUS \"\")
  message(STATUS \"To run the application:\")
  message(STATUS \"  1. Double-click: MarcSLM.bat (recommended)\")
  message(STATUS \"  2. Or directly:  MarcSLM.exe\")
  message(STATUS \"\")
  message(STATUS \"Note: All DLLs are in bin/ - no PATH modification needed\")
  message(STATUS \"============================================================================\")
  message(STATUS \"\")
" COMPONENT Application)

# ---------------------------------------------------------------------------
# 9. Optional: Create convenient install targets
# ---------------------------------------------------------------------------

# Create custom targets for easy installation
add_custom_target(install_release
  COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --config Release --target ${MAIN_APP_TARGET}
  COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --config Release --prefix "${CMAKE_INSTALL_PREFIX}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Building and installing Release configuration to ${CMAKE_INSTALL_PREFIX}"
  VERBATIM
)

add_custom_target(install_debug
  COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --config Debug --target ${MAIN_APP_TARGET}
  COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --config Debug --prefix "${CMAKE_INSTALL_PREFIX}-debug"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Building and installing Debug configuration to ${CMAKE_INSTALL_PREFIX}-debug"
  VERBATIM
)

message(STATUS "========================================")
message(STATUS "Custom targets available:")
message(STATUS "  install_release - Build and install Release")
message(STATUS "  install_debug   - Build and install Debug")
message(STATUS "========================================")

# ============================================================================
# END OF INSTALLATION CONFIGURATION
# ============================================================================