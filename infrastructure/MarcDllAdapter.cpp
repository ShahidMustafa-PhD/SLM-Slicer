#include "MarcDllAdapter.h"
#include <cstring>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif

namespace MarcSLM {
namespace Infrastructure {

#ifdef _WIN32
// Function pointer types
typedef MarcHandle (*PFN_CREATE_MARC_API)(float, float, float);
typedef void (*PFN_DESTROY_MARC_API)(MarcHandle);
typedef MarcErrorCode (*PFN_SET_MODELS)(MarcHandle, GuiDataArray);
typedef MarcErrorCode (*PFN_SET_CONFIG_JSON)(MarcHandle, const char*);
typedef MarcErrorCode (*PFN_UPDATE_MODEL)(MarcHandle);
typedef MarcErrorCode (*PFN_EXPORT_SLM_FILE)(MarcHandle);

// Function pointers
static HMODULE hMarcApiDll = NULL;
static PFN_CREATE_MARC_API pCreateMarcApi = NULL;
static PFN_DESTROY_MARC_API pDestroyMarcApi = NULL;
static PFN_SET_MODELS pSetModels = NULL;
static PFN_SET_CONFIG_JSON pSetConfigJson = NULL;
static PFN_UPDATE_MODEL pUpdateModel = NULL;
static PFN_EXPORT_SLM_FILE pExportSlmFile = NULL;

// Load the DLL and function pointers
static bool LoadMarcApiDll() {
    if (hMarcApiDll) return true; // Already loaded
    
    hMarcApiDll = LoadLibraryA("marcapi.dll");
    if (!hMarcApiDll) {
        return false;
    }
    
    pCreateMarcApi = (PFN_CREATE_MARC_API)GetProcAddress(hMarcApiDll, "create_marc_api");
    pDestroyMarcApi = (PFN_DESTROY_MARC_API)GetProcAddress(hMarcApiDll, "destroy_marc_api");
    pSetModels = (PFN_SET_MODELS)GetProcAddress(hMarcApiDll, "set_models");
    pSetConfigJson = (PFN_SET_CONFIG_JSON)GetProcAddress(hMarcApiDll, "set_config_json");
    pUpdateModel = (PFN_UPDATE_MODEL)GetProcAddress(hMarcApiDll, "update_model");
    pExportSlmFile = (PFN_EXPORT_SLM_FILE)GetProcAddress(hMarcApiDll, "export_slm_file");
    
    // Check if all required functions are loaded
    if (!pCreateMarcApi || !pDestroyMarcApi || !pSetModels || !pSetConfigJson || !pUpdateModel || !pExportSlmFile) {
        FreeLibrary(hMarcApiDll);
        hMarcApiDll = NULL;
        return false;
    }
    
    return true;
}

// Unload the DLL
static void UnloadMarcApiDll() {
    if (hMarcApiDll) {
        FreeLibrary(hMarcApiDll);
        hMarcApiDll = NULL;
        pCreateMarcApi = NULL;
        pDestroyMarcApi = NULL;
        pSetModels = NULL;
        pSetConfigJson = NULL;
        pUpdateModel = NULL;
        pExportSlmFile = NULL;
    }
}
#endif

MarcDllAdapter::~MarcDllAdapter() {
    cleanup();
}

Application::Result MarcDllAdapter::initialize(double radius, double height, double spacing) {
#ifdef _WIN32
    if (!LoadMarcApiDll()) {
        return Application::Result::error("Failed to load marcapi.dll");
    }
#endif
    
    // Clean up any existing handle
    if (m_handle) {
        cleanup();
    }
    
#ifdef _WIN32
    m_handle = pCreateMarcApi(
        static_cast<float>(radius * 2.0),  // width
        static_cast<float>(radius * 2.0),  // depth (assuming circular plate)
        static_cast<float>(spacing)
    );
#else
    // For non-Windows, assume functions are linked statically or handle differently
    m_handle = create_marc_api(
        static_cast<float>(radius * 2.0),
        static_cast<float>(radius * 2.0),
        static_cast<float>(spacing)
    );
#endif
    
    if (!m_handle) {
        return Application::Result::error("Failed to create MARC API handle");
    }
    
    return Application::Result::success();
}

Application::Result MarcDllAdapter::setModels(const std::vector<const Domain::Model*>& models) {
    if (!m_handle) {
        return Application::Result::error("DLL not initialized. Call initialize() first.");
    }
    
    if (models.empty()) {
        return Application::Result::error("No models to export");
    }
    
    // Free previous data
    freeGuiDataArray();
    
    // Allocate new array
    m_guiDataArray.count = models.size();
    m_guiDataArray.models = static_cast<GuiData*>(
        std::calloc(m_guiDataArray.count, sizeof(GuiData))
    );
    
    if (!m_guiDataArray.models) {
        m_guiDataArray.count = 0;
        return Application::Result::error("Failed to allocate memory for model array");
    }
    
    // Convert each model
    // TODO: Get config paths from somewhere (for now, use empty strings)
    for (size_t i = 0; i < models.size(); ++i) {
        m_guiDataArray.models[i] = convertModel(*models[i], "", "");
    }
    
    // Send to DLL
#ifdef _WIN32
    MarcErrorCode err = pSetModels(m_handle, m_guiDataArray);
#else
    MarcErrorCode err = set_models(m_handle, m_guiDataArray);
#endif
    if (err != MARC_S_OK) {
        return Application::Result::error("Failed to set models in DLL");
    }
    
    return Application::Result::success();
}

Application::Result MarcDllAdapter::setConfig(const std::string& configPath) {
    if (!m_handle) {
        return Application::Result::error("DLL not initialized");
    }
    
    if (configPath.empty()) {
        return Application::Result::error("Config path is empty");
    }
    
#ifdef _WIN32
    MarcErrorCode err = pSetConfigJson(m_handle, configPath.c_str());
#else
    MarcErrorCode err = set_config_json(m_handle, configPath.c_str());
#endif
    if (err != MARC_S_OK) {
        return Application::Result::error("Failed to set config in DLL");
    }
    
    return Application::Result::success();
}

Application::Result MarcDllAdapter::updateModel() {
    if (!m_handle) {
        return Application::Result::error("DLL not initialized");
    }
    
#ifdef _WIN32
    MarcErrorCode err = pUpdateModel(m_handle);
#else
    MarcErrorCode err = update_model(m_handle);
#endif
    if (err != MARC_S_OK) {
        return Application::Result::error("Failed to update model in DLL");
    }
    
    return Application::Result::success();
}

Application::Result MarcDllAdapter::exportSlmFile() {
    if (!m_handle) {
        return Application::Result::error("DLL not initialized");
    }
    
#ifdef _WIN32
    MarcErrorCode err = pExportSlmFile(m_handle);
#else
    MarcErrorCode err = export_slm_file(m_handle);
#endif
    if (err != MARC_S_OK) {
        return Application::Result::error("Failed to export SLM file");
    }
    
    return Application::Result::success();
}

void MarcDllAdapter::cleanup() {
    freeGuiDataArray();
    
    if (m_handle) {
#ifdef _WIN32
        pDestroyMarcApi(m_handle);
#else
        destroy_marc_api(m_handle);
#endif
        m_handle = nullptr;
    }
    
#ifdef _WIN32
    // Optionally unload DLL here, but keep loaded for multiple uses
    // UnloadMarcApiDll();
#endif
}

GuiData MarcDllAdapter::convertModel(
    const Domain::Model& model,
    const std::string& buildConfig,
    const std::string& stylesConfig
) {
    GuiData data;
    std::memset(&data, 0, sizeof(GuiData));
    
    // Copy file path
    std::strncpy(data.path, model.filePath().c_str(), sizeof(data.path) - 1);
    data.path[sizeof(data.path) - 1] = '\0';
    
    // Copy config paths
    if (!buildConfig.empty()) {
        std::strncpy(data.buildconfig, buildConfig.c_str(), sizeof(data.buildconfig) - 1);
        data.buildconfig[sizeof(data.buildconfig) - 1] = '\0';
    }
    
    if (!stylesConfig.empty()) {
        std::strncpy(data.stylesconfig, stylesConfig.c_str(), sizeof(data.stylesconfig) - 1);
        data.stylesconfig[sizeof(data.stylesconfig) - 1] = '\0';
    }
    
    // Copy transform
    Domain::Transform t = model.transform();
    data.number = model.id();
    data.xpos = t.x;
    data.ypos = t.y;
    data.zpos = t.z;
    data.roll = t.roll;
    data.pitch = t.pitch;
    data.yaw = t.yaw;
    
    return data;
}

void MarcDllAdapter::freeGuiDataArray() {
    if (m_guiDataArray.models) {
        std::free(m_guiDataArray.models);
        m_guiDataArray.models = nullptr;
        m_guiDataArray.count = 0;
    }
}

} // namespace Infrastructure
} // namespace MarcSLM
