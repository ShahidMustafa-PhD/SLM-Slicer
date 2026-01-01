#include "CollisionVisualizer.h"
#include "../core/domain/BuildPlate.h"
#include <vtkProperty.h>
#include <vtkActorCollection.h>
#include <vtkRenderWindow.h>
#include <set>

namespace MarcSLM {
namespace Infrastructure {

CollisionVisualizer::CollisionVisualizer(vtkSmartPointer<vtkRenderer> renderer)
    : m_renderer(renderer)
{
}

void CollisionVisualizer::update(const Domain::BuildPlate* buildPlate,
                                 const std::map<int, int>& modelToActorMap)
{
    if (!m_enabled || !buildPlate || !m_renderer) {
        return;
    }
    
    // Clear previous highlights
    clearHighlights();
    
    // Detect collisions
    auto collisions = buildPlate->detectCollisions();
    m_collisionCount = static_cast<int>(collisions.size());
    
    if (collisions.empty()) {
        return;
    }
    
    // Collect all colliding model IDs
    std::set<int> collidingModelIds;
    for (const auto& collision : collisions) {
        collidingModelIds.insert(collision.first);
        collidingModelIds.insert(collision.second);
    }
    
    // Highlight colliding actors in red
    for (int modelId : collidingModelIds) {
        auto it = modelToActorMap.find(modelId);
        if (it != modelToActorMap.end()) {
            highlightActor(it->second, 1.0, 0.0, 0.0);  // Red
            m_highlightedActors.push_back(it->second);
        }
    }
    
    // Request render
    if (m_renderer->GetRenderWindow()) {
        m_renderer->GetRenderWindow()->Render();
    }
}

void CollisionVisualizer::clearHighlights()
{
    // Restore original colors
    for (int actorId : m_highlightedActors) {
        restoreActorColor(actorId);
    }
    
    m_highlightedActors.clear();
    m_collisionCount = 0;
}

void CollisionVisualizer::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }
    
    m_enabled = enabled;
    
    if (!enabled) {
        clearHighlights();
        if (m_renderer && m_renderer->GetRenderWindow()) {
            m_renderer->GetRenderWindow()->Render();
        }
    }
}

void CollisionVisualizer::highlightActor(int actorId, double r, double g, double b)
{
    if (!m_renderer) {
        return;
    }
    
    // Find actor in renderer
    vtkActorCollection* actors = m_renderer->GetActors();
    actors->InitTraversal();
    
    vtkActor* targetActor = nullptr;
    int currentId = 0;
    
    for (vtkIdType i = 0; i < actors->GetNumberOfItems(); ++i) {
        vtkActor* actor = actors->GetNextActor();
        if (currentId == actorId) {
            targetActor = actor;
            break;
        }
        currentId++;
    }
    
    if (!targetActor) {
        return;
    }
    
    // Save original color if not already saved
    if (m_originalColors.find(actorId) == m_originalColors.end()) {
        double* color = targetActor->GetProperty()->GetColor();
        m_originalColors[actorId] = std::make_tuple(color[0], color[1], color[2]);
    }
    
    // Set highlight color
    targetActor->GetProperty()->SetColor(r, g, b);
}

void CollisionVisualizer::restoreActorColor(int actorId)
{
    auto it = m_originalColors.find(actorId);
    if (it == m_originalColors.end() || !m_renderer) {
        return;
    }
    
    // Find actor in renderer
    vtkActorCollection* actors = m_renderer->GetActors();
    actors->InitTraversal();
    
    vtkActor* targetActor = nullptr;
    int currentId = 0;
    
    for (vtkIdType i = 0; i < actors->GetNumberOfItems(); ++i) {
        vtkActor* actor = actors->GetNextActor();
        if (currentId == actorId) {
            targetActor = actor;
            break;
        }
        currentId++;
    }
    
    if (!targetActor) {
        return;
    }
    
    // Restore original color
    const auto& color = it->second;
    targetActor->GetProperty()->SetColor(
        std::get<0>(color),
        std::get<1>(color),
        std::get<2>(color)
    );
    
    m_originalColors.erase(it);
}

} // namespace Infrastructure
} // namespace MarcSLM
