#ifndef VTKMODELRENDERER_H
#define VTKMODELRENDERER_H

#include "../core/application/interfaces/IModelRenderer.h"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkPolyData.h>
#include <map>
#include <memory>

namespace MarcSLM {
namespace Infrastructure {

/**
 * @brief VTK implementation of the model renderer interface
 * 
 * Manages VTK actors and their transforms, syncing with domain Model data.
 * This class owns the rendering-specific VTK objects but does not own
 * the business data (transforms, bounds, etc.) - those live in Model.
 */
class VtkModelRenderer : public Application::IModelRenderer {
public:
    explicit VtkModelRenderer(vtkSmartPointer<vtkRenderer> renderer);
    ~VtkModelRenderer() override = default;
    
    // IModelRenderer interface
    int addModel(const Domain::Model& model, const std::shared_ptr<void>& meshData) override;
    void updateModelTransform(int actorId, const Domain::Transform& transform) override;
    void removeModel(int actorId) override;
    void setModelColor(int actorId, double r, double g, double b) override;
    void clearScene() override;
    void render() override;
    
private:
    struct ActorData {
        vtkSmartPointer<vtkActor> actor;
        vtkSmartPointer<vtkTransform> transform;
        vtkSmartPointer<vtkPolyDataMapper> mapper;
        // Keep a shared ownership of the raw VTK polydata to prevent it from
        // being destroyed while the actor/mapper are still using it.
        std::shared_ptr<void> meshData;
    };
    
    vtkSmartPointer<vtkRenderer> m_renderer;  // Now owned/shared via smart pointer
    std::map<int, ActorData> m_actors;
    int m_nextActorId = 1;
    
    void applyTransform(vtkTransform* vtkTrans, const Domain::Transform& domainTrans);
};

} // namespace Infrastructure
} // namespace MarcSLM

#endif // VTKMODELRENDERER_H
