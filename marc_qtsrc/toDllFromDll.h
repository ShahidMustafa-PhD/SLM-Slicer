#pragma once

#include <vector>
#include <string>
#include <cstring>
#include "MarcAPIInterface.h" // For GuiData and GuiDataArray definitions
#include "slmcommons.h"

class toDllFromDll {
public:
    // Convert from internal model format to DLL format
    static GuiDataArray toDll(const std::vector<InternalGuiModel>& internalModels) {
        GuiDataArray result = { nullptr, 0 }; // Safe initialization

        // Return empty result if no models
        if (internalModels.empty()) {
            return result;
        }

        result.count = internalModels.size();

        // Use calloc to zero-initialize all memory
        result.models = static_cast<GuiData*>(std::calloc(result.count, sizeof(GuiData)));
        if (!result.models) {
            result.count = 0;
            return result;
        }

        // Process each model
        for (size_t i = 0; i < result.count; ++i) {
            // Initialize all string fields to empty
            result.models[i].path[0] = '\0';
            result.models[i].buildconfig[0] = '\0';
            result.models[i].stylesconfig[0] = '\0';

            // Copy path if it exists
            if (!internalModels[i].path.empty()) {
                std::strncpy(result.models[i].path, internalModels[i].path.c_str(),
                    sizeof(result.models[i].path) - 1);
                result.models[i].path[sizeof(result.models[i].path) - 1] = '\0';
            }

            // Copy buildconfig if it exists
            if (!internalModels[i].buildconfig.empty()) {
                std::strncpy(result.models[i].buildconfig, internalModels[i].buildconfig.c_str(),
                    sizeof(result.models[i].buildconfig) - 1);
                result.models[i].buildconfig[sizeof(result.models[i].buildconfig) - 1] = '\0';
            }

            // Copy stylesconfig if it exists
            if (!internalModels[i].stylesconfig.empty()) {
                std::strncpy(result.models[i].stylesconfig, internalModels[i].stylesconfig.c_str(),
                    sizeof(result.models[i].stylesconfig) - 1);
                result.models[i].stylesconfig[sizeof(result.models[i].stylesconfig) - 1] = '\0';
            }

            // Copy numeric values
            result.models[i].number = internalModels[i].model_number;
            result.models[i].xpos = internalModels[i].xpos;
            result.models[i].ypos = internalModels[i].ypos;
            result.models[i].zpos = internalModels[i].zpos;
            result.models[i].roll = internalModels[i].roll;
            result.models[i].pitch = internalModels[i].pitch;
            result.models[i].yaw = internalModels[i].yaw;
        }

        return result;
    }

    // Convert from DLL format to internal model format
    static std::vector<InternalGuiModel> fromDll(const GuiDataArray& dllArray) {
        std::vector<InternalGuiModel> result;

        if (dllArray.models == nullptr || dllArray.count == 0) {
            return result;
        }

        result.reserve(dllArray.count);

        for (size_t i = 0; i < dllArray.count; ++i) {
            InternalGuiModel model;
            // Copy all fields
            model.path = dllArray.models[i].path;
            model.buildconfig = dllArray.models[i].buildconfig;
           // model.stylesconfig = dllArray.models[i].stylesconfig;
            model.model_number = dllArray.models[i].number;
            model.xpos = dllArray.models[i].xpos;
            model.ypos = dllArray.models[i].ypos;
            model.zpos = dllArray.models[i].zpos;
            model.roll = dllArray.models[i].roll;
            model.pitch = dllArray.models[i].pitch;
            model.yaw = dllArray.models[i].yaw;

            result.push_back(model);
        }

        return result;
    }

    // Free memory allocated for GuiDataArray
    static void freeGuiDataArray(GuiDataArray& array) {
        // Simply check if the models pointer is valid
        if (array.models != nullptr) {
            // Free the allocated memory
            std::free(array.models);

            // Reset values to safe defaults
            array.models = nullptr;
            array.count = 0;
        }
    }
};