#ifndef ISTLFILELOADER_H
#define ISTLFILELOADER_H

#include "../../domain/BoundingBox.h"
#include <memory>
#include <string>

namespace MarcSLM {
namespace Application {

/**
 * @brief Mesh data loaded from an STL file
 * 
 * This is an infrastructure-agnostic representation of mesh geometry.
 * Concrete implementations (VTK, Qt3D, etc.) will populate this from
 * their specific mesh data structures.
 */
struct MeshData {
    Domain::BoundingBox bounds;
    int triangleCount = 0;
    double volume = 0.0;
    
    // Opaque shared ownership of infrastructure-specific data (e.g., vtkPolyData)
    // Use std::shared_ptr<void> so core interfaces don't include VTK headers.
    std::shared_ptr<void> nativeData;
};

/**
 * @brief Interface for loading STL files
 * 
 * This abstraction allows the application layer to load STL files
 * without depending on specific libraries (VTK, Assimp, etc.).
 */
class IStlFileLoader {
public:
    virtual ~IStlFileLoader() = default;
    
    /**
     * @brief Load an STL file and return mesh data
     * @param filePath Absolute path to STL file
     * @return Mesh data with computed bounds, triangle count, and volume
     *         Returns nullptr on failure
     */
    virtual std::unique_ptr<MeshData> load(const std::string& filePath) = 0;
};

} // namespace Application
} // namespace MarcSLM

#endif // ISTLFILELOADER_H
