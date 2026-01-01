#ifndef ADDMODELUSECASE_H
#define ADDMODELUSECASE_H

#include "../Result.h"
#include "../interfaces/IStlFileLoader.h"
#include "../interfaces/IModelRenderer.h"
#include "../../domain/BuildPlate.h"
#include "../../domain/Model.h"
#include <string>
#include <functional>
#include <memory>

namespace MarcSLM {
namespace Application {

/**
 * @brief Use case for loading and adding a 3D model to the build plate
 * 
 * Workflow:
 * 1. Load STL file using IStlFileLoader
 * 2. Create domain Model entity
 * 3. Add model to BuildPlate
 * 4. Render model using IModelRenderer
 * 
 * This encapsulates the entire "Add Model" business process.
 */
class AddModelUseCase {
public:
    using ProgressCallback = std::function<void(const std::string&)>;
    
    /**
     * @brief Construct the use case with required dependencies
     * @param fileLoader STL file loading service
     * @param buildPlate Build plate domain model
     * @param renderer Rendering service (optional, can be null for headless mode)
     */
    AddModelUseCase(
        std::shared_ptr<IStlFileLoader> fileLoader,
        std::shared_ptr<Domain::BuildPlate> buildPlate,
        std::shared_ptr<IModelRenderer> renderer = nullptr
    );
    
    /**
     * @brief Execute the use case
     * @param filePath Absolute path to STL file
     * @return Result indicating success or failure
     */
    Result execute(const std::string& filePath);
    
    /**
     * @brief Set a callback for progress updates
     * @param callback Function called with status messages
     */
    void setProgressCallback(ProgressCallback callback) {
        m_progressCallback = std::move(callback);
    }
    
private:
    std::shared_ptr<IStlFileLoader> m_fileLoader;
    std::shared_ptr<Domain::BuildPlate> m_buildPlate;
    std::shared_ptr<IModelRenderer> m_renderer;
    ProgressCallback m_progressCallback;
    
    void notifyProgress(const std::string& message);
};

} // namespace Application
} // namespace MarcSLM

#endif // ADDMODELUSECASE_H
