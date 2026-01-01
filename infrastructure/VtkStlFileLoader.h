#ifndef VTKSTLFILELOADER_H
#define VTKSTLFILELOADER_H

#include "../core/application/interfaces/IStlFileLoader.h"

#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkPolyData.h>
#include <vtkMassProperties.h>

namespace MarcSLM {
namespace Infrastructure {

/**
 * @brief VTK implementation of STL file loader
 * 
 * Uses vtkSTLReader to load binary and ASCII STL files.
 * Computes bounds, triangle count, and volume using VTK filters.
 */
class VtkStlFileLoader : public Application::IStlFileLoader {
public:
    VtkStlFileLoader() = default;
    ~VtkStlFileLoader() override = default;
    
    std::unique_ptr<Application::MeshData> load(const std::string& filePath) override;
    
private:
    Domain::BoundingBox computeBounds(vtkPolyData* polyData);
    double computeVolume(vtkPolyData* polyData);
};

} // namespace Infrastructure
} // namespace MarcSLM

#endif // VTKSTLFILELOADER_H
