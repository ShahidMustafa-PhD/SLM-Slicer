#include "AddModelUseCase.h"
#include <filesystem>

namespace MarcSLM {
namespace Application {

AddModelUseCase::AddModelUseCase(
    std::shared_ptr<IStlFileLoader> fileLoader,
    std::shared_ptr<Domain::BuildPlate> buildPlate,
    std::shared_ptr<IModelRenderer> renderer
)
    : m_fileLoader(std::move(fileLoader))
    , m_buildPlate(std::move(buildPlate))
    , m_renderer(std::move(renderer))
    , m_progressCallback(nullptr)
{
}

Result AddModelUseCase::execute(const std::string& filePath) {
    // Validate file exists
    if (!std::filesystem::exists(filePath)) {
        return Result::error("File does not exist: " + filePath);
    }
    
    notifyProgress("Loading STL file...");
    
    // Load mesh data
    if (!m_fileLoader) return Result::error("No file loader available");
    auto meshData = m_fileLoader->load(filePath);
    if (!meshData) {
        return Result::error("Failed to load STL file: " + filePath);
    }
    
    notifyProgress("Creating model entity...");
    
    // Create domain model
    Domain::Model model(filePath);
    model.setBounds(meshData->bounds);
    model.setTriangleCount(meshData->triangleCount);
    model.setVolume(meshData->volume);
    
    // Validate model is within build volume
    // (Allow adding for now, but could make this configurable)
    if (m_buildPlate && !m_buildPlate->isInsideBuildVolume(model)) {
        notifyProgress("Warning: Model may be outside build volume");
    }
    
    // Add to build plate (assigns ID)
    if (!m_buildPlate) return Result::error("No build plate available");
    int assignedId = m_buildPlate->addModel(model);
    
    // Verify model was added
    auto addedModel = m_buildPlate->getModel(assignedId);
    if (!addedModel) {
        return Result::error("Failed to add model to build plate");
    }
    
    // Render if renderer available
    if (m_renderer && meshData->nativeData) {
        notifyProgress("Rendering model...");
        m_renderer->addModel(*addedModel, meshData->nativeData);
        m_renderer->render();
    }
    
    notifyProgress("Model added successfully");
    return Result::success();
}

void AddModelUseCase::notifyProgress(const std::string& message) {
    if (m_progressCallback) {
        m_progressCallback(message);
    }
}

} // namespace Application
} // namespace MarcSLM
