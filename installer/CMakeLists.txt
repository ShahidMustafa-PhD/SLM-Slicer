# ============================================================================
# InstallRuntimeDependencies.cmake
# ============================================================================
# Professional runtime dependency collection and installation for Windows
# applications using Qt, VTK, and other shared libraries.
#
# Usage:
#   install_runtime_dependencies(
#     TARGET <target-name>
#     DESTINATION <install-destination>
#     QT_ROOT <qt-root-path>
#     VTK_DIR <vtk-cmake-dir>
#   )
#
# Requirements:
#   - CMake 3.16+ (compatible with your minimum version)
#   - Windows platform
#   - Generator expressions support for multi-config generators
# ============================================================================

include_guard(GLOBAL)

function(install_runtime_dependencies)
  set(options "")
  set(oneValueArgs TARGET DESTINATION QT_ROOT VTK_DIR)
  set(multiValueArgs "")
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT ARG_TARGET)
    message(FATAL_ERROR "install_runtime_dependencies: TARGET is required")
  endif()

  if(NOT ARG_DESTINATION)
    message(FATAL_ERROR "install_runtime_dependencies: DESTINATION is required")
  endif()

  if(NOT WIN32)
    message(WARNING "install_runtime_dependencies: Only Windows is supported")
    return()
  endif()

  # =========================================================================
  # Step 1: Install code that runs at install-time to collect dependencies
  # =========================================================================
  
  install(CODE "
    # Determine configuration (Release or Debug)
    if(CMAKE_INSTALL_CONFIG_NAME STREQUAL \"Debug\")
      set(_config \"Debug\")
      set(_dll_suffix \"d\")
    else()
      set(_config \"Release\")
      set(_dll_suffix \"\")
    endif()
    
    message(STATUS \"Installing runtime dependencies for \${_config} configuration\")
    
    # Get the installed executable path (in root, not bin/)
    set(_exe_file \"\${CMAKE_INSTALL_PREFIX}/${ARG_TARGET}.exe\")
    
    if(NOT EXISTS \"\${_exe_file}\")
      message(WARNING \"Executable not found at install time: \${_exe_file}\")
      return()
    endif()
    
    message(STATUS \"========================================\")
    message(STATUS \"Analyzing runtime dependencies\")
    message(STATUS \"Executable: \${_exe_file}\")
    message(STATUS \"Configuration: \${_config}\")
    message(STATUS \"========================================\")
    
    # Prepare directories to exclude (system paths)
    set(_exclude_dirs
      \"C:/Windows\"
      \"C:/Windows/System32\"
      \"C:/Windows/SysWOW64\"
      \"C:/Windows/WinSxS\"
    )
    
    # Convert to regular expression patterns
    set(_exclude_regex)
    foreach(_dir \${_exclude_dirs})
      string(REPLACE \"/\" \"[/\\\\\\\\]\" _pattern \"\${_dir}\")
      list(APPEND _exclude_regex \"^\${_pattern}\")
    endforeach()
    
    # ======================================================================
    # Build search directories for Qt and VTK
    # ======================================================================
    set(_search_dirs)
    
    # Add Qt bin directory
    if(\"${ARG_QT_ROOT}\" STREQUAL \"\")
      message(WARNING \"QT_ROOT not provided, Qt DLLs may not be found\")
    else()
      set(_qt_bin \"${ARG_QT_ROOT}/bin\")
      if(EXISTS \"\${_qt_bin}\")
        list(APPEND _search_dirs \"\${_qt_bin}\")
        message(STATUS \"Qt bin directory: \${_qt_bin}\")
      endif()
    endif()
    
    # Add VTK bin directory
    if(\"${ARG_VTK_DIR}\" STREQUAL \"\")
      message(WARNING \"VTK_DIR not provided, VTK DLLs may not be found\")
    else()
      # VTK_DIR points to lib/cmake/vtk-X.Y, so we go up to find bin
      get_filename_component(_vtk_lib \"${ARG_VTK_DIR}\" DIRECTORY)
      get_filename_component(_vtk_lib \"\${_vtk_lib}\" DIRECTORY)
      get_filename_component(_vtk_root \"\${_vtk_lib}\" DIRECTORY)
      set(_vtk_bin \"\${_vtk_root}/bin\")
      if(EXISTS \"\${_vtk_bin}\")
        list(APPEND _search_dirs \"\${_vtk_bin}\")
        message(STATUS \"VTK bin directory: \${_vtk_bin}\")
      endif()
    endif()
    
    # Add install bin directory
    list(APPEND _search_dirs \"\${CMAKE_INSTALL_PREFIX}/bin\")
    
    # Add system PATH as last resort
    string(REPLACE \";\" \"\\\\;\" _env_path \"\$ENV{PATH}\")
    list(APPEND _search_dirs \"\${_env_path}\")
    
    # ======================================================================
    # Use file(GET_RUNTIME_DEPENDENCIES) to resolve all DLL dependencies
    # ======================================================================
    file(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES \"\${_exe_file}\"
      RESOLVED_DEPENDENCIES_VAR _resolved_deps
      UNRESOLVED_DEPENDENCIES_VAR _unresolved_deps
      CONFLICTING_DEPENDENCIES_PREFIX _conflicts
      DIRECTORIES \${_search_dirs}
      PRE_EXCLUDE_REGEXES \"api-ms-.*\" \"ext-ms-.*\"
      POST_EXCLUDE_REGEXES \".*[Ss]ystem32.*\" \".*[Ww]indows.*\"
    )
    
    # Report unresolved dependencies (but only non-system ones)
    if(_unresolved_deps)
      set(_critical_unresolved)
      foreach(_dep \${_unresolved_deps})
        # Check if it's a system DLL (lowercase comparison)
        string(TOLOWER \"\${_dep}\" _dep_lower)
        if(NOT \"\${_dep_lower}\" MATCHES \"^(kernel32|user32|gdi32|shell32|ole32|oleaut32|advapi32|ws2_32|opengl32|d3d.*)\\.dll\$\")
          list(APPEND _critical_unresolved \"\${_dep}\")
        endif()
      endforeach()
      
      if(_critical_unresolved)
        message(WARNING \"Unresolved dependencies (may cause runtime errors):\")
        foreach(_dep \${_critical_unresolved})
          message(WARNING \"  - \${_dep}\")
        endforeach()
      endif()
    endif()
    
    # Report conflicts (warning)
    if(DEFINED _conflicts_FILENAMES)
      message(WARNING \"Conflicting dependencies detected:\")
      foreach(_conflict \${_conflicts_FILENAMES})
        message(WARNING \"  - \${_conflict}\")
      endforeach()
    endif()
    
    # ======================================================================
    # Filter and install resolved dependencies
    # ======================================================================
    set(_dlls_to_install)
    set(_dll_count 0)
    
    foreach(_dep \${_resolved_deps})
      # Normalize path separators
      file(TO_CMAKE_PATH \"\${_dep}\" _dep)
      
      # Check if this DLL should be excluded (system library)
      set(_should_exclude FALSE)
      foreach(_pattern \${_exclude_regex})
        if(\"\${_dep}\" MATCHES \"\${_pattern}\")
          set(_should_exclude TRUE)
          break()
        endif()
      endforeach()
      
      if(_should_exclude)
        continue()
      endif()
      
      # Only install if the DLL exists
      if(EXISTS \"\${_dep}\")
        list(APPEND _dlls_to_install \"\${_dep}\")
        math(EXPR _dll_count \"\${_dll_count} + 1\")
      else()
        message(WARNING \"Resolved dependency does not exist: \${_dep}\")
      endif()
    endforeach()
    
    # Remove duplicates
    if(_dlls_to_install)
      list(REMOVE_DUPLICATES _dlls_to_install)
      list(LENGTH _dlls_to_install _dll_count)
    endif()
    
    # ======================================================================
    # Copy DLLs to the bin/ directory
    # ======================================================================
    set(_dest_dir \"\${CMAKE_INSTALL_PREFIX}/${ARG_DESTINATION}\")
    
    # Ensure destination directory exists
    file(MAKE_DIRECTORY \"\${_dest_dir}\")
    
    message(STATUS \"Installing \${_dll_count} runtime dependencies to: \${_dest_dir}\")
    message(STATUS \"----------------------------------------\")
    
    set(_install_count 0)
    foreach(_dll \${_dlls_to_install})
      get_filename_component(_dll_name \"\${_dll}\" NAME)
      set(_dest_file \"\${_dest_dir}/\${_dll_name}\")
      
      # Use execute_process with CMAKE copy command for reliability
      execute_process(
        COMMAND \"\${CMAKE_COMMAND}\" -E copy_if_different \"\${_dll}\" \"\${_dest_file}\"
        RESULT_VARIABLE _copy_result
        ERROR_VARIABLE _copy_error
        OUTPUT_QUIET
      )
      
      if(_copy_result EQUAL 0)
        message(STATUS \"  [OK] \${_dll_name}\")
        math(EXPR _install_count \"\${_install_count} + 1\")
      else()
        message(WARNING \"  [FAIL] Failed to install: \${_dll_name}\")
        if(_copy_error)
          message(WARNING \"    Error: \${_copy_error}\")
        endif()
      endif()
    endforeach()
    
    message(STATUS \"----------------------------------------\")
    message(STATUS \"Successfully installed \${_install_count}/\${_dll_count} DLLs\")
    message(STATUS \"========================================\")
  " COMPONENT Runtime)

endfunction()

# ============================================================================
# Qt Plugin Installation Helper
# ============================================================================

function(install_qt_plugins)
  set(options "")
  set(oneValueArgs DESTINATION QT_ROOT)
  set(multiValueArgs PLUGINS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT ARG_DESTINATION)
    set(ARG_DESTINATION "plugins")
  endif()

  if(NOT ARG_PLUGINS)
    set(ARG_PLUGINS platforms imageformats styles iconengines)
  endif()

  if(NOT WIN32)
    return()
  endif()

  if(NOT ARG_QT_ROOT)
    message(FATAL_ERROR "install_qt_plugins: QT_ROOT is required")
  endif()

  set(_qt_plugins_dir "${ARG_QT_ROOT}/plugins")

  if(EXISTS "${_qt_plugins_dir}")
    message(STATUS "Qt plugins directory found: ${_qt_plugins_dir}")
    
    # Install only the required configuration (Release or Debug) plugins
    install(CODE "
      # Determine configuration
      if(CMAKE_INSTALL_CONFIG_NAME STREQUAL \"Debug\")
        set(_plugin_pattern \"*d.dll\")
        set(_exclude_pattern \"[^d]\\\\.dll$\")
        set(_config_name \"Debug\")
      else()
        set(_plugin_pattern \"*.dll\")
        set(_exclude_pattern \"d\\\\.dll$\")
        set(_config_name \"Release\")
      endif()
      
      message(STATUS \"Installing Qt plugins for \${_config_name} configuration\")
      
      set(_plugin_categories \"${ARG_PLUGINS}\")
      foreach(_plugin_category \${_plugin_categories})
        set(_plugin_path \"${_qt_plugins_dir}/\${_plugin_category}\")
        
        if(EXISTS \"\${_plugin_path}\")
          file(GLOB _plugin_files \"\${_plugin_path}/\${_plugin_pattern}\")
          
          set(_installed_count 0)
          foreach(_plugin_file \${_plugin_files})
            get_filename_component(_plugin_name \"\${_plugin_file}\" NAME)
            
            # Skip wrong configuration
            if(CMAKE_INSTALL_CONFIG_NAME STREQUAL \"Debug\")
              # For Debug, only install files ending with 'd.dll'
              if(NOT \"\${_plugin_name}\" MATCHES \"d\\\\.dll$\")
                continue()
              endif()
            else()
              # For Release, exclude files ending with 'd.dll'
              if(\"\${_plugin_name}\" MATCHES \"d\\\\.dll$\")
                continue()
              endif()
            endif()
            
            set(_dest_dir \"\${CMAKE_INSTALL_PREFIX}/${ARG_DESTINATION}/\${_plugin_category}\")
            file(MAKE_DIRECTORY \"\${_dest_dir}\")
            
            execute_process(
              COMMAND \"\${CMAKE_COMMAND}\" -E copy_if_different \"\${_plugin_file}\" \"\${_dest_dir}/\${_plugin_name}\"
              RESULT_VARIABLE _copy_result
              OUTPUT_QUIET ERROR_QUIET
            )
            
            if(_copy_result EQUAL 0)
              math(EXPR _installed_count \"\${_installed_count} + 1\")
            endif()
          endforeach()
          
          message(STATUS \"  Installed \${_installed_count} plugins for category: \${_plugin_category}\")
        else()
          message(WARNING \"  Qt plugin category not found: \${_plugin_path}\")
        endif()
      endforeach()
    " COMPONENT Runtime)
    
    # Create qt.conf to help Qt locate plugins relative to executable
    set(_qt_conf_content "[Paths]\nPlugins = ${ARG_DESTINATION}\n")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" "${_qt_conf_content}")
    
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
      DESTINATION .
      COMPONENT Runtime
    )
    
    message(STATUS "  Created qt.conf pointing to ${ARG_DESTINATION}/")
  else()
    message(WARNING "Qt plugins directory not found: ${_qt_plugins_dir}")
  endif()

endfunction()