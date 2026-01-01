#include "VtkStlFileLoader.h"
#include <vtkTriangleFilter.h>

namespace MarcSLM {
namespace Infrastructure {

std::unique_ptr<Application::MeshData> VtkStlFileLoader::load(const std::string& filePath) {
    // Create STL reader
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    vtkPolyData* polyData = reader->GetOutput();
    if (!polyData || polyData->GetNumberOfPoints() == 0) {
        return nullptr;
    }

    // Create mesh data
    auto meshData = std::make_unique<Application::MeshData>();
    meshData->bounds = computeBounds(polyData);
    meshData->triangleCount = static_cast<int>(polyData->GetNumberOfPolys());
    meshData->volume = computeVolume(polyData);

    // Store native data as a shared_ptr<void> that holds the raw vtkPolyData*.
    // We intentionally increase the VTK reference count and pair it with an
    // UnRegister deleter so the polydata remains alive while meshData exists.
    vtkSmartPointer<vtkPolyData> polySmart = polyData;
    polySmart->Register(nullptr); // increment refcount for ownership by shared_ptr
    meshData->nativeData = std::shared_ptr<void>(polySmart.GetPointer(), [](void* p){
        if (p) {
            static_cast<vtkPolyData*>(p)->UnRegister(nullptr);
        }
    });

    return meshData;
}

Domain::BoundingBox VtkStlFileLoader::computeBounds(vtkPolyData* polyData) {
    double bounds[6];
    polyData->GetBounds(bounds);

    return Domain::BoundingBox(
        bounds[0], bounds[1],  // X: min, max
        bounds[2], bounds[3],  // Y: min, max
        bounds[4], bounds[5]   // Z: min, max
    );
}

double VtkStlFileLoader::computeVolume(vtkPolyData* polyData) {
    // Ensure mesh is triangulated
    vtkSmartPointer<vtkTriangleFilter> triFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triFilter->SetInputData(polyData);
    triFilter->Update();

    // Compute volume
    vtkSmartPointer<vtkMassProperties> massProps = vtkSmartPointer<vtkMassProperties>::New();
    massProps->SetInputData(triFilter->GetOutput());
    massProps->Update();

    return massProps->GetVolume();
}

} // namespace Infrastructure
} // namespace MarcSLM
