# MarcSLM Control System

**Professional 3D Model Slicer for Selective Laser Melting (SLM)**

---

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Code Structure](#code-structure)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

---

## Overview

**MarcSLM** is a high-performance, industrial-grade software solution designed for slicing 3D models specifically for Selective Laser Melting (SLM) additive manufacturing processes. Built with modern C++17 and leveraging powerful libraries like Qt6 and VTK, it provides a robust environment for preparing print jobs, visualizing layers, and managing machine control parameters.

This project bridges the gap between raw 3D geometry and machine-executable instructions, offering precise control over scanning paths and layer generation. It is designed for industrial engineers and R&D specialists working in metal additive manufacturing.

---

## Features

- **Advanced Slicing Engines**: High-precision slicing algorithms suitable for complex geometries.
- **3D Visualization**: Integrated VTK-based rendering engine for real-time model and layer inspection.
- **Layered Architecture**: Clean separation of concerns using a Domain-Driven Design approach (Core, Infrastructure, Presentation).
- **Modern UI/UX**: Responsive and intuitive user interface built with Qt 6.9+ and modern C++17 standards.
- **Hardware Integration**: interfaces with `MarcAPI` for direct machine control and parameter management.
- **Legacy Support**: Includes migration paths and compatibility layers for legacy configurations (`marc_qtsrc`).
- **Automated Deployment**: CMake-based installer generation and dependency management.

---

## Architecture

The system follows a strict Clean Architecture / Layered Architecture pattern to ensure maintainability and testability:

1.  **Core (`core/`)**:
    *   Contains business logic, domain entities, and pure algorithms.
    *   Dependency-free (no references to UI or Infrastructure frameworks).
2.  **Infrastructure (`infrastructure/`)**:
    *   Implements interfaces defined in Core.
    *   Handles external concerns: File I/O, Database access, Hardware communication (MarcAPI), and Repositories.
3.  **Presentation (`presentation/`)**:
    *   The user interface layer.
    *   Uses Qt6 for views and view-models.
    *   Depends on Core and Infrastructure layers to function.

---

## Installation

### Prerequisites

*   **Operating System**: Windows 10/11 (x64)
*   **Compiler**: MSVC 2019/2022 (C++17 support required)
*   **Build System**: CMake 3.16+ (3.28+ recommended)
*   **Dependency Manager**: `vcpkg` (for VTK and other libs)
*   **Qt Framework**: Qt 6.9.3 (MSVC 2022 64-bit)
*   **Visualization**: VTK 9.5 (installed via vcpkg or pre-built binaries)

### Build Instructions

**1. Clone the Repository**
```bash
git clone https://github.com/YourOrg/SLM-Slicer.git
cd SLM-Slicer
```

**2. Configure with CMake**
Ensure paths to Qt and vcpkg toolchain are correctly set. This project supports CMake Presets if available.

```powershell
# Standard configuration
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
```

*Note: You may need to specify `-DQt6_DIR="C:/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6"` and `-DVTK_DIR="C:/path/to/vtk"` if they are not in standard system paths.*

**3. Build the Project**
```powershell
cmake --build build --config Release
```

**4. Install / Deploy**
To create a clean distribution folder with all dependencies:
```powershell
cmake --install build --config Release --prefix install
```

---

## Usage

After installation, the application can be run directly from the `install/bin` folder or via the generated launcher script.

### Running the Application (Launcher)
The build process generates a wrapper script `MarcSLM.bat` in the install root. This script sets up local environment variables (PATH) to ensure all DLLs are found without modifying system settings.

```cmd
.\install\MarcSLM.bat
```

### Running Directly
```cmd
cd install/bin
.\MarcSLM.exe
```

### Command Line Options
Currently, the application is primarily GUI-driven. Future releases may include CLI arguments for batch processing of STL files.

---

## Code Structure

```plaintext
/
??? core/                  # Domain logic and entities
??? infrastructure/        # Data access, API implementations
??? presentation/          # GUI code (Qt6 widgets/views)
??? installer/             # CMake install scripts & packaging
??? marc_qtsrc/           # Legacy UI components (transitioning)
??? cmake/                 # CMake helper modules
??? Models/                # Sample STL/SVG files
??? CMakeLists.txt         # Root build configuration
```

---

## Contributing

We welcome contributions from the engineering team!

1.  **Branching Strategy**: Use `feature/` branches for new capabilities and `fix/` branches for bug patches.
2.  **Pull Requests**: All code must pass the build pipeline before merging.
3.  **Style Guide**: Follow modern C++17 conventions. Qt code should adhere to standard Qt naming patterns (CamelCase for classes, camelCase for methods).

---

## License

**Proprietary Software**

Copyright © 2024-2026 MarcSLM Company. All Rights Reserved.
Unauthorized copying of this file, via any medium is strictly prohibited.

---

## Acknowledgements

*   **Qt Framework**: The Qt Company
*   **VTK (Visualization Toolkit)**: Kitware
*   **Microsoft vcpkg**: C++ Library Manager
