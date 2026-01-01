# Infrastructure library (VTK, DLL adapter)
# Depends on Qt6, VTK, and MarcCore
add_library(MarcInfrastructure STATIC
    VtkStlFileLoader.cpp
    VtkModelRenderer.cpp
    MarcDllAdapter.cpp
    CollisionVisualizer.cpp
    # BuildVolumeVisualizer.cpp  # TODO: Implement
)

target_link_libraries(MarcInfrastructure PUBLIC
    MarcCore
    VTK::CommonCore
    VTK::IOGeometry
    VTK::FiltersSources
    VTK::RenderingCore
)

# Link marcapi if available (MarcDllAdapter needs it)
# Check multiple possible locations for marcapi library
set(MARCAPI_FOUND FALSE)

# First check: MarcSLM directory (where runtime files are placed)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../MarcSLM/libmarcapi.dll.a")
    target_link_libraries(MarcInfrastructure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../MarcSLM/libmarcapi.dll.a)
    message(STATUS "MarcInfrastructure: Linked to marcapi library (MarcSLM directory)")
    set(MARCAPI_FOUND TRUE)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../MarcSLM/marcapi.dll")
    target_link_libraries(MarcInfrastructure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../MarcSLM/marcapi.dll)
    message(STATUS "MarcInfrastructure: Linked to marcapi.dll (MarcSLM directory)")
    set(MARCAPI_FOUND TRUE)
# Second check: build-debug directory (current build dir)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../build-debug/libmarcapi.dll.a")
    target_link_libraries(MarcInfrastructure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../build-debug/libmarcapi.dll.a)
    message(STATUS "MarcInfrastructure: Linked to marcapi library (build-debug directory)")
    set(MARCAPI_FOUND TRUE)
# Third check: build directory (fallback)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../build/libmarcapi.dll.a")
    target_link_libraries(MarcInfrastructure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../build/libmarcapi.dll.a)
    message(STATUS "MarcInfrastructure: Linked to marcapi library (build directory)")
    set(MARCAPI_FOUND TRUE)
endif()

if(NOT MARCAPI_FOUND)
    message(WARNING "MarcInfrastructure: marcapi library not found in any expected location - MarcDllAdapter may not work")
    message(WARNING "MarcInfrastructure: Searched in: MarcSLM/, build-debug/, build/")
endif()

target_include_directories(MarcInfrastructure PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(MarcInfrastructure PUBLIC cxx_std_17)

# MSVC-specific settings
if(MSVC)
    target_compile_options(MarcInfrastructure PRIVATE 
        /wd4251  # Disable DLL interface warnings
        /wd4275  # Disable non-DLL interface base class warnings
    )
endif()
