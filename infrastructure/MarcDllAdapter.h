#ifndef MARCDLLADAPTER_H
#define MARCDLLADAPTER_H

#include "../core/domain/Model.h"
#include "../core/domain/BuildPlate.h"
#include "../core/application/Result.h"

// Include the external DLL interface
#include "../marc_qtsrc/MarcAPIInterface.h"

#include <vector>
#include <string>

namespace MarcSLM {
namespace Infrastructure {

/**
 * @brief Adapter for the MARC slicer DLL
 * 
 * Responsibilities:
 * - Convert domain Models to DLL's GuiDataArray format
 * - Call MARC API functions (create_marc_api, set_models, export_slm_file, etc.)
 * - Manage DLL handle lifecycle
 * - Handle memory allocation/deallocation for DLL data structures
 * 
 * This is the ONLY class in the codebase that knows about MarcAPIInterface.h.
 * If the DLL interface changes, only this adapter needs updates.
 */
class MarcDllAdapter {
public:
    MarcDllAdapter() = default;
    ~MarcDllAdapter();
    
    /**
     * @brief Initialize the DLL with build plate dimensions
     * @param radius Build plate radius (mm)
     * @param height Build height (mm)
     * @param spacing Model spacing for auto-arrange (mm)
     * @return Result indicating success or failure
     */
    Application::Result initialize(double radius, double height, double spacing = 5.0);
    
    /**
     * @brief Convert domain models to DLL format and send to API
     * @param models Vector of domain models
     * @return Result indicating success or failure
     */
    Application::Result setModels(const std::vector<const Domain::Model*>& models);
    
    /**
     * @brief Set the build configuration JSON file
     * @param configPath Absolute path to config JSON
     * @return Result indicating success or failure
     */
    Application::Result setConfig(const std::string& configPath);
    
    /**
     * @brief Trigger the DLL to update models with config
     * @return Result indicating success or failure
     */
    Application::Result updateModel();
    
    /**
     * @brief Export the sliced file
     * @return Result indicating success or failure
     */
    Application::Result exportSlmFile();
    
    /**
     * @brief Clean up DLL resources
     */
    void cleanup();
    
private:
    MarcHandle m_handle = nullptr;
    GuiDataArray m_guiDataArray = { nullptr, 0 };
    
    // Helper: Convert domain Model to DLL GuiData
    GuiData convertModel(const Domain::Model& model, 
                        const std::string& buildConfig,
                        const std::string& stylesConfig);
    
    // Helper: Free GuiDataArray memory
    void freeGuiDataArray();
};

} // namespace Infrastructure
} // namespace MarcSLM

#endif // MARCDLLADAPTER_H
