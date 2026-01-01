#include "VtkModelRenderer.h"
#include <vtkProperty.h>
#include <vtkRenderWindow.h>

namespace MarcSLM {
namespace Infrastructure {

VtkModelRenderer::VtkModelRenderer(vtkSmartPointer<vtkRenderer> renderer)
    : m_renderer(renderer)
    , m_nextActorId(1)
{
}

int VtkModelRenderer::addModel(const Domain::Model& model, const std::shared_ptr<void>& meshData) {
    if (!meshData) {
        return -1;
    }

    // Cast opaque pointer back to vtkPolyData*
    vtkPolyData* polyRaw = static_cast<vtkPolyData*>(meshData.get());
    if (!polyRaw) return -1;

    // Create mapper (use raw pointer as input, do not take ownership)
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyRaw);

    // Create actor
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // Create transform
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    applyTransform(transform, model.transform());
    actor->SetUserTransform(transform);

    // Default appearance
    actor->GetProperty()->SetColor(0.8, 0.8, 0.8);
    actor->GetProperty()->SetInterpolationToPhong();

    // Store actor data
    int actorId = m_nextActorId++;
    ActorData data;
    data.actor = actor;
    data.transform = transform;
    data.mapper = mapper;
    data.meshData = meshData; // keep shared ownership so polydata isn't freed
    m_actors[actorId] = data;

    // Add to renderer
    if (m_renderer) {
        m_renderer->AddActor(actor);
    }

    return actorId;
}

void VtkModelRenderer::updateModelTransform(int actorId, const Domain::Transform& transform) {
    auto it = m_actors.find(actorId);
    if (it == m_actors.end()) {
        return;
    }

    applyTransform(it->second.transform, transform);
}

void VtkModelRenderer::removeModel(int actorId) {
    auto it = m_actors.find(actorId);
    if (it == m_actors.end()) {
        return;
    }

    if (m_renderer) {
        m_renderer->RemoveActor(it->second.actor);
    }
    // actor and mapper smart pointers will release here; meshData shared_ptr will
    // release and call UnRegister on vtkPolyData if this was the last owner.
    m_actors.erase(it);
}

void VtkModelRenderer::setModelColor(int actorId, double r, double g, double b) {
    auto it = m_actors.find(actorId);
    if (it == m_actors.end()) {
        return;
    }

    it->second.actor->GetProperty()->SetColor(r, g, b);
}

void VtkModelRenderer::clearScene() {
    for (auto& pair : m_actors) {
        if (m_renderer) {
            m_renderer->RemoveActor(pair.second.actor);
        }
        // Explicitly clear meshData to ensure vtk objects are released while
        // still detached from renderer to avoid VTK accessing freed memory
        pair.second.meshData.reset();
    }
    m_actors.clear();
    m_nextActorId = 1;
}

void VtkModelRenderer::render() {
    if (m_renderer && m_renderer->GetRenderWindow()) {
        m_renderer->GetRenderWindow()->Render();
    }
}

void VtkModelRenderer::applyTransform(vtkTransform* vtkTrans, const Domain::Transform& domainTrans) {
    vtkTrans->Identity();

    // Apply translation
    vtkTrans->Translate(domainTrans.x, domainTrans.y, domainTrans.z);

    // Apply rotations (Euler angles in degrees)
    vtkTrans->RotateZ(domainTrans.yaw);
    vtkTrans->RotateY(domainTrans.pitch);
    vtkTrans->RotateX(domainTrans.roll);
}

} // namespace Infrastructure
} // namespace MarcSLM
