#ifndef MARC_API_INTERFACE_HPP
#define MARC_API_INTERFACE_HPP

#ifdef _WIN32
  #ifdef MARC_EXPORTS
    // If we're building the DLL, use dllexport
    #define MARC_API __declspec(dllexport)
  #else
    // If we're using the DLL, use dllimport
    #define MARC_API __declspec(dllimport)
  #endif
#else
  #define MARC_API
#endif

#include <stddef.h>   // for size_t
#include <stdint.h>   // for fixed-width integer types, if needed

#ifdef __cplusplus
extern "C" {
#endif

typedef int MarcErrorCode;

#define MARC_S_OK        0x00000000
#define MARC_E_FAIL      0x80004005
#define MARC_E_INVALID   0x80070057
#define MARC_E_NOTIMPL   0x80004001

const char* get_last_error_message();

// Callback function signature
typedef void (*MarcProgressCallback)(const char* message, int percent);

// C-style struct to pass paths
struct MarcPathArray {
    const char** paths;
    size_t count;      // use plain size_t here
};

typedef struct {
    char path[100];     // STL model directory path
    char buildconfig[100];   //  Build jconfiguration file directory path
    char stylesconfig[100];   //  Build Styles configuration file directory path
    int number;         // ID of model
    double xpos;        // Optimum position on print bed (X axis)
    double ypos;        // Optimum position on print bed (Y axis)
    double zpos;        // Optimum position on print bed (Z axis)
    double roll;        // Optimum orientation about X axis
    double pitch;       // Optimum orientation about Y axis
    double yaw;         // Optimum orientation about Z axis
} GuiData;

typedef struct {
    GuiData* models;      // Pointer to an array of Model structs
    size_t count;       // use size_t here too
} GuiDataArray;

// Opaque pointer to the internal implementation
typedef void* MarcHandle;

// Factory functions
MarcHandle create_marc_api(float bed_width, float bed_depth, float spacing);
void destroy_marc_api(MarcHandle handle);

// Set input data
MarcErrorCode set_models(MarcHandle handle, GuiDataArray models);

MarcErrorCode set_configs(MarcHandle handle, struct MarcPathArray configs);
MarcErrorCode set_actions(MarcHandle handle, const char** actions, size_t count);
MarcErrorCode set_transforms(MarcHandle handle, const char** transforms, size_t count);

// Operations
MarcErrorCode update_model(MarcHandle handle);
MarcErrorCode duplicate(MarcHandle handle);
MarcErrorCode duplicate_grid(MarcHandle handle);
MarcErrorCode around_center(MarcHandle handle);
MarcErrorCode align_xy(MarcHandle handle, float px, float py);
MarcErrorCode rotate_x(MarcHandle handle, float angle);
MarcErrorCode rotate_y(MarcHandle handle, float angle);
MarcErrorCode rotate_z(MarcHandle handle, float angle);
MarcErrorCode scale(MarcHandle handle, float s);
MarcErrorCode scale_to_fit(MarcHandle handle);
MarcErrorCode split(MarcHandle handle);
MarcErrorCode repair(MarcHandle handle);
MarcErrorCode info(MarcHandle handle);
MarcErrorCode print(MarcHandle handle);
MarcErrorCode set_config_json(MarcHandle handle, const char* filePath);

// Export
MarcErrorCode export_slm_file(MarcHandle handle);

// Register callback
void register_progress_callback(MarcHandle handle, MarcProgressCallback callback);

// The caller must call free_model_array() after use to avoid leaks.
GuiDataArray get_models(MarcHandle handle);

// Frees memory allocated for a ModelArray returned by get_models()
MarcErrorCode free_model_array(GuiDataArray array);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MARC_API_INTERFACE_HPP
