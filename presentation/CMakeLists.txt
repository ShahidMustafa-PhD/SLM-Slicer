# Presentation library (ViewModels, UI widgets)
# Depends on Qt6, MarcCore, and MarcInfrastructure
add_library(MarcPresentation STATIC
    MainWindowViewModel.cpp
    ModelListWidget.cpp
    PropertiesPanel.cpp
)

target_link_libraries(MarcPresentation PUBLIC
    MarcCore
    MarcInfrastructure
    Qt6::Core
    Qt6::Widgets
)

target_include_directories(MarcPresentation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(MarcPresentation PUBLIC cxx_std_17)

# Enable Qt MOC for this library
set_target_properties(MarcPresentation PROPERTIES
    AUTOMOC ON
)

# MSVC-specific settings
if(MSVC)
    target_compile_options(MarcPresentation PRIVATE 
        /wd4251  # Disable DLL interface warnings
        /wd4275  # Disable non-DLL interface base class warnings
    )
endif()
