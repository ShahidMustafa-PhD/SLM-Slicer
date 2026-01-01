#ifndef IMODELRENDERER_H
#define IMODELRENDERER_H

#include "../../domain/Model.h"
#include "../../domain/Transform.h"
#include <string>
#include <memory>

namespace MarcSLM {
namespace Application {

/**
 * @brief Interface for rendering 3D models in a viewport
 * 
 * This abstraction separates rendering logic from business logic,
 * allowing the domain/application layers to remain independent of
 * the rendering library (VTK, Qt3D, OpenGL, etc.).
 * 
 * Implementations:
 * - VtkModelRenderer: Uses VTK for rendering
 * - (Future) Qt3DModelRenderer: Uses QtQuick3D
 */
class IModelRenderer {
public:
    virtual ~IModelRenderer() = default;
    
    /**
     * @brief Add a model to the scene and start rendering it
     * @param model Domain model to render
     * @param meshData Opaque shared_ptr returned by IStlFileLoader (implementation-specific)
     * @return Renderer-specific actor ID for future updates
     */
    virtual int addModel(const Domain::Model& model, const std::shared_ptr<void>& meshData) = 0;
    
    /**
     * @brief Update the transform of a rendered model
     * @param actorId Renderer-specific ID from addModel()
     * @param transform New transform to apply
     */
    virtual void updateModelTransform(int actorId, const Domain::Transform& transform) = 0;
    
    /**
     * @brief Remove a model from the scene
     * @param actorId Renderer-specific ID from addModel()
     */
    virtual void removeModel(int actorId) = 0;
    
    /**
     * @brief Set the color of a rendered model
     * @param actorId Renderer-specific ID
     * @param r Red (0.0 - 1.0)
     * @param g Green (0.0 - 1.0)
     * @param b Blue (0.0 - 1.0)
     */
    virtual void setModelColor(int actorId, double r, double g, double b) = 0;
    
    /**
     * @brief Clear all models from the scene
     */
    virtual void clearScene() = 0;
    
    /**
     * @brief Refresh the viewport
     */
    virtual void render() = 0;
};

} // namespace Application
} // namespace MarcSLM

#endif // IMODELRENDERER_H
