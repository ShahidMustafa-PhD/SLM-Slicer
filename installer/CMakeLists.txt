# ========================================
# INSTALLER CONFIGURATION FOR ALL BUILD TYPES
# ========================================

cmake_minimum_required(VERSION 3.16)

# Ensure main app target is defined
if(NOT DEFINED MAIN_APP_TARGET)
  message(FATAL_ERROR "MAIN_APP_TARGET must be set by parent before adding installer directory")
endif()

message(STATUS "=========== INSTALLER CONFIGURATION ===========")
message(STATUS "Main app target: ${MAIN_APP_TARGET}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Runtime output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")

# ========================================
# HELPER FUNCTIONS
# ========================================

function(install_runtime_dependencies)
    set(options "")
    set(oneValueArgs TARGET DESTINATION QT_ROOT VTK_DIR)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Install Qt DLLs
    file(GLOB QT_DLLS "${ARG_QT_ROOT}/bin/Qt6*.dll")
    install(FILES ${QT_DLLS} DESTINATION ${ARG_DESTINATION})

    # Install VTK DLLs - assume VTK is installed in VTK_DIR/../../..
    get_filename_component(VTK_ROOT "${ARG_VTK_DIR}/../../.." ABSOLUTE)
    file(GLOB VTK_DLLS "${VTK_ROOT}/bin/vtk*.dll")
    install(FILES ${VTK_DLLS} DESTINATION ${ARG_DESTINATION})

    # Also MSVC runtime if needed (simplified)
    # Note: In a real scenario, use InstallRequiredSystemLibraries or similar
endfunction()

function(install_qt_plugins)
    set(options "")
    set(oneValueArgs DESTINATION QT_ROOT)
    set(multiValueArgs PLUGINS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(plugin IN LISTS ARG_PLUGINS)
        # Check if the plugin directly exists first
        if(EXISTS "${ARG_QT_ROOT}/plugins/${plugin}")
            install(DIRECTORY "${ARG_QT_ROOT}/plugins/${plugin}" DESTINATION "${ARG_DESTINATION}")
        elseif(EXISTS "${ARG_QT_ROOT}/${plugin}") # Sometimes relative to root
            install(DIRECTORY "${ARG_QT_ROOT}/${plugin}" DESTINATION "${ARG_DESTINATION}")
        else()
             message(WARNING "Qt Plugin folder not found: ${plugin} in ${ARG_QT_ROOT}/plugins")
        endif()
    endforeach()
endfunction()

# ========================================
# INSTALL EXECUTABLE AND RUNTIME FILES
# ========================================

# Install the main executable
install(TARGETS ${MAIN_APP_TARGET}
    RUNTIME DESTINATION bin
    COMPONENT applications
)

# Copy all runtime files from MarcSLM build directory to install/bin
# This includes Qt DLLs, plugins, and everything built by windeployqt
install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    DESTINATION bin
    COMPONENT applications
    FILES_MATCHING
    PATTERN "*.exe"
    PATTERN "*.dll"
    PATTERN "*.pdb"
    PATTERN "platforms/*"
    PATTERN "imageformats/*"
    PATTERN "styles/*"
)

# Install runtime dependencies
install_runtime_dependencies(TARGET ${MAIN_APP_TARGET}
    DESTINATION bin
    QT_ROOT ${QT_INSTALL_PREFIX}
    VTK_DIR ${VTK_DIR}
)

# Install Qt plugins
install_qt_plugins(DESTINATION bin
    QT_ROOT ${QT_INSTALL_PREFIX}
    PLUGINS "platforms" "imageformats" "styles"
)

# ========================================
# MINIMAL SUPPLEMENTAL INSTALLATION
# ========================================

# Only install marcapi if it exists - avoid complex vcpkg logic that might fail
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/marcapi.dll")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/marcapi.dll"
        DESTINATION bin
        COMPONENT libraries
    )
    message(STATUS "Will install marcapi.dll")
else()
    message(STATUS "INFO: marcapi.dll not found (optional)")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libmarcapi.dll.a")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/libmarcapi.dll.a"
        DESTINATION lib
        COMPONENT development
    )
    message(STATUS "Will install libmarcapi.dll.a")
endif()

# ========================================
# CREATE INSTALLATION INFO FILE
# ========================================

# Create a deployment info file at install time
install(CODE "
    set(info_file \"\${CMAKE_INSTALL_PREFIX}/INSTALLATION_COMPLETE.txt\")
    file(WRITE \"\${info_file}\" \"MarcSLM ${CMAKE_BUILD_TYPE} Build Installation
==================================

Installation completed successfully!

Build Type: ${CMAKE_BUILD_TYPE}
Install Date: \${CURRENT_TIMESTAMP}
Version: 1.0.0

Installation Location:
  ${CMAKE_INSTALL_PREFIX}

Binary Location:
  ${CMAKE_INSTALL_PREFIX}/bin/MarcSLM.exe

Key Files:
  ${CMAKE_INSTALL_PREFIX}/bin/MarcSLM.exe        - Main application
  ${CMAKE_INSTALL_PREFIX}/bin/platforms/qwindows.dll - Qt platform plugin
  ${CMAKE_INSTALL_PREFIX}/bin/Qt6*.dll           - Qt libraries

Quick Start:
  1. Navigate to: ${CMAKE_INSTALL_PREFIX}\\\\bin
  2. Run: MarcSLM.exe
  
Or from Visual Studio:
  1. In Solution Explorer, right-click project
  2. Select 'Open Folder in File Explorer'
  3. Navigate to install-${CMAKE_BUILD_TYPE}\\\\bin
  4. Double-click MarcSLM.exe

System Requirements:
  - Windows 10/11 (64-bit)
  - Visual C++ 2019+ redistributable (usually already installed)

Troubleshooting:
  If the app fails to start:
  - Verify ${CMAKE_INSTALL_PREFIX}\\\\bin\\\\platforms\\\\qwindows.dll exists
  - Check Windows Event Viewer for detailed error messages
  - Ensure all Qt DLLs are present in the bin/ folder

Build Process Notes:
  - This installation was created automatically when you clicked 'Build All'
  - All dependencies are self-contained in the bin/ folder
  - No system-wide installation required
  - Safe to copy the entire install-${CMAKE_BUILD_TYPE} folder to other machines
\")
    message(STATUS \"? Installation successful at: \${CMAKE_INSTALL_PREFIX}\")
    message(STATUS \"? Application ready at: \${CMAKE_INSTALL_PREFIX}/bin/MarcSLM.exe\")
")

# ========================================
# POST-INSTALL VERIFICATION
# ========================================

install(CODE "
    message(STATUS \"\")
    message(STATUS \"=== INSTALLATION VERIFICATION ===\")
    
    # Verify key files exist
    set(required_files
        \"bin/${MAIN_APP_TARGET}.exe\"
        \"bin/platforms/qwindows.dll\"
    )
    
    foreach(required_file IN LISTS required_files)
        set(file_path \"\${CMAKE_INSTALL_PREFIX}/\${required_file}\")
        if(EXISTS \"\${file_path}\")
            message(STATUS \"? Found: \${required_file}\")
        else()
            message(WARNING \"? MISSING: \${required_file}\")
        endif()
    endforeach()
    
    # Count DLLs installed
    file(GLOB dll_count \"\${CMAKE_INSTALL_PREFIX}/bin/*.dll\")
    list(LENGTH dll_count num_dlls)
    message(STATUS \"Total DLLs installed: \${num_dlls}\")
    
    message(STATUS \"\")
    message(STATUS \"Installation location: \${CMAKE_INSTALL_PREFIX}\")
    message(STATUS \"Run from: \${CMAKE_INSTALL_PREFIX}\\\\bin\\\\${MAIN_APP_TARGET}.exe\")
    message(STATUS \"\")
")

# ========================================
# CPACK CONFIGURATION (Optional)
# ========================================

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "MarcSLM")
set(CPACK_PACKAGE_VENDOR "Marc Inc.")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MarcSLM Qt Application (${CMAKE_BUILD_TYPE} Build)")
set(CPACK_PACKAGE_FILE_NAME "MarcSLM-1.0.0-${CMAKE_BUILD_TYPE}")

include(CPack)

message(STATUS "CPACK configured for optional package creation")
message(STATUS "Package name will be: MarcSLM-1.0.0-${CMAKE_BUILD_TYPE}")
message(STATUS "=============================================")
