# Core library (domain + application layers)
# Pure C++17, no Qt or VTK dependencies
add_library(MarcCore STATIC
    # Domain layer
    domain/Model.cpp
    domain/BuildPlate.cpp
    
    # Application layer
    application/usecases/AddModelUseCase.cpp
)

target_include_directories(MarcCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(MarcCore PUBLIC cxx_std_17)

# No external dependencies for core library
# This ensures domain logic remains framework-agnostic
